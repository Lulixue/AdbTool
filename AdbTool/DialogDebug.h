#pragma once
#include "afxcmn.h"


// CDialogDebug 对话框

class CDialogDebug : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogDebug)

public:
	CDialogDebug(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogDebug();


// 对话框数据
	enum { IDD = IDD_DLG_DBG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_ctrlAdbCmd;
	CFont *m_pFont;
	LRESULT OnAppendAdbCmd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcDestroy();
};
