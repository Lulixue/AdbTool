#pragma once
#include "afxcmn.h"
#include "AdbFileManager.h"
#include <map>
#include "afxwin.h"
using std::map;
using std::multimap;

#define UMSG_UPDATE_FILE_LIST	(WM_USER + 0x4544)
#define UMSG_UPDATE_TREE_LIST		(WM_USER + 0x4532)
#define UMSG_REFRESH_PARENT		(WM_USER + 0x4535)
#define UMSG_FILE_MANAGER_CLOSE (WM_USER + 0xADBC)
// CDlgFileManager 对话框
enum {
	FORDER_DEFAULT = 0x00,
	FORDER_NAME = (0x10 << 0),
	FORDER_OWNER = (0x10 << 1),
	FORDER_MODE = (0x10 << 2),
	FORDER_SIZE = (0x10 << 3),
	FORDER_TIME = (0x10 << 4),
};
struct LIST_FILE_ITEM_T {
	LIST_FILE_ITEM_T() {}
	LIST_FILE_ITEM_T(CAndroidFile *pf, HTREEITEM item) 
		:  file(*pf), titem(item) {}
	CAndroidFile file;
	HTREEITEM titem;
};

class CDlgFileManager : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFileManager)

public:
	CDlgFileManager(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFileManager();

// 对话框数据
	enum { IDD = IDD_DLG_FILE_MANAGER };
private:
	static const CString REDIR_FILE_SYSTEM;
	static const CString REDIR_SDCARD;
	static const CString REDIR_EXT_SDCARD;
	void AddComboFastRedir(CString strItem);
	void AddFastRedirect(CAndroidFile *pFile, HTREEITEM item);
	void OrderFiles();
	void UpdateList();
	void UpdateStatistic();
	void UpdateCtrls(HTREEITEM item, BOOL bUpdateList = TRUE, BOOL bForceUpdate = TRUE);
	void InitTree();
	int ToImage(CAndroidFile *file) const;
	CAndroidFile* GetFile(HTREEITEM item);
	HTREEITEM* FileToItem(CAndroidFile *file);
	FileIndex* GetIndexes(HTREEITEM item);
	DWORD_PTR ToItemData(HTREEITEM item) const;
	void ExpandTreeItem(HTREEITEM item);
	void AddListItem(const CAndroidFile *file, HTREEITEM item, BOOL bStatistic = FALSE);
	BOOL ImportFile(CString strSrcPath);
	BOOL DeleteAndroidFile(CAndroidFile *pFile);
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	BOOL CDlgFileManager::OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	BOOL IsItemInvalid(int item) const { return item < 1/*== -1*/;}
	void ExpandFile(CAndroidFile *file, int depth);
	void SetCurrentItem(HTREEITEM item);
	void Update(HTREEITEM item);
	void EnableButtons(BOOL bEnable);
	void EnableCtrls(BOOL bEnable);
	CTreeCtrl m_treeFileSystem;
	CListCtrl m_listDirectory;
private:
	int		m_nFileOrder;
	int		m_nLastItem;
	CToolTipCtrl m_toolTip;
	HTREEITEM m_itemTreeCurrent;
	CAndroidFile *m_pCurrentFile;
	vector<FILE_COLUMN_T> m_vecColumns;
	map<CString, HTREEITEM> m_mapFastRedirect;
	CAdbFileManager m_fileManager;
	CImageList m_imgList;
	// 指针对应的, CAdbFileManager depth和subfiles下标
	map<HTREEITEM, FileIndex> m_mapItemIndex; 
	vector<LIST_FILE_ITEM_T> m_vecListFiles;
	vector<LIST_FILE_ITEM_T> m_vecListStaticsItems;
public:
	afx_msg void OnTvnSelchangedTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnUmsgUpdateList(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNcDestroy();
	afx_msg void OnHdnItemdblclickListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_cbFastRedirect;
	afx_msg void OnBnClickedBtnManagerBack();
	afx_msg void OnBnClickedBtnImportFile();
	afx_msg void OnBnClickedBtnExportFile();
	CButton m_btnExportFile;
	CButton m_btnImportFile;
	afx_msg void OnImporttypeFile();
	afx_msg void OnImporttypeDirectory();

	afx_msg void OnNMRClickListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDirreportmenuExport();
	afx_msg void OnDirreportmenuDelete();
	afx_msg void OnNMClickListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnUmsgRefreshList(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnCbnSelchangeComboCurrentPath();
	afx_msg void OnLvnGetInfoTipListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnHotTrackListDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnRemount();
protected:
	afx_msg LRESULT OnUmsgUpdateFileList(WPARAM wParam, LPARAM lParam);
};

BOOL SelectDir(HWND hWnd, CString &strPath, BOOL bCreateNew = TRUE);
BOOL OpenFile(HWND hWnd, CString &strPath);