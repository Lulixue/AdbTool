// InputLogDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "InputLogDlg.h"
#include "afxdialogex.h"
#include "AdbInterface.h"
#include <map>

// CInputLogDlg 对话框

/*
* Android log priority values, in ascending priority order.
*/
enum android_LogPriority {
	ANDROID_LOG_UNKNOWN,
	ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
	ANDROID_LOG_VERBOSE = 0,
	ANDROID_LOG_DEBUG,
	ANDROID_LOG_INFO,
	ANDROID_LOG_WARN,
	ANDROID_LOG_ERROR,
	ANDROID_LOG_FATAL,
	ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
};

std::map<int, CString> g_mapPrintkLevel;
std::map<int, CString> g_mapLogLevel;

IMPLEMENT_DYNAMIC(CInputLogDlg, CDialogEx)

CInputLogDlg::CInputLogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputLogDlg::IDD, pParent)
{

}

CInputLogDlg::~CInputLogDlg()
{
}

void InitMaps()
{
	if (!g_mapPrintkLevel.empty()) 
	{
		return;
	}

	g_mapPrintkLevel[KERN_EMERG] = TEXT("KERN_EMERG");
	g_mapPrintkLevel[KERN_ALERT] = TEXT("KERN_ALERT");
	g_mapPrintkLevel[KERN_CRIT] = TEXT("KERN_CRIT");
	g_mapPrintkLevel[KERN_ERR] = TEXT("KERN_ERR");
	g_mapPrintkLevel[KERN_WARNING] = TEXT("KERN_WARNING");
	g_mapPrintkLevel[KERN_NOTICE] = TEXT("KERN_NOTICE");
	g_mapPrintkLevel[KERN_INFO] = TEXT("KERN_INFO");
	g_mapPrintkLevel[KERN_DEBUG] = TEXT("KERN_DEBUG");
	g_mapPrintkLevel[KERN_ALL] = TEXT("KERN_ALL");

	g_mapLogLevel[ANDROID_LOG_VERBOSE] = TEXT("VERBOSE");
	g_mapLogLevel[ANDROID_LOG_DEBUG] = TEXT("DEBUG");
	g_mapLogLevel[ANDROID_LOG_INFO] = TEXT("INFO");
	g_mapLogLevel[ANDROID_LOG_WARN] = TEXT("WARN");
	g_mapLogLevel[ANDROID_LOG_ERROR] = TEXT("ERROR");
	//g_mapLogLevel[ANDROID_LOG_FATAL] = TEXT("FATAL");

}


BOOL CInputLogDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	m_cbInputDevices.AddString(TEXT("All"));

	vector<INPUT_EVENT_DEV_T>::iterator it = ADB.m_vecInputDevices.begin();
	for (int i = 0; it != ADB.m_vecInputDevices.end(); it++, i++) 
	{
		CString strItem;
		strItem.Format(TEXT("Event%d: %s"), it->no, it->strName);

		m_cbInputDevices.AddString(strItem);

		vector<CString>::const_iterator cit = it->vecAttrs.begin();
		for (; cit != it->vecAttrs.end(); cit++)
		{
			m_strAllAttrs += *cit;
			m_strAllAttrs += TEXT("\n");
		}
		m_strAllAttrs += TEXT("\n");
	}

	m_strAllAttrs.Replace(TEXT("\n"), TEXT("\r\n"));
	
	m_cbInputDevices.SetCurSel(0);

	m_richInputAttrs.SetWindowTextW(m_strAllAttrs);

	InitMaps();
	

	std::map<int, CString>::const_iterator cit = g_mapPrintkLevel.begin();
	for (; cit != g_mapPrintkLevel.end(); cit++)
	{
		CString item;
		item.Format(TEXT("[%d]%s"), cit->first, cit->second);
		m_cbLogLevel.AddString(item);
	}

	cit = g_mapLogLevel.begin();
	for (; cit != g_mapLogLevel.end(); cit++)
	{
		CString item;
		item.Format(TEXT("[%s]%s"), cit->second.Mid(0,1), cit->second);
		m_cbLogcatFilterLevel.AddString(item);
	}
	m_editLogcatFilter.SetWindowTextW(TEXT("Focaltech"));

	int nCurrentLevel = ADB.GetPrintkLevel();
	m_cbLogLevel.SetCurSel(nCurrentLevel);


	((CButton*)GetDlgItem(IDC_CHECK_LABEL))->SetCheck(BST_CHECKED);

	m_droidCurrentInt.name = TEXT("fts_ts");
	RefreshInterrupt();
	return TRUE;
}

void CInputLogDlg::SelectCpu(int nCpu)
{
	CString cpu;
	CString item;
	map<int, long long>::const_iterator cit;

	m_cbIntCpus.SetCurSel(nCpu);
	m_cbIntCpus.GetWindowTextW(cpu);

	
	/* all */
	if (cpu.Find(TEXT("All")) != -1)
	{
		cit = m_droidCurrentInt.cpu_triggered.begin();
		for (int i = 0; cit != m_droidCurrentInt.cpu_triggered.end(); cit++, i++)
		{
			item.AppendFormat(TEXT("(%d:%4ld)"), cit->first, cit->second);
			if (((i+1)%2) == 0) {
				item.Append(TEXT("\r\n"));
			}
		}
	}
	else {
		int cpuNo = _wtoi(cpu.Mid(3));
		cit = m_droidCurrentInt.cpu_triggered.find(cpuNo);
		if (cit != m_droidCurrentInt.cpu_triggered.end()) {
			item.AppendFormat(TEXT("(%d:%4ld)\r\n"), cit->first, cit->second);
		}
	}
	if (m_droidCurrentInt.IsSysInt()) {
		item.AppendFormat(TEXT("[Sys:%s]"), m_droidCurrentInt.name);
	} else {
		item.AppendFormat(TEXT("[%d:%s]"), m_droidCurrentInt.no, m_droidCurrentInt.name);
	}
	m_editDetails.SetWindowTextW(item);
}

void CInputLogDlg::SelectInterrupt(int pos)
{
	CString name;
	m_cbIntNames.SetCurSel(pos);
	m_cbIntNames.GetWindowTextW(name);

	BOOL bRet = m_mgrInterrupts.NameToInt(name, m_droidCurrentInt);
	if (!bRet) {
		return;
	}

	if (m_cbIntCpus.GetCount() != (m_droidCurrentInt.cpu_triggered.size()+1)) {
		m_cbIntCpus.ResetContent();
		m_cbIntCpus.AddString(TEXT("All"));
		map<int, long long>::const_iterator cit = m_droidCurrentInt.cpu_triggered.begin();
		CString item;
		for (; cit != m_droidCurrentInt.cpu_triggered.end(); cit++) {
			item.Format(TEXT("CPU%d"), cit->first);
			m_cbIntCpus.AddString(item);
		}
		m_cbIntCpus.SetCurSel(0);
	}

	
	SelectCpu(m_cbIntCpus.GetCurSel());
}

void CInputLogDlg::RefreshInterrupt()
{
	m_cbIntNames.ResetContent();
	m_editDetails.Clear();

	m_mgrInterrupts.Refresh();
	vector<CString> intNames;
	m_mgrInterrupts.GetIntNames(intNames);
	int maxLength = 10;

	vector<CString>::const_iterator intcit = intNames.begin();
	for (; intcit != intNames.end(); intcit++)
	{
		m_cbIntNames.AddString(*intcit);
		if (intcit->GetLength() > maxLength) {
			maxLength = intcit->GetLength();
		}
	}
	int pos = m_cbIntNames.FindString(-1, m_droidCurrentInt.name);
	SelectInterrupt(pos);
	m_cbIntNames.SetDroppedWidth(maxLength * 8);
}

void CInputLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_INPUT, m_cbInputDevices);
	DDX_Control(pDX, IDC_RICHEDIT_INPUT_ATTR, m_richInputAttrs);
	DDX_Control(pDX, IDC_COMBO_LOGLEVELS, m_cbLogLevel);
	DDX_Control(pDX, IDC_COMBO_FILTER_LEVEL, m_cbLogcatFilterLevel);
	DDX_Control(pDX, IDC_EDIT_LOGCAT_FILTER, m_editLogcatFilter);
	DDX_Control(pDX, IDC_COMBO_INT_CPU, m_cbIntCpus);
	DDX_Control(pDX, IDC_COMBO_INT_NAME, m_cbIntNames);
	DDX_Control(pDX, IDC_EDIT_INT_TRIGGERED, m_editDetails);
}


BEGIN_MESSAGE_MAP(CInputLogDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_GET_EVENT, &CInputLogDlg::OnBnClickedBtnGetEvent)
	ON_BN_CLICKED(IDC_BTN_CAT_PROC_KMSG, &CInputLogDlg::OnBnClickedBtnCatProcKmsg)
	ON_BN_CLICKED(IDC_BTN_DMESG, &CInputLogDlg::OnBnClickedBtnDmesg)
	ON_CBN_SELCHANGE(IDC_COMBO_INPUT, &CInputLogDlg::OnCbnSelchangeComboInput)
	ON_CBN_SELCHANGE(IDC_COMBO_LOGLEVELS, &CInputLogDlg::OnCbnSelchangeComboLoglevels)
	ON_BN_CLICKED(IDC_BTN_ADB_LOGCAT, &CInputLogDlg::OnBnClickedBtnAdbLogcat)
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_BTN_INT_REFRESH, &CInputLogDlg::OnBnClickedBtnIntRefresh)
	ON_CBN_SELCHANGE(IDC_COMBO_INT_NAME, &CInputLogDlg::OnCbnSelchangeComboIntName)
	ON_CBN_SELCHANGE(IDC_COMBO_INT_CPU, &CInputLogDlg::OnCbnSelchangeComboIntCpu)
END_MESSAGE_MAP()


// CInputLogDlg 消息处理程序


void CInputLogDlg::OnBnClickedBtnGetEvent()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCheck = ((CButton*)(GetDlgItem(IDC_CHECK_LABEL)))->GetCheck();
	int nSel = m_cbInputDevices.GetCurSel();
	CString strCmd;

	strCmd = TEXT("adb shell getevent");
	if (nCheck == BST_CHECKED)
	{
		strCmd += TEXT(" -l");
	}

	if (nSel > 0)
	{
		strCmd.AppendFormat(TEXT(" /dev/input/event%d"), nSel - 1);
	}

	PARAM_T para;
	para.strCmd = strCmd;

	
	CAdbInterface::CreateCmdWindow(&para);
}



void CInputLogDlg::OnBnClickedBtnCatProcKmsg()
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell cat /proc/kmsg");


	CAdbInterface::CreateCmdWindow(&para);
}


void CInputLogDlg::OnBnClickedBtnDmesg()
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell dmesg");


	CAdbInterface::CreateCmdWindow(&para);
}


void CInputLogDlg::OnCbnSelchangeComboInput()
{
	// TODO: 在此添加控件通知处理程序代码

	int sel = m_cbInputDevices.GetCurSel();
	if (sel == 0) 
	{
		m_richInputAttrs.SetWindowTextW(m_strAllAttrs);
	}
	else 
	{
		vector<INPUT_EVENT_DEV_T>::iterator it = ADB.m_vecInputDevices.begin();
		for (; it != ADB.m_vecInputDevices.end(); it++) 
		{
			if (it->no == (sel-1))
			{
				CString strText;
				strText.Format(TEXT("/dev/input/event%d: \n"), it->no);

				vector<CString>::const_iterator cit = it->vecAttrs.begin();
				cit++;
				for (; cit != it->vecAttrs.end(); cit++)
				{
					strText += *cit;
					strText += TEXT("\n");
				}
				strText.Replace(TEXT("\n"), TEXT("\r\n"));

				m_richInputAttrs.SetWindowTextW(strText);
				return;
			}
		}


	}
}


void CInputLogDlg::OnCbnSelchangeComboLoglevels()
{
	int nCurSel = m_cbLogLevel.GetCurSel();
	
	ADB.SetPrintkLevel(nCurSel);
}


void CInputLogDlg::OnBnClickedBtnAdbLogcat()
{
	PARAM_T para;
	para.strCmd = TEXT("adb logcat");
	CString filter;
	int level;


	m_editLogcatFilter.GetWindowTextW(filter);
	level = m_cbLogcatFilterLevel.GetCurSel();
	if (!filter.IsEmpty()) {
		level = (level == -1) ? ANDROID_LOG_VERBOSE : level;
		para.strCmd.AppendFormat(TEXT(" %s:%s *:S"),
			filter, g_mapLogLevel[level].Mid(0, 1));

	}

	CAdbInterface::CreateCmdWindow(&para);
}


void CInputLogDlg::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	delete this;
}


void CInputLogDlg::OnBnClickedBtnIntRefresh()
{
	RefreshInterrupt();
}


void CInputLogDlg::OnCbnSelchangeComboIntName()
{
	SelectInterrupt(m_cbIntNames.GetCurSel());
}


void CInputLogDlg::OnCbnSelchangeComboIntCpu()
{
	SelectCpu(m_cbIntCpus.GetCurSel());
}
