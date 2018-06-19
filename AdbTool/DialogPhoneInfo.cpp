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
}


BEGIN_MESSAGE_MAP(CDialogPhoneInfo, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CDialogPhoneInfo 消息处理程序
void CDialogPhoneInfo::GetPhoneInfo()
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell getprop");
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	CAdbInterface::CreateAdbProcess(&para);

	m_editPhoneInfo.SetWindowTextW(para.strReturn);
}

void CDialogPhoneInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	GetPhoneInfo();
}


void CDialogPhoneInfo::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	delete this;
}
