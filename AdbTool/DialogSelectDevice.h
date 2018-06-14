#pragma once
#include "afxwin.h"
#include "AdbInterface.h"
#include <map>

using std::map;
// CDialogSelectDevice �Ի���

class CDialogSelectDevice : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogSelectDevice)

public:
	CDialogSelectDevice(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogSelectDevice();

// �Ի�������
	enum { IDD = IDD_DIALOG_CHOOSE_DEV };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listAdbDevices;
	afx_msg void OnLbnDblclkListAdbDevices();
	afx_msg void OnClose();

private:
	map<int, ADB_DEVICE*> m_mapDevices;
};
