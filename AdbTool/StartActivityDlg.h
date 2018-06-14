#pragma once
#include "afxwin.h"


// CStartActivityDlg 对话框

class CStartActivityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStartActivityDlg)

public:
	CStartActivityDlg(CString package, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStartActivityDlg();

// 对话框数据
	enum { IDD = IDD_DLG_AM_START_ACTIVITY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbDefaultActivities;
	afx_msg void OnBnClickedBtnAmStartact();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void EnableCtrls(BOOL bEnable);
	BOOL StartActivity();
private:
	CString m_strPackageName;
};
