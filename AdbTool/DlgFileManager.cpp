// DlgFileManager.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DlgFileManager.h"
#include "afxdialogex.h"
#include <ShlObj.h>//必须包含此头文件  


static CDlgFileManager *g_dlgCurrent;

IMPLEMENT_DYNAMIC(CDlgFileManager, CDialogEx)

CDlgFileManager::CDlgFileManager(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgFileManager::IDD, pParent), m_pCurrentFile(NULL)
	, m_itemTreeCurrent(NULL)
{

}

CDlgFileManager::~CDlgFileManager()
{
	TRACE("%s\n", __FUNCTION__);

}

void CDlgFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_FILE_SYSTEM, m_treeFileSystem);
	DDX_Control(pDX, IDC_LIST_DIRECTORY, m_listDirectory);
	DDX_Control(pDX, IDC_COMBO_CURRENT_PATH, m_cbFastRedirect);
	DDX_Control(pDX, IDC_BTN_EXPORT_FILE, m_btnExportFile);
	DDX_Control(pDX, IDC_BTN_IMPORT_FILE, m_btnImportFile);
}

BEGIN_MESSAGE_MAP(CDlgFileManager, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_FILE_SYSTEM, &CDlgFileManager::OnTvnSelchangedTreeFileSystem)
	ON_MESSAGE(UMSG_UPDATE_TREE_LIST, &CDlgFileManager::OnUmsgUpdateList)
	ON_WM_NCDESTROY()
	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &CDlgFileManager::OnHdnItemdblclickListDirectory)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIRECTORY, &CDlgFileManager::OnNMDblclkListDirectory)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_DIRECTORY, &CDlgFileManager::OnLvnColumnclickListDirectory)
	ON_BN_CLICKED(IDC_BTN_MANAGER_BACK, &CDlgFileManager::OnBnClickedBtnManagerBack)
	ON_BN_CLICKED(IDC_BTN_IMPORT_FILE, &CDlgFileManager::OnBnClickedBtnImportFile)
	ON_BN_CLICKED(IDC_BTN_EXPORT_FILE, &CDlgFileManager::OnBnClickedBtnExportFile)
	ON_COMMAND(ID_IMPORTTYPE_FILE, &CDlgFileManager::OnImporttypeFile)
	ON_COMMAND(ID_IMPORTTYPE_DIRECTORY, &CDlgFileManager::OnImporttypeDirectory)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIRECTORY, &CDlgFileManager::OnNMRClickListDirectory)
	ON_COMMAND(ID_DIRREPORTMENU_EXPORT, &CDlgFileManager::OnDirreportmenuExport)
	ON_COMMAND(ID_DIRREPORTMENU_DELETE, &CDlgFileManager::OnDirreportmenuDelete)
	ON_NOTIFY(NM_CLICK, IDC_LIST_DIRECTORY, &CDlgFileManager::OnNMClickListDirectory)
	ON_MESSAGE(UMSG_REFRESH_PARENT, &CDlgFileManager::OnUmsgRefreshList)
	ON_CBN_SELCHANGE(IDC_COMBO_CURRENT_PATH, &CDlgFileManager::OnCbnSelchangeComboCurrentPath)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST_DIRECTORY, &CDlgFileManager::OnLvnGetInfoTipListDirectory)
	ON_NOTIFY(LVN_HOTTRACK, IDC_LIST_DIRECTORY, &CDlgFileManager::OnLvnHotTrackListDirectory)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_FILE_SYSTEM, &CDlgFileManager::OnNMRClickTreeFileSystem)
	ON_NOTIFY(NM_CLICK, IDC_TREE_FILE_SYSTEM, &CDlgFileManager::OnNMClickTreeFileSystem)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_REMOUNT, &CDlgFileManager::OnBnClickedBtnRemount)
	ON_MESSAGE(UMSG_UPDATE_FILE_LIST, &CDlgFileManager::OnUmsgUpdateFileList)
END_MESSAGE_MAP()


const CString CDlgFileManager::REDIR_EXT_SDCARD = TEXT("Ext Sdcard");
const CString CDlgFileManager::REDIR_FILE_SYSTEM = TEXT("File System");
const CString CDlgFileManager::REDIR_SDCARD = TEXT("Sdcard");

BOOL CDlgFileManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	g_dlgCurrent = this;
	m_fileManager.Refresh();
	m_toolTip.EnableToolTips();  
	m_toolTip.Create(this, TTS_ALWAYSTIP);  
	m_toolTip.Activate(TRUE);  
	m_toolTip.SetDelayTime(TTDT_INITIAL, 0);  

	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Name"), m_vecColumns.size(), 80));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Owner"), m_vecColumns.size(), 60));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Mode"), m_vecColumns.size(), 80));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Size"), m_vecColumns.size(), 60));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Date"), m_vecColumns.size(), 100));

	m_mapFastRedirect[REDIR_FILE_SYSTEM] = NULL;
	m_mapFastRedirect[REDIR_EXT_SDCARD] = NULL;
	m_mapFastRedirect[REDIR_SDCARD] = NULL;

	vector<FILE_COLUMN_T>::const_iterator cit = m_vecColumns.begin();
	for (; cit != m_vecColumns.end(); cit++) 
	{
		m_listDirectory.InsertColumn(cit->index, cit->name);
		m_listDirectory.SetColumnWidth(cit->index, cit->width);
	}

	DWORD dwStyle = m_listDirectory.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_listDirectory.SetExtendedStyle(dwStyle); //设置扩展风格

	HICON idir = AfxGetApp()->LoadIconW(IDI_ICON_ANDROID_FOLDER);
	HICON ifile = theApp.LoadIcon(IDI_ICON_ANDROID_FILE);
	m_imgList.Create(15, 15, ILC_COLOR32, 2, 2);
	m_imgList.Add(idir);
	m_imgList.Add(ifile);
	m_treeFileSystem.SetImageList(&m_imgList, 0);
	InitTree();
	return TRUE;
}

CString GetFileSizeInfo(long long size)
{
	double sizeKB = size / 1024.0;
	double sizeMB = sizeKB / 1024.0;
	double sizeGB = sizeMB / 1024.0;

	CString retSize;
	if (sizeGB > 1) {
		retSize.Format(TEXT("%.1fG"), sizeGB);
	} else if (sizeMB > 1) {
		retSize.Format(TEXT("%.1fM"), sizeMB);
	} else if (sizeKB > 1) {
		retSize.Format(TEXT("%.1fK"), sizeKB);
	} else {
		retSize.Format(TEXT("%ldB"), size);
	}
	return retSize;
}

FileIndex* CDlgFileManager::GetIndexes(HTREEITEM item)
{
	map<HTREEITEM, FileIndex>::iterator it = m_mapItemIndex.find(item);
	if (it == m_mapItemIndex.end()) {
		return NULL;
	}
	return &it->second;
}

void CDlgFileManager::AddListItem(const CAndroidFile *pFile, HTREEITEM hItem, BOOL bStatistic)
{
	int countList = 0;
	int nRow;
	int list = 0;
	countList = m_listDirectory.GetItemCount();
	nRow = m_listDirectory.InsertItem(countList, pFile->m_strName);
	m_listDirectory.SetItemText(nRow, ++list, pFile->m_strOwner);
	m_listDirectory.SetItemText(nRow, ++list, pFile->m_strMode);
	if (TRUE/*!pFile->IsDirectory() || bStatistic*/) {
		CString size;
		if (bStatistic) {
			size.Format(TEXT("[%s]"), GetFileSizeInfo(pFile->m_llSize));
		} else {
			size = GetFileSizeInfo(pFile->m_llSize);
		}
		m_listDirectory.SetItemText(nRow, ++list, size);
	} else {
		/* 目录不显示大小 */
		m_listDirectory.SetItemText(nRow, ++list, TEXT(""));
	}
	m_listDirectory.SetItemText(nRow, ++list, pFile->m_strTime);
	m_listDirectory.SetItemData(nRow, (DWORD_PTR)hItem);
	
}
void CDlgFileManager::OrderFiles()
{
	multimap<CString, LIST_FILE_ITEM_T> mappedFiles;
	multimap<long long, LIST_FILE_ITEM_T> intFiles;
	BOOL bIntOrder = FALSE;
	vector<LIST_FILE_ITEM_T>::const_iterator cit = m_vecListFiles.begin();
	switch (m_nFileOrder & (~ASCEND_SUFFIX))
	{
	case FORDER_MODE:
		for (; cit != m_vecListFiles.end(); cit++) {
			mappedFiles.insert(std::make_pair(cit->file.m_strMode, *cit));
		}
		break;
	case FORDER_NAME:
		for (; cit != m_vecListFiles.end(); cit++) {
			mappedFiles.insert(std::make_pair((cit->file.m_strName), *cit));
		}
		break;
	case FORDER_SIZE:
		bIntOrder = TRUE;
		for (; cit != m_vecListFiles.end(); cit++) {
			intFiles.insert(std::make_pair((cit->file.m_llSize), *cit));
		}
		break;
	case FORDER_TIME:
		for (; cit != m_vecListFiles.end(); cit++) {
			mappedFiles.insert(std::make_pair((cit->file.m_strTime), *cit));
		}
		break;
	case FORDER_OWNER:
		for (; cit != m_vecListFiles.end(); cit++) {
			mappedFiles.insert(std::make_pair(cit->file.m_strOwner, *cit));
		}
		break;
	default:
		return;
	}
	m_vecListFiles.clear();

	if (bIntOrder) {
		if (ORDER_IS_ASCEND(m_nFileOrder)) {
			multimap<long long, LIST_FILE_ITEM_T>::const_iterator npuCit = intFiles.begin();
			for (; npuCit != intFiles.end(); npuCit++) {
				m_vecListFiles.push_back(npuCit->second);
			}
		} else {
			multimap<long long, LIST_FILE_ITEM_T>::const_reverse_iterator npuCit = intFiles.rbegin();
			for (; npuCit != intFiles.rend(); npuCit++) {
				m_vecListFiles.push_back(npuCit->second);
			}
		}
		return;
	}


	if (ORDER_IS_ASCEND(m_nFileOrder)) {
		multimap<CString, LIST_FILE_ITEM_T>::const_iterator npuCit = mappedFiles.begin();
		for (; npuCit != mappedFiles.end(); npuCit++) {
			m_vecListFiles.push_back(npuCit->second);
		}
	} else {
		multimap<CString, LIST_FILE_ITEM_T>::const_reverse_iterator npuCit = mappedFiles.rbegin();
		for (; npuCit != mappedFiles.rend(); npuCit++) {
			m_vecListFiles.push_back(npuCit->second);
		}
	}
}
void CDlgFileManager::UpdateList()
{
	m_listDirectory.DeleteAllItems();
	OrderFiles();
	vector<LIST_FILE_ITEM_T>::const_iterator cit = m_vecListStaticsItems.begin();
	for (; cit != m_vecListStaticsItems.end(); cit++)
	{
		AddListItem(&(cit->file), cit->titem, TRUE);
	}

	cit = m_vecListFiles.begin();
	for (; cit != m_vecListFiles.end(); cit++)
	{
		AddListItem(&(cit->file), cit->titem);
	}
}

void CDlgFileManager::UpdateStatistic()
{
	m_vecListStaticsItems.clear();
	LIST_FILE_ITEM_T lfit;
	lfit.titem = NULL;
	
	int nFile = 0, nDir = 0, nOthers = 0;
	long long nSizeByte = 0;
	vector<LIST_FILE_ITEM_T>::const_iterator cit = m_vecListFiles.begin();
	for (; cit != m_vecListFiles.end(); cit++)
	{
		switch (cit->file.m_nType) {
		case FTYPE_FILE:
			nFile++;
			break;
		case FTYPE_DIR:
			nDir++;
			break;
		case FTYPE_BLOCK:
		case FTYPE_LINK:
		case FTYPE_CHAR:
		default:
			nOthers++;
			break;
		}
		nSizeByte += cit->file.m_llSize;
	}
	lfit.file.m_strName.Format(TEXT("[Total:%d]"), nDir+nFile+nOthers);
	lfit.file.m_strOwner.Format(TEXT("[Dir:%d]"), nDir);
	lfit.file.m_strMode.Format(TEXT("[File:%d]"), nFile+nOthers);
	lfit.file.m_llSize = nSizeByte;
	m_vecListStaticsItems.push_back(lfit);

}

void CDlgFileManager::EnableCtrls(BOOL bEnable)
{
	const UINT CTRLS[] = {
		IDC_BTN_MANAGER_BACK,
		IDC_TREE_FILE_SYSTEM, IDC_LIST_DIRECTORY,
		IDC_COMBO_CURRENT_PATH, IDC_BTN_REMOUNT,
	};
	int size = ARRAYSIZE(CTRLS);
	
	for (int i = 0; i < size; i++) {
		CWnd *pWnd = GetDlgItem(CTRLS[i]);
		if (pWnd) {
			pWnd->EnableWindow(bEnable);
		}
	}
}

void CDlgFileManager::UpdateCtrls(HTREEITEM hItem, BOOL bUpdateList, BOOL bForceUpdate)
{
	if (hItem == NULL) { 
		return;
	}
	FileIndex *pIndex = GetIndexes(hItem);
	if (!pIndex) {
		return;
	}
	CAndroidFile *pFile = GetFile(hItem);
	if (bUpdateList) {
		m_vecListFiles.clear();
	}
	if (!pFile->IsDirectory()) {
		if (bUpdateList) {
			m_vecListFiles.push_back(LIST_FILE_ITEM_T(pFile, hItem));
		}
		return;
	}

	if (m_treeFileSystem.GetChildItem(hItem) != NULL)
	{
		if (bForceUpdate) {
			HTREEITEM subitem = m_treeFileSystem.GetChildItem(hItem);
			while (subitem) {
				m_treeFileSystem.DeleteItem(subitem);
				subitem = m_treeFileSystem.GetChildItem(hItem);
			}
		} else {
			return;
		}
	}

	FileIndex baseIndexes(*pIndex);
	int count = pFile->m_vecSubfiles.size();
	for (int i = 0; i < count; i++) {
		FileIndex subIndex(baseIndexes);
		subIndex.push_back(i);
		CAndroidFile *pSub = pFile->m_vecSubfiles.at(i);
		HTREEITEM subItem = m_treeFileSystem.InsertItem(pSub->m_strName, ToImage(pSub), ToImage(pSub), hItem);
		m_mapItemIndex[subItem] = subIndex;

		AddFastRedirect(pSub, subItem);
		if (pSub->m_bExpanded) {
			UpdateCtrls(subItem, FALSE);
			if (pSub->IsDirectory()) {
				m_treeFileSystem.Expand(subItem,TVE_EXPAND); 
			}
		}
		if (bUpdateList) {
			m_vecListFiles.push_back(LIST_FILE_ITEM_T(pSub, subItem));
		}
	}
}

CAndroidFile* CDlgFileManager::GetFile(HTREEITEM item)
{
	map<HTREEITEM, FileIndex>::const_iterator cit = m_mapItemIndex.find(item);
	if (cit == m_mapItemIndex.end()) {
		return NULL;
	}
	CAndroidFile *file = m_fileManager.GetFile(cit->second);

	return file;
}

void CDlgFileManager::Update(HTREEITEM hitem)
{
	if (hitem == NULL) {
		hitem = m_treeFileSystem.GetFirstVisibleItem();
	}
	UpdateCtrls(hitem);
	UpdateStatistic();
	m_nFileOrder = FORDER_DEFAULT;
	SetCurrentItem(hitem);
	UpdateList();
}

int CDlgFileManager::ToImage(CAndroidFile *pFile) const
{
	if (pFile->IsDirectory()) {
		return 0;
	} else {
		return 1;
	}
}
static BOOL g_bExpandThreadRunning = FALSE;
unsigned WINAPI ThreadExpandTreeItem(LPVOID lP) 
{
	CAndroidFile::P_SUBFILE_PARA_T spt = (CAndroidFile::P_SUBFILE_PARA_T)(lP);

	TRACE("start ThreadExpandTreeItem %d\n", GetCurrentThreadId());

	CAdbInterface::AddSubThread(GetCurrentThread());
	if (g_bExpandThreadRunning) {
		delete spt;
		CAdbInterface::RemoveSubThread(GetCurrentThread());
		return -1;
	}
	g_bExpandThreadRunning = TRUE;

	DWORD begin = ::GetCurrentTime();
	g_dlgCurrent->EnableButtons(FALSE);
	g_dlgCurrent->EnableCtrls(FALSE);
	spt->file->Expand(spt->depth);
	g_dlgCurrent->EnableCtrls(TRUE);
	DWORD end = ::GetCurrentTime();
	g_dlgCurrent->PostMessageW(UMSG_UPDATE_TREE_LIST);
	TRACE("expand elaps: %d ms\n", end-begin);

	delete spt;
	g_bExpandThreadRunning = FALSE;
	CAdbInterface::RemoveSubThread(GetCurrentThread());
	return 0;
}


void CDlgFileManager::ExpandFile(CAndroidFile *file, int depth)
{
	CAndroidFile::P_SUBFILE_PARA_T spt = new CAndroidFile::SUBFILE_PARA_T;
	spt->file = file;
	spt->depth = depth;

	_beginthreadex(NULL, 0, ThreadExpandTreeItem, (void*)spt, 0, NULL);

}

DWORD_PTR CDlgFileManager::ToItemData(HTREEITEM item) const
{
	return m_treeFileSystem.GetItemData(item);
}

void CDlgFileManager::EnableButtons(BOOL bEnable)
{
	m_btnExportFile.EnableWindow(bEnable);
	m_btnImportFile.EnableWindow(bEnable);
}

void CDlgFileManager::InitTree()
{
	m_treeFileSystem.DeleteAllItems();
	m_mapItemIndex.clear();
	FileIndex rootIndex;
	CAndroidFile *pRoot = m_fileManager.GetRoot();
	HTREEITEM hRootItem = m_treeFileSystem.InsertItem(pRoot->m_strName, ToImage(pRoot), ToImage(pRoot));
	rootIndex.push_back(0);
	m_mapItemIndex[hRootItem] = rootIndex;
	AddFastRedirect(pRoot, hRootItem);
	ExpandFile(pRoot, ROOT_INIT_DEPTH);
}
void CDlgFileManager::SetCurrentItem(HTREEITEM item)
{
	m_pCurrentFile = GetFile(item);
	m_itemTreeCurrent = item;
	EnableButtons(m_pCurrentFile != NULL);
	if (item)
		m_treeFileSystem.Expand(item,TVE_EXPAND); 
}
void CDlgFileManager::ExpandTreeItem(HTREEITEM item)
{
	CAndroidFile *pFile = GetFile(item);
	SetCurrentItem(item);
	if (pFile) {
		ExpandFile(pFile, m_fileManager.IsRoot(pFile) ? ROOT_INIT_DEPTH : NORMAL_DEPTH);
	}
}

void CDlgFileManager::OnTvnSelchangedTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM selectItem = pNMTreeView->itemNew.hItem;
	ExpandTreeItem(selectItem);

	*pResult = 0;
}

unsigned WINAPI ThreadUpdateList(LPVOID lP) 
{
	HTREEITEM item = (HTREEITEM)(lP);
	g_dlgCurrent->EnableButtons(FALSE);
	g_dlgCurrent->EnableCtrls(FALSE);
	g_dlgCurrent->Update(item);
	g_dlgCurrent->EnableCtrls(TRUE);
	g_dlgCurrent->SetCurrentItem(item);

	return 0;
}

afx_msg LRESULT CDlgFileManager::OnUmsgUpdateList(WPARAM wParam, LPARAM lParam)
{
	/*beginthreadex(0, NULL, ThreadUpdateList, (void*)m_itemTreeCurrent, 0, NULL);*/
	Update(m_itemTreeCurrent);
	return 0;
}


void CDlgFileManager::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	// TODO: 在此处添加消息处理程序代码
	delete this;
	g_bExpandThreadRunning = FALSE;
}


void CDlgFileManager::OnHdnItemdblclickListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if (IsItemInvalid(phdr->iItem)) {
		*pResult = 0;
		return;
	}
	*pResult = 0;
}


void CDlgFileManager::OnNMDblclkListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (IsItemInvalid(pNMItemActivate->iItem)) {
		*pResult = 0;
		return;
	}
	HTREEITEM item = (HTREEITEM)m_listDirectory.GetItemData(pNMItemActivate->iItem);
	ExpandTreeItem(item);

	*pResult = 0;
}


void CDlgFileManager::OnLvnColumnclickListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);


	if (pNMLV->iSubItem == -1) {
		return;
	}

	int orderHigh = (0x10 << pNMLV->iSubItem);
	if (m_nFileOrder & orderHigh) {
		m_nFileOrder = ORDER_IS_ASCEND(m_nFileOrder) ? 
orderHigh : (orderHigh | ASCEND_SUFFIX);
	} else {
		m_nFileOrder = orderHigh;
	}
	TRACE("File Order: %02X\n", m_nFileOrder);

	PostMessageW(UMSG_UPDATE_FILE_LIST);

	*pResult = 0;
}


void CDlgFileManager::OnBnClickedBtnManagerBack()
{
	PostMessage(UMSG_REFRESH_PARENT);
}


void CDlgFileManager::OnBnClickedBtnImportFile()
{
	CMenu menu; //定义下面要用到的cmenu对象
	menu.LoadMenu(IDR_MENU_IMPORT_TYPE); //装载自定义的右键菜单 
	CMenu *pPopup = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单

	CPoint point1;//定义一个用于确定光标位置的位置 
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标 


	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point1.x,point1.y, this);//在指定位置显示弹出菜单
}


void CDlgFileManager::OnBnClickedBtnExportFile()
{
	CString strSavePath;
	if (!SelectDir(GetSafeHwnd(), strSavePath)) {
		return;
	}
	TRACE(TEXT("save to: %s\n"), strSavePath);

	PARAM_T para;
	para.strCmd.Format(TEXT("adb pull %s %s"), m_pCurrentFile->m_strPath, strSavePath);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	CString msg;
	if (para.strReturn.Find(TEXT("error:")) != -1)
	{
		msg.Format(TEXT("导出%s失败(%s)"), m_pCurrentFile->m_strPath, para.strReturn);
		MessageBox(msg, TEXT("Error"), MB_ICONERROR);
	} else {
		CString error;
		msg.Format(TEXT("导出%s成功!"), m_pCurrentFile->m_strPath);
		MessageBox(msg, TEXT("Info"), MB_ICONINFORMATION);
	}
}

BOOL SelectDir(HWND hWnd, CString &strPath, BOOL bCreateNew)  
{  
	BROWSEINFO bifolder;  
	wchar_t FileName[MAX_PATH] = {0};  
	ZeroMemory(&bifolder, sizeof(BROWSEINFO));   
	bifolder.hwndOwner = hWnd;              // 拥有者句柄  
	bifolder.pszDisplayName = FileName;     // 存放目录路径缓冲区  
	bifolder.lpszTitle = TEXT("请选择文件夹");    // 标题  

	bifolder.ulFlags = BIF_EDITBOX; // 新的样式,带编辑框  
	if (bCreateNew) {
		bifolder.ulFlags |= BIF_NEWDIALOGSTYLE;
	}

	LPITEMIDLIST idl = SHBrowseForFolder(&bifolder);  
	if (idl == NULL)  
	{  
		return FALSE;  
	}  

	SHGetPathFromIDList(idl,FileName);      
	strPath = FileName;  
	return TRUE;  
}  
BOOL OpenFile(HWND hWnd, CString &strPath)  
{   
	wchar_t szPathName[MAX_PATH] = {0};  
	OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };//or  {sizeof (OPENFILENAME)}    
	ofn.hwndOwner = hWnd;          // 拥有者句柄      
	// 过滤器,以\0相隔: 显示名称\0过滤器\0显示名称\0过滤器\0\0  
	ofn.lpstrFilter = TEXT("所有文件(*.*)\0*.*\0\0");  
	ofn.lpstrFile = szPathName;             // 存放用户选择文件的 路径及文件名 缓冲区   
	ofn.nMaxFile = sizeof(szPathName);      // 缓冲区大小,单位为字节，至少256  
	ofn.lpstrTitle = TEXT("选择文件");      // 选择文件对话框标题    
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST /*| OFN_ALLOWMULTISELECT 允许选择多个文件*/;  

	BOOL bOk = GetOpenFileName(&ofn);  
	if (!bOk)    
	{    
		return FALSE;  
	}   
	strPath = szPathName;  

	return TRUE;  
}  

BOOL CDlgFileManager::DeleteAndroidFile(CAndroidFile *pFile)
{
	CString strSrcPath = pFile->m_strPath;
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell rm -rf %s"), strSrcPath);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	CString msg;
	if (para.strReturn.Find(TEXT("error:")) != -1)
	{
		msg.Format(TEXT("删除%s失败(%s)"), strSrcPath, para.strReturn);
		MessageBox(msg, TEXT("Error"), MB_ICONERROR);
		return FALSE;
	} else {
		CString error;
		msg.Format(TEXT("删除%s成功!"), strSrcPath);
		MessageBox(msg, TEXT("Info"), MB_ICONINFORMATION);
		return TRUE;
	}
}

BOOL CDlgFileManager::ImportFile(CString strSrcPath)
{
	CString strDestPath;
	if (m_pCurrentFile->IsDirectory())
	{
		strDestPath = m_pCurrentFile->m_strPath;
	} else {
		strDestPath = m_pCurrentFile->m_strParent;
	}

	PARAM_T para;
	para.strCmd.Format(TEXT("adb push %s %s"), strSrcPath, strDestPath);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	CString msg;
	if (para.strReturn.Find(TEXT("error:")) != -1)
	{
		msg.Format(TEXT("导入%s失败(%s)"), strDestPath, para.strReturn);
		MessageBox(msg, TEXT("Error"), MB_ICONERROR);
		return FALSE;
	} else {
		CString error;
		msg.Format(TEXT("导入%s成功!"), strSrcPath);
		MessageBox(msg, TEXT("Info"), MB_ICONINFORMATION);
		return TRUE;
	}
}

void CDlgFileManager::OnImporttypeFile()
{
	CString strSrcFile;
	if (!OpenFile(GetSafeHwnd(),strSrcFile)) {
		return;
	}
	TRACE(TEXT("send file: %s\n"), strSrcFile);
	ImportFile(strSrcFile);
	PostMessage(UMSG_REFRESH_PARENT);
}


void CDlgFileManager::OnImporttypeDirectory()
{
	CString strSrcDir;
	if (!SelectDir(GetSafeHwnd(), strSrcDir, FALSE)) {
		return;
	}
	TRACE(TEXT("send dir: %s\n"), strSrcDir);
	ImportFile(strSrcDir);
	PostMessage(UMSG_REFRESH_PARENT);
}


void CDlgFileManager::OnNMRClickListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (IsItemInvalid(pNMItemActivate->iItem)) {
		*pResult = 0;
		return;
	}
	
	HTREEITEM hCurrentItem = (HTREEITEM)m_listDirectory.GetItemData(pNMItemActivate->iItem);
	SetCurrentItem(hCurrentItem);

	CMenu menu; //定义下面要用到的cmenu对象
	menu.LoadMenu(IDR_MENU_DIR_LIST_MENU); //装载自定义的右键菜单 
	CMenu *pPopup = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单

	CPoint point1;//定义一个用于确定光标位置的位置 
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标 


	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point1.x,point1.y, this);//在指定位置显示弹出菜单

	*pResult = 0;
}


void CDlgFileManager::OnDirreportmenuExport()
{
	m_btnExportFile.SendMessage(BM_CLICK,0,0);
}


void CDlgFileManager::OnDirreportmenuDelete()
{
	CString strInfo;
	strInfo.Format(TEXT("确定删除%s?"), m_pCurrentFile->m_strPath);
	int nRet = MessageBox(strInfo, TEXT("Warning"), MB_ICONQUESTION | MB_YESNOCANCEL);
	if (nRet != IDYES) {
		return;
	}
	DeleteAndroidFile(m_pCurrentFile);
	PostMessage(UMSG_REFRESH_PARENT);
}


void CDlgFileManager::OnNMClickListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (IsItemInvalid(pNMItemActivate->iItem)) {
		*pResult = 0;
		return;
	}
	HTREEITEM hCurrentItem = (HTREEITEM)m_listDirectory.GetItemData(pNMItemActivate->iItem);
	SetCurrentItem(hCurrentItem);
	*pResult = 0;
}


afx_msg LRESULT CDlgFileManager::OnUmsgRefreshList(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM itemParent = m_treeFileSystem.GetParentItem(m_itemTreeCurrent);
	ExpandTreeItem(itemParent);

	return 0;
}
void CDlgFileManager::AddComboFastRedir(CString strItem)
{
	int size = m_cbFastRedirect.GetCount();
	CString strText;
	for (int i = 0; i < size; i++) {
		m_cbFastRedirect.GetLBText(i, strText);
		if (!strText.CompareNoCase(strItem)) {
			return;
		}
	}
	m_cbFastRedirect.AddString(strItem);
}
void CDlgFileManager::AddFastRedirect(CAndroidFile *pFile, HTREEITEM item)
{
	if (!pFile->IsDirectory()) {
		return;
	}
	if (pFile->m_strPath == CAndroidFile::EXT_SDCARD_DIR) 
	{
		m_mapFastRedirect[REDIR_EXT_SDCARD] = item;
		AddComboFastRedir(REDIR_EXT_SDCARD);
	} 
	else if (pFile->m_strPath == CAndroidFile::SDCARD_DIR) 
	{
		m_mapFastRedirect[REDIR_SDCARD] = item;
		AddComboFastRedir(REDIR_SDCARD);
	} 
	else if (pFile->m_strPath == CAndroidFile::ROOT_DIR) 
	{
		m_mapFastRedirect[REDIR_FILE_SYSTEM] = item;
		AddComboFastRedir(REDIR_FILE_SYSTEM);
		m_cbFastRedirect.SetCurSel(0);
	}
}

void CDlgFileManager::OnCbnSelchangeComboCurrentPath()
{
	CString strText;
	m_cbFastRedirect.GetWindowTextW(strText);
	map<CString, HTREEITEM>::const_iterator cit = m_mapFastRedirect.find(strText);
	if (cit != m_mapFastRedirect.end()) {
		if (cit->second) {
			ExpandTreeItem(cit->second);
		}
	}
}


void CDlgFileManager::OnLvnGetInfoTipListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	*pResult = 0;
}



void CDlgFileManager::OnLvnHotTrackListDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nItem = pNMLV->iItem;
	
	if (IsItemInvalid(nItem) || (nItem >= m_listDirectory.GetItemCount())) {
		m_nLastItem = -1;
		*pResult = 0;
		return;
	}

	if (m_nLastItem != nItem) {
		m_nLastItem = nItem;
		HTREEITEM htItem = (HTREEITEM)m_listDirectory.GetItemData(nItem);
		CAndroidFile *pFile = GetFile(htItem);
		CString strInfo;
		strInfo.Format(TEXT("[%s:%s]%s(%s)"), pFile->m_strOwner, CAndroidFile::TypeToString(pFile->m_nType), pFile->m_strName, pFile->m_strPath);
		m_toolTip.AddTool(GetDlgItem(IDC_LIST_DIRECTORY), strInfo.GetBuffer());
		m_toolTip.Pop();
	}
		
	

	*pResult = 0;
}


BOOL CDlgFileManager::PreTranslateMessage(MSG* pMsg)
{
	UINT ctrlId = GetWindowLong(pMsg->hwnd,GWL_ID);
	if (pMsg->message == WM_MOUSEMOVE) {
		if (ctrlId == IDC_LIST_DIRECTORY) {
			if (!IsItemInvalid(m_nLastItem)) {
				m_toolTip.RelayEvent(pMsg);
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDlgFileManager::OnNMRClickTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetCurrentItem(m_treeFileSystem.GetDropHilightItem());

	CMenu menu; //定义下面要用到的cmenu对象
	menu.LoadMenu(IDR_MENU_DIR_LIST_MENU); //装载自定义的右键菜单 
	CMenu *pPopup = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单

	CPoint point1;//定义一个用于确定光标位置的位置 
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标 


	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point1.x,point1.y, this);//在指定位置显示弹出菜单


	*pResult = 0;
}


void CDlgFileManager::OnNMClickTreeFileSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CDlgFileManager::OnClose()
{
	//::PostMessage(g_hMainWnd, UMSG_FILE_MANAGER_CLOSE, 0, 0);
	if (g_bExpandThreadRunning) {
		return;
	}
	CAndroidFile::ReleaseAllFile();
	CDialogEx::OnClose();
}

unsigned WINAPI ThreadRemount(LPVOID lP)
{
	CDlgFileManager *pDlg = (CDlgFileManager *)lP;
	pDlg->EnableCtrls(FALSE);
	PARAM_T para;
	para.strCmd.Format(TEXT("adb root"));
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	para.strCmd.Format(TEXT("adb remount"));
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	pDlg->EnableCtrls(TRUE);
	pDlg->PostMessageW(UMSG_UPDATE_TREE_LIST);
	return 0;
}

void CDlgFileManager::OnBnClickedBtnRemount()
{
	_beginthreadex(NULL, 0, ThreadRemount, (void*)this, 0, NULL);
}


afx_msg LRESULT CDlgFileManager::OnUmsgUpdateFileList(WPARAM wParam, LPARAM lParam)
{
	UpdateList();
	return 0;
}
