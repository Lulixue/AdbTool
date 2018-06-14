#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <vector>
#include <map>
#include "AdbInterface.h"
using std::vector;
using std::map;

#define UMSG_UPDATE_PACKAGE_LIST (WM_USER+0xADAD)
enum {
	PKGT_SYSTEM = 0,
	PKGT_THIRD_PARTY = 1,
};

enum {
	LPPARAM_FILE = 0x01,  
	LPPARAM_THIRDPARTY = 0x02,
	LPPARAM_DISABLED = 0x04,
	LPPARAM_ENABLED = 0x08,
	LPPARAM_SYSTEM = 0x10,
	LPPARAM_UID = 0x20,
	LPPARAM_INSTALLER = 0x40,
	LPPARAM_UNINSTALLED = 0x80,
};

enum {
	PKGORDER_DEFAULT = 0x00,
	PKGORDER_NAME = (0x10 << 0),
	PKGORDER_APP = (0x10 << 1),
	PKGORDER_PATH = (0x10 << 2),
	PKGORDER_UID = (0x10 << 3),
};
enum {
	LISTPKG_BEGIN ,
	LISTPKG_PACKAGE,
	LISTPKG_UID,
	LISTPKG_END,
};

typedef struct {
	CString package;
	CString path;
	int type;
	CString UID;
} ANDROID_PACKAGE_T, *P_ANDORID_PACKAGE_T;

// CDialogPackageManager 对话框

class CDialogPackageManager : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPackageManager)

public:
	CDialogPackageManager(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogPackageManager();

// 对话框数据
	enum { IDD = IDD_DLG_PACKAGE_MANAGER };

protected:
	BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listPackages;
	afx_msg void OnBnClickedBtnInstallPackage();
	CComboBox m_cbPackageTypes;
	afx_msg void OnNcDestroy();
	void InitPackages();
	void EnableCtrls(BOOL bEnable);

	void Refresh();

private:
	BOOL GetSelectedPackage(ANDROID_PACKAGE_T &pkg) const;
	BOOL IsItemInvalid(int nItem) const { return nItem == -1;}
	void OrderPackage(vector<ANDROID_PACKAGE_T> &ordered);
	void GetPackages(int param, vector<ANDROID_PACKAGE_T> &packages);
	void UpdateList();
	CString ToParams(int param);
	ANDROID_PACKAGE_T ParsePackageLine(CString line);
public:
private:
	vector<ANDROID_PACKAGE_T> m_vecPackages;
	vector<ANDROID_PACKAGE_T> m_vec3PartyPackages;
	int m_nPackageOrder;
	int m_nCurrentSelectItem;
	CToolTipCtrl m_toolTip;
protected:
	afx_msg LRESULT OnUmsgUpdatePackageList(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLvnColumnclickListPackages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListPackages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnUninstallPackage();
	afx_msg void OnPackageoperationsForcestop();
	afx_msg void OnPackageoperationsUninstall();
	afx_msg void OnNMRClickListPackages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnHotTrackListPackages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnPackagesRefresh();
	afx_msg void OnPackageoperationsStartactivity();
};

BOOL ForceStopPackage(HWND hwnd, CString package);
BOOL UninstallPackage(HWND hwnd, CString package);