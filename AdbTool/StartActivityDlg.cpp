// StartActivityDlg.cpp : ÊµÏÖÎÄ¼þ
//

#include "stdafx.h"
#include "AdbTool.h"
#include "StartActivityDlg.h"
#include "afxdialogex.h"
#include "AdbInterface.h"

// CStartActivityDlg ¶Ô»°¿ò

IMPLEMENT_DYNAMIC(CStartActivityDlg, CDialogEx)

CStartActivityDlg::CStartActivityDlg(CString package, CWnd* pParent /*=NULL*/)
	: m_strPackageName(package), CDialogEx(CStartActivityDlg::IDD, pParent)
{

}

CStartActivityDlg::~CStartActivityDlg()
{
}

void CStartActivityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEFAULT_ACTIVITIES, m_cbDefaultActivities);
}


BEGIN_MESSAGE_MAP(CStartActivityDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_AM_STARTACT, &CStartActivityDlg::OnBnClickedBtnAmStartact)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BOOL CStartActivityDlg::StartActivity()
{
	CString strStartAct;
	m_cbDefaultActivities.GetWindowTextW(strStartAct);

	CString para;
	para.Format(TEXT("%s/%s"), m_strPackageName, strStartAct);

	BOOL bRet = ADB.StartActivity(para);
	if (!bRet) {
		MessageBox(TEXT("Failed to start activity!"), TEXT("Error"), MB_ICONSTOP);
	} else {
		MessageBox(TEXT("Activity started!"), TEXT("Info"), MB_ICONINFORMATION);
	}
	return bRet;
}

unsigned WINAPI ThreadStartActivity(LPVOID lP)
{
	CStartActivityDlg *pDlg = (CStartActivityDlg *)lP;
	
	pDlg->EnableCtrls(FALSE);
	BOOL bRet = pDlg->StartActivity();
	pDlg->EnableCtrls(TRUE);

	if (bRet)
	{
		PostMessage(pDlg->GetSafeHwnd(), WM_CLOSE, 0, 0);
	} 
	return 0;
}

void CStartActivityDlg::OnBnClickedBtnAmStartact()
{
	_beginthreadex(NULL, 0, ThreadStartActivity, (void*)this, 0, NULL);
}

void CStartActivityDlg::EnableCtrls(BOOL bEnable)
{
	static UINT CTRLS[] = {IDC_BTN_AM_STARTACT, IDC_COMBO_DEFAULT_ACTIVITIES};
	int size = ARRAYSIZE(CTRLS);

	for (int i = 0 ; i < size; i++) {
		GetDlgItem(CTRLS[i])->EnableWindow(bEnable);
	}

}
void CStartActivityDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	m_cbDefaultActivities.AddString(TEXT(".MainActivity"));
	m_cbDefaultActivities.AddString(TEXT(".FullscreenActivity"));
	m_cbDefaultActivities.SetCurSel(0);

	SetDlgItemText(IDC_EDIT_START_PACKAGE, m_strPackageName);
}
