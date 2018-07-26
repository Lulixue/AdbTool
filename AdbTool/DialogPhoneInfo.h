#pragma once
#include "afxwin.h"
#include <map>

using std::map;

#define ANDROID_INFO_KEY_INFO	TEXT("KeyInfo")
#define ANDROID_INFO_PROP		TEXT("Properties")
#define ANDROID_INFO_CPUS		TEXT("Cpus")
#define ANDROID_INFO_MEM		TEXT("Memories")
#define ANDROID_INFO_VERSION	TEXT("Version")
#define ANDROID_INFO_CMDLINE	TEXT("Cmdline")

#define GETPROP_ANDROID_RELEASE	TEXT("ro.build.version.release")
#define GETPROP_ANDROID_SDK		TEXT("ro.build.version.sdk")
#define GETPROP_PRODUCT			TEXT("ro.build.product")
#define GETPROP_BUILD_TYPE		TEXT("ro.build.type")
#define GETPROP_MODEL			TEXT("ro.product.model")
#define GETPROP_SERIALNO		TEXT("ro.boot.serialno")
#define GETPROP_SELINUX			TEXT("ro.boot.selinux")


enum {
	AINFO_KEY_INFO = 0,
	AINFO_PROP,
	AINFO_CPUS,
	AINFO_MEM,
	AINFO_VERSION,
	AINFO_CMDLINE,
};
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

private:
	CString GetLinuxVersion(const CString version) const;
	CString GetBuildDate(const CString version) const;
	CString GetProp(const CString key) const;
	void InitAndroidProps();
	map<CString, CString> m_mapAndroidProps;
	CString m_strGetPropLine;
};
