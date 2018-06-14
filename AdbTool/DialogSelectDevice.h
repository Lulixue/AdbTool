#pragma once
#include "afxwin.h"
#include "AdbInterface.h"
#include <map>

using std::map;
// CDialogSelectDevice 对话框

class CDialogSelectDevice : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogSelectDevice)

public:
	CDialogSelectDevice(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogSelectDevice();

// 对话框数据
	enum { IDD = IDD_DIALOG_CHOOSE_DEV };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listAdbDevices;
	afx_msg void OnLbnDblclkListAdbDevices();
	afx_msg void OnClose();

private:
	map<int, ADB_DEVICE*> m_mapDevices;
};
