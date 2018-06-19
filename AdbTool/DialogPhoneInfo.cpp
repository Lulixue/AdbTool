// DialogPhoneInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogPhoneInfo.h"
#include "afxdialogex.h"
#include "AdbInterface.h"


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
	default:
		para.strCmd = TEXT("adb shell getprop");
		break;
	}
	CAdbInterface::CreateAdbProcess(&para);

	m_editPhoneInfo.SetWindowTextW(para.strReturn);
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

BOOL CDialogPhoneInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cbInfoTypes.AddString(ANDROID_INFO_PROP);
	m_cbInfoTypes.AddString(ANDROID_INFO_CPUS);
	m_cbInfoTypes.AddString(ANDROID_INFO_MEM);
	m_cbInfoTypes.AddString(ANDROID_INFO_VERSION);
	m_cbInfoTypes.AddString(ANDROID_INFO_CMDLINE);
	m_cbInfoTypes.SetCurSel(0);

	return TRUE;
}

BOOL CDialogPhoneInfo::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CDialogEx::OnNcCreate(lpCreateStruct))
		return FALSE;

	return TRUE;
}
