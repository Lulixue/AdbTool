#pragma once
#include "afxwin.h"


// CDialogPhoneInfo �Ի���

class CDialogPhoneInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPhoneInfo)

public:
	CDialogPhoneInfo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogPhoneInfo();

// �Ի�������
	enum { IDD = IDD_DIALOG_PHONE_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	void GetPhoneInfo();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editPhoneInfo;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcDestroy();
};
