// DialogProcessManager.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogProcessManager.h"
#include "afxdialogex.h"
#include <map>
#include "StartActivityDlg.h"
using std::multimap;


// CDialogProcessManager 对话框

IMPLEMENT_DYNAMIC(CDialogProcessManager, CDialogEx)

CDialogProcessManager::CDialogProcessManager(CWnd* pParent /*=NULL*/)
	: m_itemtreeAllProcess(NULL)
	, m_itemtreeTreeProcess(NULL)
	, m_nProcessOrder(PROCORDER_DEFAULT)
	, m_itemtreePackgeProcess(NULL)
	, CDialogEx(CDialogProcessManager::IDD, pParent)
{

}

CDialogProcessManager::~CDialogProcessManager()
{
}

void CDialogProcessManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_PROCESSES, m_treeProcesses);
	DDX_Control(pDX, IDC_LIST_PROCESSES, m_listProcesses);
}


BEGIN_MESSAGE_MAP(CDialogProcessManager, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_NCDESTROY()
	ON_WM_PAINT()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PROCESSES, &CDialogProcessManager::OnTvnSelchangedTreeProcesses)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREE_PROCESSES, &CDialogProcessManager::OnTvnItemexpandedTreeProcesses)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PROCESSES, &CDialogProcessManager::OnLvnColumnclickListProcesses)
	ON_COMMAND(ID_PROCESSSUBMENU_KILL, &CDialogProcessManager::OnProcesssubmenuKill)
	ON_MESSAGE(UMSG_REFRESH_ANDROID_PROCESS, &CDialogProcessManager::OnUmsgRefreshAndroidProcess)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESSES, &CDialogProcessManager::OnNMRClickListProcesses)
	ON_NOTIFY(LVN_HOTTRACK, IDC_LIST_PROCESSES, &CDialogProcessManager::OnLvnHotTrackListProcesses)
	ON_COMMAND(ID_PROCESSSUBMENU_REFRESH, &CDialogProcessManager::OnProcesssubmenuRefresh)
	ON_COMMAND(ID_PACKAGEOPERATIONS_FORCESTOP, &CDialogProcessManager::OnPackageoperationsForcestop)
	ON_COMMAND(ID_PACKAGEOPERATIONS_STARTACTIVITY, &CDialogProcessManager::OnPackageoperationsStartactivity)
	ON_COMMAND(ID_PACKAGEOPERATIONS_UNINSTALL, &CDialogProcessManager::OnPackageoperationsUninstall)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCESSES, &CDialogProcessManager::OnNMDblclkListProcesses)
END_MESSAGE_MAP()


// CDialogProcessManager 消息处理程序

BOOL CDialogProcessManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_toolTip.EnableToolTips();  
	m_toolTip.Create(this, TTS_ALWAYSTIP);  
	m_toolTip.Activate(TRUE);  
	m_toolTip.SetDelayTime(TTDT_INITIAL, 0);  

	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("PID"), m_vecColumns.size(), 50));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("User"), m_vecColumns.size(), 50));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Name"), m_vecColumns.size(), 120));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("S"), m_vecColumns.size(), 20));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("PPID"), m_vecColumns.size(), 35));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Mem"), m_vecColumns.size(), 50));
	m_vecColumns.push_back(FILE_COLUMN_T(TEXT("Virt"), m_vecColumns.size(), 45));


	DWORD dwStyle = m_listProcesses.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_listProcesses.SetExtendedStyle(dwStyle); //设置扩展风格


	vector<FILE_COLUMN_T>::const_iterator cit = m_vecColumns.begin();
	for (; cit != m_vecColumns.end(); cit++) 
	{
		m_listProcesses.InsertColumn(cit->index, cit->name);
		m_listProcesses.SetColumnWidth(cit->index, cit->width);
	}


	m_itemtreeAllProcess = m_treeProcesses.InsertItem(TEXT("All"));
	m_itemtreeTreeProcess = m_treeProcesses.InsertItem(TEXT("Tree"));
	m_itemtreePackgeProcess = m_treeProcesses.InsertItem(TEXT("Package"));

	Refresh();
	return TRUE;
}


void CDialogProcessManager::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

}

void CDialogProcessManager::EnableCtrls(BOOL bEnable)
{
	const UINT CTRLS[] = {
		IDC_TREE_PROCESSES,IDC_LIST_PROCESSES
	};
	int size = ARRAYSIZE(CTRLS);

	for (int i = 0; i < size; i++) {
		GetDlgItem(CTRLS[i])->EnableWindow(bEnable);
	}
}

void CDialogProcessManager::LoadProcess()
{
	m_managerProcess.InitProcesses();
}

BOOL g_bLoadFinished = FALSE;
unsigned WINAPI ThreadRefreshProcess(LPVOID lP)
{
	CDialogProcessManager *pDlg = (CDialogProcessManager *)lP;

	g_bLoadFinished = FALSE;
	pDlg->EnableCtrls(FALSE);
	pDlg->LoadProcess();
	g_bLoadFinished = TRUE;
	pDlg->PostRefreshMsg();
	pDlg->EnableCtrls(TRUE);
	return 0;
}

void CDialogProcessManager::Refresh()
{
	_beginthreadex(NULL, 0, ThreadRefreshProcess, (void*)this, 0, NULL);
}

void CDialogProcessManager::RemoveSubitems(HTREEITEM pitem)
{
	HTREEITEM subitem = m_treeProcesses.GetChildItem(pitem);
	while (subitem) {
		m_treeProcesses.DeleteItem(subitem);
		subitem = m_treeProcesses.GetChildItem(pitem);
	}
}
void CDialogProcessManager::PostRefreshMsg()
{
	Invalidate();
	//PostMessage(UMSG_REFRESH_ANDROID_PROCESS);
}
void CDialogProcessManager::UpdateCtrls()
{
	if (!m_itemtreeTreeProcess || !m_itemtreeAllProcess) {
		return;
	}

	RemoveSubitems(m_itemtreeTreeProcess);
	RemoveSubitems(m_itemtreeAllProcess);
	RemoveSubitems(m_itemtreePackgeProcess);

	vector<CAndroidProcess> processes = m_managerProcess.GetAllProcess();
	vector<CAndroidProcess>::const_iterator cit = processes.begin();
	vector<CAndroidProcess>::const_iterator tit;
	for (; cit != processes.end(); cit++)
	{
		CString title;
		title.Format(TEXT("[%s]%s"), cit->PID, cit->NAME);
		HTREEITEM subitem = m_treeProcesses.InsertItem(title,m_itemtreeAllProcess);
	}

	processes = m_managerProcess.GetTreeProcess();
	cit = processes.begin();
	for (; cit != processes.end(); cit++)
	{
		CString title;
		title.Format(TEXT("[%s]%s"), cit->PID, cit->NAME);
		HTREEITEM subitem = m_treeProcesses.InsertItem(title, m_itemtreeTreeProcess);
		tit = cit->m_vecSubThreads.begin();
		for (; tit != cit->m_vecSubThreads.end(); tit++)
		{
			title.Format(TEXT("[%s]%s"), tit->PID, tit->NAME);
			m_treeProcesses.InsertItem(title,subitem);
		}
	}
	processes = m_managerProcess.GetPackageProcess();
	cit = processes.begin();
	for (; cit != processes.end(); cit++)
	{
		CString title;
		title.Format(TEXT("[%s]%s"), cit->PID, cit->NAME);
		HTREEITEM subitem = m_treeProcesses.InsertItem(title,m_itemtreePackgeProcess);
	}

	m_nProcessOrder = PROCORDER_DEFAULT;
	m_treeProcesses.Select(m_itemtreeTreeProcess, TVGN_CARET);
	m_treeProcesses.Expand(m_itemtreeTreeProcess, TVE_COLLAPSE);
	m_treeProcesses.SetFocus();
	SelectTreeItem(m_itemtreeTreeProcess);
}


void CDialogProcessManager::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	// TODO: 在此处添加消息处理程序代码
	delete this;
}


CString CDialogProcessManager::ItemToPID(HTREEITEM item)
{
	CString title = m_treeProcesses.GetItemText(item);
	int begpos = title.Find(TEXT('['));
	int endpos = title.Find(TEXT(']'));

	if ((begpos == -1) || (endpos == -1)) {
		return TEXT("InvalidPID");
	}
	return title.Mid(begpos+1, endpos-begpos-1);
}

void CDialogProcessManager::AddListItem(const CAndroidProcess &process, BOOL bStatistic)
{
	int countList = 0;
	int nRow;
	int list = 0;
	countList = m_listProcesses.GetItemCount();
	nRow = m_listProcesses.InsertItem(countList, process.PID);
	m_listProcesses.SetItemText(nRow, ++list, process.USER);
	m_listProcesses.SetItemText(nRow, ++list, process.NAME);
	m_listProcesses.SetItemText(nRow, ++list, process.STATUS);
	m_listProcesses.SetItemText(nRow, ++list, process.PPID);
	CString memory;
	if (bStatistic) {
		memory.Format(TEXT("[%s]"), process.ToHReadMemory());
	} else {
		memory = process.ToHReadMemory();
	}
	m_listProcesses.SetItemText(nRow, ++list, memory);
	if (bStatistic) {
		m_listProcesses.SetItemText(nRow, ++list, TEXT("-"));
		m_listProcesses.SetItemState(nRow, LIS_ENABLED, LIS_ENABLED);
	} else {
		m_listProcesses.SetItemText(nRow, ++list, process.ToHReadVirtualMemory());
	}
}

void CDialogProcessManager::OrderProcesses()
{
	multimap<CString, CAndroidProcess> mappedPackges;
	multimap<int, CAndroidProcess> intPackages;
	BOOL bIntOrder = FALSE;
	vector<CAndroidProcess>::const_iterator cit = m_vecListProcesses.begin();
	switch (m_nProcessOrder & (~ASCEND_SUFFIX))
	{
	case PROCORDER_USER:
		for (; cit != m_vecListProcesses.end(); cit++) {
			mappedPackges.insert(std::make_pair(cit->USER, *cit));
		}
		break;
	case PROCORDER_VIRUTAL:
		bIntOrder = TRUE;
		for (; cit != m_vecListProcesses.end(); cit++) {
			intPackages.insert(std::make_pair(_wtoi(cit->VSZ), *cit));
		}
		break;
	case PROCORDER_MEMORY:
		bIntOrder = TRUE;
		for (; cit != m_vecListProcesses.end(); cit++) {
			intPackages.insert(std::make_pair(_wtoi(cit->RSS), *cit));
		}
		break;
	case PROCORDER_PID:
		bIntOrder = TRUE;
		for (; cit != m_vecListProcesses.end(); cit++) {
			intPackages.insert(std::make_pair(_wtoi(cit->PID), *cit));
		}
		break;
	case PROCORDER_PPID:
		bIntOrder = TRUE;
		for (; cit != m_vecListProcesses.end(); cit++) {
			intPackages.insert(std::make_pair(_wtoi(cit->PPID), *cit));
		}
		break;
	case PROCORDER_STATUS:
		for (; cit != m_vecListProcesses.end(); cit++) {
			mappedPackges.insert(std::make_pair(cit->STATUS, *cit));
		}
		break;
	case PROCORDER_NAME:
		for (; cit != m_vecListProcesses.end(); cit++) {
			mappedPackges.insert(std::make_pair(cit->NAME, *cit));
		}
		break;
	default:
		return;
	}
	m_vecListProcesses.clear();

	if (bIntOrder) {
		if (ORDER_IS_ASCEND(m_nProcessOrder)) {
			multimap<int, CAndroidProcess>::const_iterator npuCit = intPackages.begin();
			for (; npuCit != intPackages.end(); npuCit++) {
				m_vecListProcesses.push_back(npuCit->second);
			}
		} else {
			multimap<int, CAndroidProcess>::const_reverse_iterator npuCit = intPackages.rbegin();
			for (; npuCit != intPackages.rend(); npuCit++) {
				m_vecListProcesses.push_back(npuCit->second);
			}
		}
		return;
	}


	if (ORDER_IS_ASCEND(m_nProcessOrder)) {
		multimap<CString, CAndroidProcess>::const_iterator npuCit = mappedPackges.begin();
		for (; npuCit != mappedPackges.end(); npuCit++) {
			m_vecListProcesses.push_back(npuCit->second);
		}
	} else {
		multimap<CString, CAndroidProcess>::const_reverse_iterator npuCit = mappedPackges.rbegin();
		for (; npuCit != mappedPackges.rend(); npuCit++) {
			m_vecListProcesses.push_back(npuCit->second);
		}
	}
}

void CDialogProcessManager::SelectTreeItem(HTREEITEM item)
{
	m_itemtreeSelected = item;
	m_vecListProcesses.clear();
	m_vecListStatistic.clear();
	vector<CAndroidProcess>::const_iterator cit;
	vector<CAndroidProcess> processes;
	if (item == m_itemtreeAllProcess)
	{
		processes = m_managerProcess.GetAllProcess();
	}
	else if (item == m_itemtreeTreeProcess)
	{
		processes = m_managerProcess.GetTreeProcess();
	}  
	else if (item == m_itemtreePackgeProcess)
	{
		processes = m_managerProcess.GetPackageProcess();
	}
	else {
		CString pid = ItemToPID(item);
		m_managerProcess.GetChildren(pid, processes);
		processes.insert(processes.begin(), m_managerProcess.GetDroidProcess(pid));
	}

	CAndroidProcess procTotal;
	long memory = 0;
	long virtmemo = 0;

	cit = processes.begin();
	for (; cit != processes.end(); cit++)
	{
		memory += cit->GetMemory();
		virtmemo += cit->GetVirtMemory();
		m_vecListProcesses.push_back(*cit);
	}
	if (m_vecListProcesses.size() > 0)
	{
		procTotal.NAME = TEXT("[Total]");
		procTotal.PID.Format(TEXT("[%d]"), m_vecListProcesses.size());
		procTotal.RSS.Format(TEXT("%ld"), memory);
		procTotal.VSZ.Format(TEXT("%ld"), virtmemo);
		m_vecListStatistic.push_back(procTotal);
	}
	
	UpdateList();
}

void CDialogProcessManager::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	if (g_bLoadFinished) {
		g_bLoadFinished = FALSE;
		UpdateCtrls();
	}
}


void CDialogProcessManager::OnTvnSelchangedTreeProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	SelectTreeItem(pNMTreeView->itemNew.hItem);


	*pResult = 0;
}


void CDialogProcessManager::OnTvnItemexpandedTreeProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	SelectTreeItem(pNMTreeView->itemNew.hItem);

	*pResult = 0;
}
void CDialogProcessManager::UpdateList()
{
	m_listProcesses.DeleteAllItems();
	OrderProcesses();
	
	vector<CAndroidProcess>::const_iterator cit;
	cit = m_vecListStatistic.begin();
	for (; cit != m_vecListStatistic.end(); cit++)
	{
		AddListItem(*cit, TRUE);
	}

	cit = m_vecListProcesses.begin();
	for (; cit != m_vecListProcesses.end(); cit++)
	{
		AddListItem(*cit);
	}
}

void CDialogProcessManager::OnLvnColumnclickListProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->iSubItem == -1) {
		return;
	}

	int orderHigh = (0x10 << pNMLV->iSubItem);
	if (m_nProcessOrder & orderHigh) {
		m_nProcessOrder = ORDER_IS_ASCEND(m_nProcessOrder) ? 
orderHigh : (orderHigh | ASCEND_SUFFIX);
	} else {
		m_nProcessOrder = orderHigh;
	}
	TRACE("Process Order: %02X\n", m_nProcessOrder);
	UpdateList();

	*pResult = 0;
}


void CDialogProcessManager::OnProcesssubmenuKill()
{
	CAndroidProcess process;
	if (!GetProcessSeleted(process)) {
		return;
	}

	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell kill %s"), process.PID);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (!para.bRet) {
		return;
	}
	CString info;
	if (para.strReturn.IsEmpty()) {
		info.Format(TEXT("进程[%s]%s已被成功杀死!"), process.PID, process.NAME);
		MessageBox(info, TEXT("Info"), MB_ICONINFORMATION);
		Refresh();
	} else {
		info.Format(TEXT("尝试杀死进程[%s]%s失败: %s"), process.PID, process.NAME, para.strReturn);
		MessageBox(info, TEXT("Error"), MB_ICONSTOP);
	}
}


afx_msg LRESULT CDialogProcessManager::OnUmsgRefreshAndroidProcess(WPARAM wParam, LPARAM lParam)
{
	UpdateCtrls();
	return 0;
}

BOOL CDialogProcessManager::GetProcessSeleted(CAndroidProcess &process) const
{
	if ((m_nCurrentSelectItem < 0) ||
		(m_nCurrentSelectItem > (int)(m_vecListProcesses.size()-1)))
	{
		return FALSE;
	}
	process = m_vecListProcesses.at(m_nCurrentSelectItem);
	return TRUE;
}

void CDialogProcessManager::OnNMRClickListProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem <= 0) {
		return;
	}

	m_nCurrentSelectItem = pNMItemActivate->iItem-1;
	CMenu menu; //定义下面要用到的cmenu对象
	if (IsPackageItemSelected()) {
		menu.LoadMenu(IDR_MENU_PACKAGE_OPERATIONS);
	} else {
		menu.LoadMenu(IDR_MENU_PROCESS); //装载自定义的右键菜单 
	}
	CMenu *pPopup = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单

	CPoint point1;//定义一个用于确定光标位置的位置 
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标 

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point1.x,point1.y, this);//在指定位置显示弹出菜单


	*pResult = 0;
}


void CDialogProcessManager::OnLvnHotTrackListProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->iItem <= 0) {
		return;
	}

	int nItem = pNMLV->iItem-1;

	if (m_nCurrentSelectItem != nItem) {
		Sleep(100);
		m_nCurrentSelectItem = nItem;

		CAndroidProcess process;
		if (!GetProcessSeleted(process)) {
			return;
		}

		CString strInfo;
		strInfo.Format(TEXT("[%s:%s]%s"), process.USER, process.PID, process.NAME);
		m_toolTip.AddTool(GetDlgItem(IDC_LIST_PROCESSES), strInfo.GetBuffer());
		m_toolTip.Pop();
	}


	*pResult = 0;
}


BOOL CDialogProcessManager::PreTranslateMessage(MSG* pMsg)
{
	UINT ctrlId = GetWindowLong(pMsg->hwnd,GWL_ID);
	if (pMsg->message == WM_MOUSEMOVE) {
		if (ctrlId == IDC_LIST_PROCESSES) {
			if (!IsItemInvalid(m_nCurrentSelectItem)) {
				m_toolTip.RelayEvent(pMsg);
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDialogProcessManager::SelectThreadItem(CString PID)
{
	HTREEITEM subitem = m_treeProcesses.GetChildItem(m_itemtreeTreeProcess);
	while (subitem) {
		if (PID == ItemToPID(subitem)) {
			m_treeProcesses.Select(subitem, TVGN_CARET);
			m_treeProcesses.Expand(subitem, TVE_COLLAPSE);
			m_treeProcesses.SetFocus();
			SelectTreeItem(subitem);
			return;
		}

		subitem = m_treeProcesses.GetNextSiblingItem(subitem);
	}

	

}
BOOL CDialogProcessManager::IsChildOf(HTREEITEM child, HTREEITEM parent) const
{
	if (!child || !parent) {
		return FALSE;
	}
	if (child == parent) {
		return TRUE;
	}
	HTREEITEM subitem = m_treeProcesses.GetChildItem(parent);
	while (subitem) {
		if (subitem == child) {
			return TRUE;
		}
		subitem = m_treeProcesses.GetNextSiblingItem(subitem);
	}
	return FALSE;
}

BOOL CDialogProcessManager::IsPackageItemSelected() const
{
	return IsChildOf(m_itemtreeSelected, m_itemtreePackgeProcess);
}

BOOL CDialogProcessManager::IsThreadItemSelected() const
{
	return IsChildOf(m_itemtreeSelected, m_itemtreeTreeProcess);
}


void CDialogProcessManager::OnProcesssubmenuRefresh()
{
	Refresh();
}


void CDialogProcessManager::OnPackageoperationsForcestop()
{
	CAndroidProcess process;
	if (!GetProcessSeleted(process)) {
		return;
	}
	CString info;
	info.Format(TEXT("确认停止:%s?"), process.NAME);
	int nRet = MessageBox(info, TEXT("Warning"), MB_ICONQUESTION | MB_YESNOCANCEL);
	if (nRet != IDYES) {
		return;
	}
	BOOL bRet = ForceStopPackage(GetSafeHwnd(), process.NAME);
	if (bRet) {
		Refresh();
	}
}


void CDialogProcessManager::OnPackageoperationsStartactivity()
{
	CAndroidProcess process;
	if (!GetProcessSeleted(process)) {
		return;
	}
	CStartActivityDlg dlg(process.NAME, this);
	dlg.DoModal();
}


void CDialogProcessManager::OnPackageoperationsUninstall()
{
	CAndroidProcess process;
	if (!GetProcessSeleted(process)) {
		return;
	}
	CString info;
	info.Format(TEXT("确认卸载包:%s?"), process.NAME);
	int nRet = MessageBox(info, TEXT("Warning"), MB_ICONQUESTION | MB_YESNOCANCEL);
	if (nRet != IDYES) {
		return;
	}
	BOOL bRet = UninstallPackage(GetSafeHwnd(), process.NAME);
	if (bRet) {
		Refresh();
	}
}

/* 在tree中跳转到子进程　*/
void CDialogProcessManager::OnNMDblclkListProcesses(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if (pNMItemActivate->iItem <= 0) {
		return;
	}
	if (!IsThreadItemSelected() && !IsPackageItemSelected()) {
		return;
	}

	m_nCurrentSelectItem = pNMItemActivate->iItem-1;
	CAndroidProcess process;
	if (!GetProcessSeleted(process)) {
		return;
	}
	if (m_managerProcess.ProcessHasChildren(process.PID)) 
	{
		SelectThreadItem(process.PID);
	}
}
