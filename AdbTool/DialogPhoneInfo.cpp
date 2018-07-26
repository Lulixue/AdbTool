// DialogPhoneInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogPhoneInfo.h"
#include "afxdialogex.h"
#include "AdbInterface.h"
#include <regex>


// CDialogPhoneInfo 对话框

IMPLEMENT_DYNAMIC(CDialogPhoneInfo, CDialogEx)

CDialogPhoneInfo::CDialogPhoneInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogPhoneInfo::IDD, pParent)
{

}

CDialogPhoneInfo::~CDialogPhoneInfo()
{
}

void CDialogPhoneInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PHONE_INFO, m_editPhoneInfo);
	DDX_Control(pDX, IDC_COMBO_INFO_TYPES, m_cbInfoTypes);
}


BEGIN_MESSAGE_MAP(CDialogPhoneInfo, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_NCDESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_INFO_TYPES, &CDialogPhoneInfo::OnCbnSelchangeComboInfoTypes)
	ON_WM_NCCREATE()
END_MESSAGE_MAP()


// CDialogPhoneInfo 消息处理程序
void CDialogPhoneInfo::GetPhoneInfo(int type)
{
	PARAM_T para;
	CString strText;
	
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	switch (type) {
	case AINFO_CPUS:
		para.strCmd = TEXT("adb shell cat /proc/cpuinfo");
		break;
	case AINFO_MEM:
		para.strCmd = TEXT("adb shell cat /proc/meminfo");
		break;
	case AINFO_VERSION:
		para.strCmd = TEXT("adb shell cat /proc/version");
		break;
	case AINFO_CMDLINE:
		para.strCmd = TEXT("adb shell cat /proc/cmdline");
		break;
	case AINFO_PROP:
		para.strCmd = TEXT("adb shell getprop");
		break;
	case AINFO_KEY_INFO:
		{
			strText.AppendFormat(TEXT("SerialNo: %s\n"), GetProp(GETPROP_SERIALNO));
			strText.AppendFormat(TEXT("Model: %s\n"), GetProp(GETPROP_MODEL));
			strText.AppendFormat(TEXT("Product: %s\n"), GetProp(GETPROP_PRODUCT));
			strText.AppendFormat(TEXT("SElinux: %s\n"), GetProp(GETPROP_SELINUX));
			strText.AppendFormat(TEXT("Android: %s(API:%s)\n"), GetProp(GETPROP_ANDROID_RELEASE), GetProp(GETPROP_ANDROID_SDK));
		}
	
	default:
		break;
	}
	if (type == AINFO_PROP)  {
		InitAndroidProps();
		strText = m_strGetPropLine;
	} else if (AINFO_KEY_INFO == type) {
		para.nType |= CMD_READ_BACK_FILE;
		para.strCmd = TEXT("adb shell cat /proc/version");
		CAdbInterface::CreateAdbProcess(&para);
		strText.AppendFormat(TEXT("Linux: %s\n"), GetLinuxVersion(para.strReturn));
		strText.AppendFormat(TEXT("Build Date: %s\n"), GetBuildDate(para.strReturn));
	}
	else {
		CAdbInterface::CreateAdbProcess(&para);
		strText.Append(para.strReturn);
	}
	strText.Replace(TEXT("\r"), TEXT("\n"));
	strText.Replace(TEXT("\n"), TEXT("\r\n"));
	strText.Replace(TEXT("\r\n\r\n"), TEXT("\r\n"));

	m_editPhoneInfo.SetWindowTextW(strText);
}

CString CDialogPhoneInfo::GetLinuxVersion(const CString version) const
{
	int begin = version.Find(TEXT("ersion "));
	int end = version.Find(TEXT('('));

	return version.Mid(begin+7, end-begin-7);
}

CString CDialogPhoneInfo::GetBuildDate(const CString version) const
{
	static const std::wstring weekPat(TEXT("(Mon|Tue|Wed|Thu|Fri|Sat|Sun)"));
	static const std::wstring datePat(TEXT("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)"));
	static const std::wregex dateRegex(weekPat);

	std::wstring line(version.GetString());

	std::wsmatch results;
	std::regex_search(line, results, dateRegex);
	if (results.empty()) {
		return TEXT("");
	}
	return version.Mid(results.position(0));
}

void CDialogPhoneInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	GetPhoneInfo(m_cbInfoTypes.GetCurSel());
}


void CDialogPhoneInfo::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	delete this;
}


void CDialogPhoneInfo::OnCbnSelchangeComboInfoTypes()
{
	GetPhoneInfo(m_cbInfoTypes.GetCurSel());
}

void CDialogPhoneInfo::InitAndroidProps()
{
	PARAM_T para;
	CString strText;

	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;
	para.strCmd = TEXT("adb shell getprop");
	CAdbInterface::CreateAdbProcess(&para);

	m_strGetPropLine = para.strReturn;
	m_mapAndroidProps.clear();
	CString strKey, strValue;
	CString strItem;

	int len = para.strReturn.GetLength();
	int begin, end;
	TCHAR single;
	for (int i = 0; i < len; i++)
	{
		single = para.strReturn.GetAt(i);
		if (single == TEXT('\r') ||
			single == TEXT('\n'))
		{
			if (strItem.IsEmpty()) {
				continue;
			}
			begin = strItem.Find(TEXT('['));
			end = strItem.Find(TEXT(']'));
			strKey = strItem.Mid(begin+1, end-begin-1);

			begin = strItem.ReverseFind(TEXT('['));
			end = strItem.ReverseFind(TEXT(']'));
			strValue = strItem.Mid(begin+1, end-begin-1);

			if (!strKey.IsEmpty() && !strValue.IsEmpty())
			{
				m_mapAndroidProps[strKey] = strValue;
			}
			strItem.Empty();
			continue;
		} else {
			strItem.AppendChar(single);
		}
		
	}

}

CString CDialogPhoneInfo::GetProp(const CString key) const
{
	map<CString, CString>::const_iterator cit = m_mapAndroidProps.find(key);
	if (cit != m_mapAndroidProps.end()) {
		return cit->second;
	}
	return TEXT("");
}

BOOL CDialogPhoneInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cbInfoTypes.AddString(ANDROID_INFO_KEY_INFO);
	m_cbInfoTypes.AddString(ANDROID_INFO_PROP);
	m_cbInfoTypes.AddString(ANDROID_INFO_CPUS);
	m_cbInfoTypes.AddString(ANDROID_INFO_MEM);
	m_cbInfoTypes.AddString(ANDROID_INFO_VERSION);
	m_cbInfoTypes.AddString(ANDROID_INFO_CMDLINE);
	m_cbInfoTypes.SetCurSel(0);
	InitAndroidProps();
	return TRUE;
}

BOOL CDialogPhoneInfo::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CDialogEx::OnNcCreate(lpCreateStruct))
		return FALSE;

	return TRUE;
}
