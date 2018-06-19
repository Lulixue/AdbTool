#pragma once
#include "afxwin.h"


// CDialogPhoneInfo 对话框

class CDialogPhoneInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPhoneInfo)

public:
	CDialogPhoneInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogPhoneInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_PHONE_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	void GetPhoneInfo();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editPhoneInfo;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcDestroy();
};
