// DialogDebug.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogDebug.h"
#include "afxdialogex.h"
#include "AdbInterface.h"

HWND g_hwndDebug;

// CDialogDebug 对话框

IMPLEMENT_DYNAMIC(CDialogDebug, CDialogEx)

CDialogDebug::CDialogDebug(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogDebug::IDD, pParent), m_pFont(NULL)
{

}

CDialogDebug::~CDialogDebug()
{
	if (m_pFont)
	{
		delete m_pFont;
	}
}

void CDialogDebug::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21_ADB_CMD, m_ctrlAdbCmd);
}


BEGIN_MESSAGE_MAP(CDialogDebug, CDialogEx)
	ON_MESSAGE(MSG_APPEND_ADB_CMD, &CDialogDebug::OnAppendAdbCmd)
	ON_WM_SHOWWINDOW()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CDialogDebug 消息处理程序


LRESULT CDialogDebug::OnAppendAdbCmd(WPARAM wParam, LPARAM lParam)
{
	CString *pLog = (CString *)wParam;

	CString strNewText;
	m_ctrlAdbCmd.GetWindowTextW(strNewText);
	if ((int)lParam == 0) {
		strNewText += TEXT("\r\n> ");
	}
	strNewText += *pLog;
	strNewText += TEXT("\r\n");

	delete pLog;
	m_ctrlAdbCmd.SetWindowTextW(strNewText.GetString());
	m_ctrlAdbCmd.PostMessageW(WM_VSCROLL, SB_BOTTOM, 0);
	return 0;
}

void CDialogDebug::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	
	if (m_pFont == NULL) {
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT)); 
		wcscpy_s(lf.lfFaceName, TEXT("微软雅黑"));
		lf.lfQuality = CLEARTYPE_QUALITY; 

		lf.lfHeight         = 100;
		lf.lfWeight         = 15;
		lf.lfWidth          = 40 ;
		lf.lfEscapement     = 0 ;
		lf.lfOrientation      = 0 ;
		lf.lfItalic           = 0 ;
		lf.lfUnderline       = 0 ;
		lf.lfStrikeOut       = 0 ;
		lf.lfCharSet        = DEFAULT_CHARSET ;
		lf.lfOutPrecision    = 0 ;
		lf.lfClipPrecision    = 0 ;
		lf.lfPitchAndFamily  = 0 ;

		m_pFont = new CFont();
		m_pFont->CreatePointFontIndirect(&lf);

		m_ctrlAdbCmd.SetFont(m_pFont);
	}
	// TODO: 在此处添加消息处理程序代码
	g_hwndDebug = this->GetSafeHwnd();
}


void CDialogDebug::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	delete this;
}
