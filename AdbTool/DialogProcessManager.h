#pragma once
#include "afxcmn.h"
#include "AdbProcessManager.h"

#define UMSG_REFRESH_ANDROID_PROCESS	(WM_USER + 0xE56D)
// CDialogProcessManager 对话框

enum {
	PROCORDER_DEFAULT = 0x00,
	PROCORDER_PID = (0x10 << 0),
	PROCORDER_USER = (0x10 << 1),
	PROCORDER_NAME = (0x10 << 2),
	PROCORDER_PPID = (0x10 << 3),
	PROCORDER_STATUS = (0x10 << 4),
	PROCORDER_MEMORY = (0x10 << 5),
	PROCORDER_VIRUTAL = (0x10 << 6),
};


class CDialogProcessManager : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogProcessManager)

public:
	CDialogProcessManager(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogProcessManager();

// 对话框数据
	enum { IDD = IDD_DIALOG_PROCESS_MANAGER };

protected:
	BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	BOOL GetProcessSeleted(CAndroidProcess &process) const;
	void OrderProcesses();
	void AddListItem(const CAndroidProcess &process, BOOL bStatistic = FALSE);
	CString ItemToPID(HTREEITEM item);
	void Refresh();
	void UpdateCtrls();
	void RemoveSubitems(HTREEITEM pitem);
	BOOL IsThreadItemSelected() const;
	BOOL IsPackageItemSelected() const;
	BOOL IsChildOf(HTREEITEM child, HTREEITEM parent) const;
	void SelectThreadItem(CString PID);
	void SelectTreeItem(HTREEITEM item);
	void UpdateList();
	BOOL IsItemInvalid(int nItem) const { return nItem <= -1;}
private:
	vector<CAndroidProcess> m_vecListProcesses;
	vector<CAndroidProcess> m_vecListStatistic;
	vector<FILE_COLUMN_T> m_vecColumns;
	CTreeCtrl m_treeProcesses;
	CListCtrl m_listProcesses;
	CAdbProcessManager m_managerProcess;
	HTREEITEM m_itemtreeAllProcess;
	HTREEITEM m_itemtreeTreeProcess;
	HTREEITEM m_itemtreePackgeProcess;
	HTREEITEM m_itemtreeSelected;
	int m_nCurrentSelectItem;
	int m_nProcessOrder;
	CToolTipCtrl m_toolTip;
public:
	void PostRefreshMsg();
	void LoadProcess();
	void EnableCtrls(BOOL bEnable);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
public:
	afx_msg void OnNcDestroy();
public:
	afx_msg void OnPaint();
	afx_msg void OnTvnSelchangedTreeProcesses(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandedTreeProcesses(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListProcesses(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcesssubmenuKill();
protected:
	afx_msg LRESULT OnUmsgRefreshAndroidProcess(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNMRClickListProcesses(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnHotTrackListProcesses(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcesssubmenuRefresh();
	afx_msg void OnPackageoperationsForcestop();
	afx_msg void OnPackageoperationsStartactivity();
	afx_msg void OnPackageoperationsUninstall();
	afx_msg void OnNMDblclkListProcesses(NMHDR *pNMHDR, LRESULT *pResult);
};

BOOL ForceStopPackage(HWND hwnd, CString package);
BOOL UninstallPackage(HWND hwnd, CString package);