// DialogSelectDevice.cpp : 实现文件
//

#include "stdafx.h"
#include "AdbTool.h"
#include "DialogSelectDevice.h"
#include "afxdialogex.h"


// CDialogSelectDevice 对话框

IMPLEMENT_DYNAMIC(CDialogSelectDevice, CDialogEx)

CDialogSelectDevice::CDialogSelectDevice(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogSelectDevice::IDD, pParent)
{

}

CDialogSelectDevice::~CDialogSelectDevice()
{
}

void CDialogSelectDevice::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ADB_DEVICES, m_listAdbDevices);
}


BEGIN_MESSAGE_MAP(CDialogSelectDevice, CDialogEx)
	ON_LBN_DBLCLK(IDC_LIST_ADB_DEVICES, &CDialogSelectDevice::OnLbnDblclkListAdbDevices)
	ON_WM_CLOSE()
END_MESSAGE_MAP()



BOOL CDialogSelectDevice::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	m_mapDevices.clear();
	vector<ADB_DEVICE> &devices = ADB.m_vecAdbDevices;

	int size = devices.size();

	TRACE("Devices:\n");
	for (int i = 0, j = 0; i < size; i++) 
	{
		CString strDevice;
		strDevice.Format(TEXT("[%d] %s"), j+1, devices[i].model);

		if (!devices[i].IsValidDevice()) {
			strDevice.AppendFormat(TEXT(" (state: %s, unavailable!)"), devices[i].state);
			TRACE(TEXT("%s"), strDevice);
			TRACE("\n");
			continue;
		}
		strDevice.AppendFormat(TEXT(" (%s)"), devices[i].serial_no);
		TRACE(TEXT("%s"), strDevice);
		TRACE("\n");
		
		m_listAdbDevices.AddString(strDevice);
		m_mapDevices[j] = &devices[i];
		j++;
	}

	m_listAdbDevices.SetSel(0);
	m_listAdbDevices.SetCurSel(0);

	if (m_listAdbDevices.GetCount() == 1) {
		PostMessage(WM_CLOSE, 0, 0);
	}

	return TRUE;
}

void CDialogSelectDevice::OnLbnDblclkListAdbDevices()
{
	
	PostMessage(WM_CLOSE, 0, 0);
}


void CDialogSelectDevice::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int i = m_listAdbDevices.GetCurSel();
	if (i >= 0) {
		ADB.SelectDevice(*m_mapDevices[i]);
	}
	CDialogEx::OnClose();
}
