// DialogPackageManager.cpp : ÊµÏÖÎÄ¼þ
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogPackageManager.h"
#include "afxdialogex.h"
#include "StartActivityDlg.h"
#include <cmath>
using std::multimap;

extern CDialogPackageManager *g_pDlgPackageManager;
// CDialogPackageManager ¶Ô»°¿ò

IMPLEMENT_DYNAMIC(CDialogPackageManager, CDialogEx)

CDialogPackageManager::CDialogPackageManager(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogPackageManager::IDD, pParent)
{

}

CDialogPackageManager::~CDialogPackageManager()
{
    TRACE("%s\n", __FUNCTION__);
}

void CDialogPackageManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PACKAGES, m_listPackages);
	DDX_Control(pDX, IDC_COMBO_PACKAGES_TYPES, m_cbPackageTypes);
}


BEGIN_MESSAGE_MAP(CDialogPackageManager, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_INSTALL_PACKAGE, &CDialogPackageManager::OnBnClickedBtnInstallPackage)
	ON_WM_NCDESTROY()
	ON_MESSAGE(UMSG_UPDATE_PACKAGE_LIST, &CDialogPackageManager::OnUmsgUpdatePackageList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PACKAGES, &CDialogPackageManager::OnLvnColumnclickListPackages)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PACKAGES, &CDialogPackageManager::OnNMClickListPackages)
	ON_BN_CLICKED(IDC_BTN_UNINSTALL_PACKAGE, &CDialogPackageManager::OnBnClickedBtnUninstallPackage)
	ON_COMMAND(ID_PACKAGEOPERATIONS_FORCESTOP, &CDialogPackageManager::OnPackageoperationsForcestop)
	ON_COMMAND(ID_PACKAGEOPERATIONS_UNINSTALL, &CDialogPackageManager::OnPackageoperationsUninstall)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PACKAGES, &CDialogPackageManager::OnNMRClickListPackages)
	ON_NOTIFY(LVN_HOTTRACK, IDC_LIST_PACKAGES, &CDialogPackageManager::OnLvnHotTrackListPackages)
	ON_BN_CLICKED(IDC_BTN_PACKAGES_REFRESH, &CDialogPackageManager::OnBnClickedBtnPackagesRefresh)
	ON_COMMAND(ID_PACKAGEOPERATIONS_STARTACTIVITY, &CDialogPackageManager::OnPackageoperationsStartactivity)
END_MESSAGE_MAP()


unsigned WINAPI ThreadInitPackages(LPVOID lP)
{
	CDialogPackageManager *pDlg = (CDialogPackageManager *)lP;
	pDlg->EnableCtrls(FALSE);
	pDlg->InitPackages();
	pDlg->EnableCtrls(TRUE);

	return 0;
}


BOOL CDialogPackageManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	m_toolTip.EnableToolTips();  
	m_toolTip.Create(this, TTS_ALWAYSTIP);  
	m_toolTip.Activate(TRUE);  
	m_toolTip.SetDelayTime(TTDT_INITIAL, 0);  

	m_listPackages.InsertColumn(0, TEXT("Package"));
	m_listPackages.InsertColumn(1, TEXT("App"));
	m_listPackages.InsertColumn(2, TEXT("Path"));
	m_listPackages.InsertColumn(3, TEXT("UID"));
	m_listPackages.SetColumnWidth(0, 160);
	m_listPackages.SetColumnWidth(1, 50);
	m_listPackages.SetColumnWidth(2, 200);
	m_listPackages.SetColumnWidth(3, 50);

	DWORD dwStyle = m_listPackages.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//Ñ¡ÖÐÄ³ÐÐÊ¹ÕûÐÐ¸ßÁÁ£¨Ö»ÊÊÓÃÓëreport·ç¸ñµÄlistctrl£©
	dwStyle |= LVS_EX_GRIDLINES;//Íø¸ñÏß£¨Ö»ÊÊÓÃÓëreport·ç¸ñµÄlistctrl£©
	m_listPackages.SetExtendedStyle(dwStyle); //ÉèÖÃÀ©Õ¹·ç¸ñ

	m_cbPackageTypes.AddString(TEXT("Packages"));
	m_cbPackageTypes.SetCurSel(0);

	_beginthreadex(NULL, 0, ThreadInitPackages, (void*)this, 0, NULL);
	return TRUE;
}

void ThreadInstallApk(LPVOID lP)
{
	CString *pStrPath = (CString *)lP;

	CString strTitle;
	CString strInfo;
	g_pDlgPackageManager->GetWindowTextW(strTitle);
	strInfo = strTitle;
	strInfo += TEXT(" (Installing...)");
	g_pDlgPackageManager->SetWindowTextW(strInfo);

	int nPos = pStrPath->ReverseFind(TEXT('\\'));
	CString strFileName = pStrPath->Mid(nPos+1);

	g_pDlgPackageManager->EnableCtrls(FALSE);
	if (ADB.InstallApk(*pStrPath))
	{
		g_pDlgPackageManager->SetWindowTextW(strTitle);

		MessageBox(g_pDlgPackageManager->GetSafeHwnd(), strFileName + TEXT(" °²×°³É¹¦!"), TEXT("ÏûÏ¢"), MB_ICONINFORMATION);
	}
	else 
	{
		g_pDlgPackageManager->SetWindowTextW(strTitle);

		CString strError = strFileName;
		strError += TEXT(" °²×°Ê§°Ü!\r\nÏêÏ¸ÐÅÏ¢: ");
		strError += ADB.GetAdbLastError();

		::MessageBox(g_pDlgPackageManager->GetSafeHwnd(), strError, TEXT("´íÎó"), MB_ICONERROR);
	}
	delete pStrPath;
	g_pDlgPackageManager->EnableCtrls(TRUE);

	_beginthreadex(NULL, 0, ThreadInitPackages, (void*)g_pDlgPackageManager, 0, NULL);
}


void CDialogPackageManager::OnBnClickedBtnInstallPackage()
{

	CFileDialog fileDlgApk(TRUE, TEXT("Android App"),
		NULL, OFN_OVERWRITEPROMPT |OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
		TEXT("Android App|*.apk|"), this);

	wchar_t buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	CString strCurDir = buffer;
	int pos = strCurDir.ReverseFind(TEXT('\\'));
	strCurDir = strCurDir.Mid(0, pos);
	strCurDir += TEXT("\\apks");

	fileDlgApk.m_ofn.lpstrInitialDir = strCurDir;

	if (fileDlgApk.DoModal() != IDOK) {
		return;
	}

	CString strPath = fileDlgApk.GetPathName();

	_beginthread(ThreadInstallApk, 0, (LPVOID)(new CString(strPath)));
}

CString CDialogPackageManager::ToParams(int nParam)
{
	CString params;
	if (nParam & LPPARAM_DISABLED) {
		params.Append(TEXT("-d "));
	}
	else if (nParam & LPPARAM_ENABLED) {
		params.Append(TEXT("-e "));
	}
	if (nParam & LPPARAM_FILE) {
		params.Append(TEXT("-f "));
	}
	if (nParam & LPPARAM_INSTALLER) {
		params.Append(TEXT("-i "));
	}
	if (nParam & LPPARAM_SYSTEM) {
		params.Append(TEXT("-s "));
	} else if (nParam & LPPARAM_THIRDPARTY) {
		params.Append(TEXT("-3 "));
	}
	if (nParam & LPPARAM_UID) {
		params.Append(TEXT("-U "));
	}
	if (nParam & LPPARAM_UNINSTALLED) {
		params.Append(TEXT("-u "));
	}

	return params;
}

void CDialogPackageManager::GetPackages(int param, vector<ANDROID_PACKAGE_T> &packages)
{
	packages.clear();

	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell pm list packages %s"), ToParams(param));
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	if (!para.bRet) {
		return;
	}
	if (para.strReturn.Find(TEXT("Unknown option")) != -1) {
		GetPackages(param & ~LPPARAM_UID, packages);
		return;
	}

	int length = para.strReturn.GetLength();
	wchar_t single;
	CString strLine;
	for (int i = 0; i < length; i++) {
		single = para.strReturn.GetAt(i);

		if ((single != TEXT('\n')) &&
			(single != TEXT('\r')))
		{
			strLine.AppendChar(single);
		}

		if ((single == TEXT('\n')) ||
			(single == TEXT('\r')) ||
			(i == (length-1)))
		{
			if (strLine.IsEmpty()) {
				continue;
			}
			packages.push_back(ParsePackageLine(strLine));
			strLine.Empty();
		} 
	}

}
/* package:/system/app/CaptivePortalLogin/CaptivePortalLogin.apk=com.android.captiv
eportallogin uid:10038
 */
ANDROID_PACKAGE_T CDialogPackageManager::ParsePackageLine(CString line)
{
	ANDROID_PACKAGE_T apt;
	apt.type = PKGT_SYSTEM;

	int length = line.GetLength();
	wchar_t single;
	CString part;
	for (int i = 0; i < length; i++) {
		single = line.GetAt(i);

		if (single != TEXT(' ')) {
			part.AppendChar(single);
		}

		if ((single == TEXT(' '))||
			(i == (length-1)))
		{
			if (part.IsEmpty()) {
				continue;
			}
			int pos = part.Find(TEXT("package:"));
			if (pos == 0) {
				int endPos = part.Find(TEXT("apk="));
				if (endPos == -1) {
					apt.package = part.Mid(pos+8);
				} else {
					endPos+=3;
					apt.package = part.Mid(endPos+1);
					apt.path = part.Mid(pos+8, endPos-pos-8);
				}
				part.Empty();
				continue;
			}
			pos = part.Find(TEXT("uid:"));
			if (pos == 0) {
				apt.UID = (part.Mid(pos+4));
				part.Empty();
				continue;
			}
		} 
	}
	return apt;
}

void CDialogPackageManager::EnableCtrls(BOOL bEnable)
{
	static UINT CTRLS[] = {
		IDC_BTN_INSTALL_PACKAGE, IDC_BTN_UNINSTALL_PACKAGE,
		IDC_COMBO_PACKAGES_TYPES, IDC_LIST_PACKAGES, IDC_BTN_PACKAGES_REFRESH,
	};
	static int size = ARRAYSIZE(CTRLS);
	for (int i = 0; i < size; i++) {
		GetDlgItem(CTRLS[i])->EnableWindow(bEnable);
	}
}
	
void CDialogPackageManager::InitPackages()
{
	GetPackages(LPPARAM_FILE | LPPARAM_UID, m_vecPackages);
	GetPackages(LPPARAM_THIRDPARTY | LPPARAM_UID, m_vec3PartyPackages);
	m_nPackageOrder = PKGORDER_DEFAULT;

	vector<ANDROID_PACKAGE_T>::iterator ait = m_vecPackages.begin();
	vector<ANDROID_PACKAGE_T>::const_iterator subcit;
	for (; ait != m_vecPackages.end(); ait++) 
	{
		subcit = m_vec3PartyPackages.begin();
		for (; subcit != m_vec3PartyPackages.end(); subcit++)
		{
			if (ait->package == subcit->package)
			{
				ait->type = PKGT_THIRD_PARTY;
				break;
			}
		}
	}

	PostMessage(UMSG_UPDATE_PACKAGE_LIST);
}
void CDialogPackageManager::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	delete this;
}

void CDialogPackageManager::OrderPackage(vector<ANDROID_PACKAGE_T> &ordered)
{
	multimap<CString, ANDROID_PACKAGE_T> namePathUidPackges;
	multimap<int, ANDROID_PACKAGE_T> appPackages;

	vector<ANDROID_PACKAGE_T>::const_iterator cit = m_vecPackages.begin();
	switch (m_nPackageOrder & (~ASCEND_SUFFIX))
	{
	case PKGORDER_UID:
		for (; cit != m_vecPackages.end(); cit++) {
			namePathUidPackges.insert(std::make_pair(cit->UID, *cit));
		}
		break;
	case PKGORDER_PATH:
		for (; cit != m_vecPackages.end(); cit++) {
			namePathUidPackges.insert(std::make_pair(cit->path, *cit));
		}
		break;
	case PKGORDER_APP:
		for (; cit != m_vecPackages.end(); cit++) {
			appPackages.insert(std::make_pair(cit->type, *cit));
		}
		ordered.clear();
		if (ORDER_IS_ASCEND(m_nPackageOrder)) {
			multimap<int, ANDROID_PACKAGE_T>::const_iterator appCit = appPackages.begin();
			for (; appCit != appPackages.end(); appCit++) {
				ordered.push_back(appCit->second);
			}
		} else {
			multimap<int, ANDROID_PACKAGE_T>::const_reverse_iterator appCit = appPackages.rbegin();
			for (; appCit != appPackages.rend(); appCit++) {
				ordered.push_back(appCit->second);
			}
		}
		return;
	case PKGORDER_NAME:
		for (; cit != m_vecPackages.end(); cit++) {
			namePathUidPackges.insert(std::make_pair(cit->package, *cit));
		}
		break;
	default:
		return;
	}
	ordered.clear();
	if (ORDER_IS_ASCEND(m_nPackageOrder)) {
		multimap<CString, ANDROID_PACKAGE_T>::const_iterator npuCit = namePathUidPackges.begin();
		for (; npuCit != namePathUidPackges.end(); npuCit++) {
			ordered.push_back(npuCit->second);
		}
	} else {
		multimap<CString, ANDROID_PACKAGE_T>::const_reverse_iterator npuCit = namePathUidPackges.rbegin();
		for (; npuCit != namePathUidPackges.rend(); npuCit++) {
			ordered.push_back(npuCit->second);
		}
	}
}

void CDialogPackageManager::UpdateList()
{

	m_listPackages.DeleteAllItems();

	OrderPackage(m_vecPackages);
	vector<ANDROID_PACKAGE_T>::const_iterator cit = m_vecPackages.begin();

	for (; cit != m_vecPackages.end(); cit++) 
	{
		int countList = 0;
		int nRow;
		int list = 0;
		countList = m_listPackages.GetItemCount();
		nRow = m_listPackages.InsertItem(countList, cit->package);
		m_listPackages.SetItemText(nRow, ++list, cit->type == PKGT_THIRD_PARTY ? TEXT("User") : TEXT("System"));
		m_listPackages.SetItemText(nRow, ++list, cit->path);

		m_listPackages.SetItemText(nRow, ++list, cit->UID);
	}
	m_nCurrentSelectItem = -1;
	m_cbPackageTypes.ResetContent();
	CString pkgInfo;
	pkgInfo.Format(TEXT("Packages(%d)"), m_vecPackages.size());
	m_cbPackageTypes.AddString(pkgInfo);
	m_cbPackageTypes.SetCurSel(0);
}

afx_msg LRESULT CDialogPackageManager::OnUmsgUpdatePackageList(WPARAM wParam, LPARAM lParam)
{
	UpdateList();
	return 0;
}


void CDialogPackageManager::OnLvnColumnclickListPackages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->iSubItem == -1) {
		return;
	}

	int orderHigh = (0x10 << pNMLV->iSubItem);
	if (m_nPackageOrder & orderHigh) {
		m_nPackageOrder = ORDER_IS_ASCEND(m_nPackageOrder) ? 
							orderHigh : (orderHigh | ASCEND_SUFFIX);
	} else {
		m_nPackageOrder = orderHigh;
	}
	TRACE("Order: %02X\n", m_nPackageOrder);

	PostMessage(UMSG_UPDATE_PACKAGE_LIST);
	*pResult = 0;
}


void CDialogPackageManager::OnNMClickListPackages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ÔÚ´ËÌí¼Ó¿Ø¼þÍ¨Öª´¦Àí³ÌÐò´úÂë
	m_nCurrentSelectItem = pNMItemActivate->iItem;
	*pResult = 0;
}


void CDialogPackageManager::OnBnClickedBtnUninstallPackage()
{
	OnPackageoperationsUninstall();
}

BOOL UninstallPackage(HWND hwnd, CString package)
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell pm uninstall %s"), package);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	if (!para.bRet) {
		return FALSE;
	}
	
	CString info;
	if (para.strReturn.Find(TEXT("Success")) != -1) {
		info.Format(TEXT("Uninstalled: %s"), package);
		::MessageBox(hwnd, info, TEXT("Info"), MB_ICONINFORMATION);
		return TRUE;
	}
	info.Format(TEXT("Uninstall(%s) failed: %s"), package, para.strReturn);
	::MessageBox(hwnd, info, TEXT("Error"), MB_ICONSTOP);
	return FALSE;
}

BOOL ForceStopPackage(HWND hwnd, CString package)
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell am force-stop %s"), package);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	if (!para.bRet) {
		return FALSE;
	}
	CString info;
	if (para.strReturn.IsEmpty()) {
		info.Format(TEXT("Force Stopped: %s"), package);
		MessageBox(hwnd, info, TEXT("Info"), MB_ICONINFORMATION);
		return TRUE;
	}
	info.Format(TEXT("Failed to force-stop(%s): %s"), package, para.strReturn);
	MessageBox(hwnd, info, TEXT("Error"), MB_ICONSTOP);
	return FALSE;

}

BOOL CDialogPackageManager::GetSelectedPackage(ANDROID_PACKAGE_T &pkg) const
{
	if (IsItemInvalid(m_nCurrentSelectItem)) {
		return FALSE;
	}
	pkg = m_vecPackages[m_nCurrentSelectItem];
	return TRUE;
}

void CDialogPackageManager::OnPackageoperationsForcestop()
{
	ANDROID_PACKAGE_T apt;
	if (!GetSelectedPackage(apt)) {
		return;
	}
	CString info;
	info.Format(TEXT("Force Stop:%s?"), apt.package);
	int nRet = MessageBox(info, TEXT("Warning"), MB_ICONQUESTION | MB_YESNOCANCEL);
	if (nRet != IDYES) {
		return;
	}
	ForceStopPackage(GetSafeHwnd(), apt.package);
}

void CDialogPackageManager::Refresh()
{
	_beginthreadex(NULL, 0, ThreadInitPackages, (void*)this, 0, NULL);
}

void CDialogPackageManager::OnPackageoperationsUninstall()
{
	ANDROID_PACKAGE_T apt;
	if (!GetSelectedPackage(apt)) {
		return;
	}
	CString info;
	info.Format(TEXT("Sure to uninstall:%s?"), apt.package);
	int nRet = MessageBox(info, TEXT("Warning"), MB_ICONQUESTION | MB_YESNOCANCEL);
	if (nRet != IDYES) {
		return;
	}
	UninstallPackage(GetSafeHwnd(), apt.package);
	Refresh();
}


void CDialogPackageManager::OnNMRClickListPackages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (IsItemInvalid(pNMItemActivate->iItem)) {
		 return;
	}
	m_nCurrentSelectItem = pNMItemActivate->iItem;
	CMenu menu; //¶¨ÒåÏÂÃæÒªÓÃµ½µÄcmenu¶ÔÏó
	menu.LoadMenu(IDR_MENU_PACKAGE_OPERATIONS); //×°ÔØ×Ô¶¨ÒåµÄÓÒ¼ü²Ëµ¥ 
	CMenu *pPopup = menu.GetSubMenu(0); //»ñÈ¡µÚÒ»¸öµ¯³ö²Ëµ¥£¬ËùÒÔµÚÒ»¸ö²Ëµ¥±ØÐëÓÐ×Ó²Ëµ¥

	CPoint point1;//¶¨ÒåÒ»¸öÓÃÓÚÈ·¶¨¹â±êÎ»ÖÃµÄÎ»ÖÃ 
	GetCursorPos(&point1);//»ñÈ¡µ±Ç°¹â±êµÄÎ»ÖÃ£¬ÒÔ±ãÊ¹µÃ²Ëµ¥¿ÉÒÔ¸úËæ¹â±ê 

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point1.x,point1.y, this);//ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾µ¯³ö²Ëµ¥


	*pResult = 0;
}

BOOL CDialogPackageManager::PreTranslateMessage(MSG* pMsg)
{
	UINT ctrlId = GetWindowLong(pMsg->hwnd,GWL_ID);
	if (pMsg->message == WM_MOUSEMOVE) {
		if (ctrlId == IDC_LIST_PACKAGES) {
			if (!IsItemInvalid(m_nCurrentSelectItem)) {
				m_toolTip.RelayEvent(pMsg);
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDialogPackageManager::OnLvnHotTrackListPackages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ÔÚ´ËÌí¼Ó¿Ø¼þÍ¨Öª´¦Àí³ÌÐò´úÂë
	int nItem = pNMLV->iItem;

	if (IsItemInvalid(nItem) || (nItem >= m_listPackages.GetItemCount())) {
		m_nCurrentSelectItem = -1;
		*pResult = 0;
		return;
	}

	if (m_nCurrentSelectItem != nItem) {
		Sleep(100);
		m_nCurrentSelectItem = nItem;
		ANDROID_PACKAGE_T apt;
		if (!GetSelectedPackage(apt)) {
			return;
		}

		CString strInfo;
		strInfo.Format(TEXT("%s(%s)"), apt.package, apt.path);
		m_toolTip.AddTool(GetDlgItem(IDC_LIST_PACKAGES), strInfo.GetBuffer());
		m_toolTip.Pop();
	}

	*pResult = 0;
}


void CDialogPackageManager::OnBnClickedBtnPackagesRefresh()
{
	Refresh();
}


void CDialogPackageManager::OnPackageoperationsStartactivity()
{
	ANDROID_PACKAGE_T apt;
	if (!GetSelectedPackage(apt)) {
		return;
	}

	CStartActivityDlg dlg(apt.package, this);
	dlg.DoModal();
}
