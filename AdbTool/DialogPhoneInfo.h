#pragma once
#include "afxwin.h"

#define ANDROID_INFO_PROP		TEXT("Properties")
#define ANDROID_INFO_CPUS		TEXT("Cpus")
#define ANDROID_INFO_MEM		TEXT("Memories")
#define ANDROID_INFO_VERSION	TEXT("Version")
#define ANDROID_INFO_CMDLINE	TEXT("Cmdline")

enum {
	AINFO_PROP = 0,
	AINFO_CPUS,
	AINFO_MEM,
	AINFO_VERSION,
	AINFO_CMDLINE,
};
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
	void GetPhoneInfo(int type);
	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editPhoneInfo;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcDestroy();
	CComboBox m_cbInfoTypes;
	afx_msg void OnCbnSelchangeComboInfoTypes();
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
};
