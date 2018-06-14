#pragma once
#include "afxcmn.h"


// CDialogDebug �Ի���

class CDialogDebug : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogDebug)

public:
	CDialogDebug(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogDebug();


// �Ի�������
	enum { IDD = IDD_DLG_DBG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_ctrlAdbCmd;
	CFont *m_pFont;
	LRESULT OnAppendAdbCmd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcDestroy();
};
