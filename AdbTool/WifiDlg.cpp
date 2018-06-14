// DlgWifi.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AdbTool.h"
#include "afxdialogex.h"

#include "winsock.h"

#include "iphlpapi/iphlpapi.h" // GetAdaptersInfo()
#pragma comment(lib,"iphlpapi/iphlpapi.lib")

#include "WifiDlg.h"
#include "AdbInterface.h"

extern HWND g_hMainWnd;
// CWifiDlg �Ի���

IMPLEMENT_DYNAMIC(CWifiDlg, CDialogEx)

CWifiDlg::CWifiDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWifiDlg::IDD, pParent)
{

}

CWifiDlg::~CWifiDlg()
{

}

BOOL CWifiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	if (!RestartUSB()) {
		UINT ret = MessageBox(TEXT("�뽫�ֻ�����USB"), TEXT("Info"), 
				MB_ICONINFORMATION|MB_OKCANCEL);
		if (ret == IDCANCEL) {
			PostMessage(WM_CLOSE);
			return FALSE;
		}
	}

	SetInOperation(TRUE);
	Init();
	m_editTcpPort.SetWindowTextW(TEXT("5555"));

	return TRUE;
}

void CWifiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_NETWORK, m_cbNetworks);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_richeditLog);
	DDX_Control(pDX, IDC_IPADDRESS_WIFI, m_ctrlIp);
	DDX_Control(pDX, IDC_IPADDRESS_PHONE, m_ctrlPhoneIp);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editTcpPort);
	DDX_Control(pDX, IDC_CHECK_ADB_WIRELESS, m_chkNoTcpPort);
}


BEGIN_MESSAGE_MAP(CWifiDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CWifiDlg::OnBnClickedBtnConnect)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_NETWORK, &CWifiDlg::OnSelchangeComboNetwork)
	ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CWifiDlg::OnBnClickedBtnDisconnect)
	ON_BN_CLICKED(IDC_CHECK_ADB_WIRELESS, &CWifiDlg::OnBnClickedCheckAdbWireless)
	ON_WM_CLOSE()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CWifiDlg ��Ϣ�������

void CWifiDlg::Init()
{
	m_vecNetworks.clear();
	// ����ӿ���Ϣ�����20��
	IP_ADAPTER_INFO IOInfo[20];
	// ����ṹָ���ʼ��
	PIP_ADAPTER_INFO pIOInfo=NULL;
	DWORD Result=0;
	// ��ȡ��С
	unsigned long nLen=sizeof(IOInfo);
	// ��ȡ������Ϣ
	Result=GetAdaptersInfo(IOInfo,&nLen);
	CString strLog;
	if(NO_ERROR!=Result)
	{
		m_richeditLog.SetWindowTextW(TEXT("��ȡ������Ϣʧ��!"));
		return;
	}
	else
	{
		pIOInfo=IOInfo; // ������ָ��ָ������ӿ���Ϣ�洢�ṹ
		while(pIOInfo!=NULL)  // ��Ϊ�վ�ѭ����ȡ����
		{
			
			NETWORK_T network;
			network.name = pIOInfo->AdapterName;
			network.desc = pIOInfo->Description;
			network.ip = pIOInfo->IpAddressList.IpAddress.String;
			network.mac.Format(TEXT("%02X:%02X:%02X:%02X:%02X:%02X\n"),pIOInfo->Address[0],pIOInfo->Address[2],pIOInfo->Address[3],pIOInfo->Address[4],pIOInfo->Address[5]);
	
			pIOInfo=pIOInfo->Next;

			m_vecNetworks.push_back(network);
			strLog.AppendFormat(TEXT("--- %d ---\n"), m_vecNetworks.size());
			strLog.AppendFormat(TEXT("Desc: %s\n"), network.desc);
			strLog.AppendFormat(TEXT("IP: %s\n\n"), network.ip);
		}
	}

	int size = m_vecNetworks.size(); 
	int nDef = 0;
	CString strDefIp;
	m_cbNetworks.ResetContent();
	for (int i = 0; i < size; i++) 
	{
		CString strItem;
		strItem = m_vecNetworks.at(i).desc;
		if ( (strItem.Find(TEXT("Wireless")) != -1) ||
			 (strItem.Find(TEXT("wireless")) != -1) )
		{
			nDef = i;
			strDefIp = m_vecNetworks.at(i).ip;
		}

		m_cbNetworks.AddString(strItem);
	}
	m_cbNetworks.SetCurSel(nDef);
	m_ctrlIp.SetWindowTextW(strDefIp);
	
	DWORD addr;
	m_ctrlIp.GetAddress(addr);
	addr++;
	m_ctrlPhoneIp.SetAddress(addr);

	m_richeditLog.SetWindowTextW(strLog);
	m_richeditLog.PostMessage(WM_VSCROLL, SB_BOTTOM,0);

}

void CWifiDlg::AppendText(CString log) 
{
	CString strText;
	m_richeditLog.GetWindowTextW(strText);

	strText += log;
	strText += TEXT("\n");

	m_richeditLog.SetWindowTextW(strText);

	m_richeditLog.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
}

UINT CWifiDlg::GetTcpPort() const
{
	return GetDlgItemInt(IDC_EDIT_PORT);
}

CString CWifiDlg::GetPhoneIP() const 
{
	CString strIp;

	m_ctrlPhoneIp.GetWindowTextW(strIp);

	return strIp;
}

void CWifiDlg::EnableCtrls(BOOL bEnable)
{
	static UINT ctrls[] = {
		IDC_BTN_DISCONNECT, IDC_BTN_CONNECT, IDC_EDIT_PORT, 
		IDC_IPADDRESS_PHONE, IDC_IPADDRESS_WIFI,
		IDC_COMBO_NETWORK, IDC_CHECK_ADB_WIRELESS,
	};

	int len = sizeof(ctrls) / sizeof(ctrls[0]);

	for (int i = 0; i < len; i++)
	{
		CWnd *pWnd = GetDlgItem(ctrls[i]);
		if (pWnd) {
			pWnd->EnableWindow(bEnable);
		}
	}

}

BOOL CWifiDlg::IsNoTcpPort() const
{
	int state = ((CButton*)GetDlgItem(IDC_CHECK_ADB_WIRELESS))->GetCheck();

	return (state == BST_CHECKED);
}

void ThreadConnectWifi(LPVOID lP) 
{
	CWifiDlg *pDlg = (CWifiDlg*)lP;

	CString strPhoneIp = pDlg->GetPhoneIP();
	UINT port = pDlg->GetTcpPort();

	pDlg->AppendText(TEXT("> Start Connect..."));
	PARAM_T para;
	para.nType = CMD_INFINITE;

	pDlg->EnableCtrls(FALSE);
	if (pDlg->IsNoTcpPort())
	{
		para.strCmd = TEXT("adb connect ");
		para.strCmd += strPhoneIp;

		pDlg->AppendText(para.strCmd);
		para.bRet = CAdbInterface::CreateAdbProcess(&para);
		pDlg->AppendText(para.strReturn);

		if (para.strReturn.Find(TEXT("connected")) != -1)
		{
			pDlg->AppendText(TEXT("--- ������ ---"));
		}
	}
	else 
	{
		para.strCmd.Format(TEXT("adb tcpip %d"), port);
		pDlg->AppendText(para.strCmd);
		para.bRet = CAdbInterface::CreateAdbProcess(&para);
		pDlg->AppendText(para.strReturn);

		para.strCmd = TEXT("adb connect ");
		para.strCmd += strPhoneIp;
		para.strCmd.AppendFormat(TEXT(":%d"), port);

		pDlg->AppendText(para.strCmd);
		para.bRet = CAdbInterface::CreateAdbProcess(&para);
		pDlg->AppendText(para.strReturn);


		if (para.strReturn.Find(TEXT("connected to")) != -1) 
		{
			MessageBox(pDlg->GetSafeHwnd(), TEXT("������Ͽ�USB����"), TEXT("��ʾ"), MB_ICONINFORMATION);
		}
	}
	pDlg->SetInOperation(FALSE);
	PostMessage(g_hMainWnd, UWM_RECONNECT_DEVICE, 0, 0);
	pDlg->EnableCtrls(TRUE);
	return;
}

void CWifiDlg::OnBnClickedBtnConnect()
{

	if (m_ctrlPhoneIp.IsBlank()) 
	{
		MessageBox(TEXT("����д�ֻ�IP��ַ"));
		return;
	}
	else 
	{
		DWORD addrServerIp, addrPhoneIp;
		m_ctrlIp.GetAddress(addrServerIp);
		m_ctrlPhoneIp.GetAddress(addrPhoneIp);
		if (addrPhoneIp == (addrServerIp+1))
		{
			CString strInfo;
			CString strIp;
			m_ctrlPhoneIp.GetWindowTextW(strIp);
			strInfo.Format(TEXT("�ֻ���IP��: %s ��?"), strIp);
			if (IDYES != MessageBox(strInfo, TEXT("����"), MB_YESNO | MB_ICONWARNING))
			{
				return;
			}
		}
	}
	
	_beginthread(ThreadConnectWifi, 0, this);
}	


void CWifiDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

}

void CWifiDlg::OnSelchangeComboNetwork()
{
	int nCurSel = m_cbNetworks.GetCurSel();
	m_ctrlIp.SetWindowTextW(m_vecNetworks.at(nCurSel).ip);

	DWORD addr;
	m_ctrlIp.GetAddress(addr);
	addr++;
	m_ctrlPhoneIp.SetAddress(addr);

}

unsigned WINAPI ThreadSwitchUSB(LPVOID lP)
{
	CWifiDlg *pDlg = (CWifiDlg*)lP;

	pDlg->EnableCtrls(FALSE);
	pDlg->RestartUSB();
	pDlg->EnableCtrls(TRUE);

	return 0;
}


void CWifiDlg::OnBnClickedBtnDisconnect()
{
	_beginthreadex(NULL, 0, ThreadSwitchUSB, this, 0, NULL);
}

BOOL CWifiDlg::RestartUSB()
{
	AppendText(TEXT("> Switch to USB mode"));
	PARAM_T para;
	para.nType = CMD_INFINITE;
	//para.strCmd = TEXT("adb kill-server");

	//AppendText(para.strCmd);
	//para.bRet = ADB.CreateAdbProcess(&para);
	//AppendText(para.strReturn);

	para.strCmd = TEXT("adb usb");
	AppendText(para.strCmd);
	para.bRet = ADB.CreateAdbProcess(&para);
	AppendText(para.strReturn);

	if ((para.strReturn.Find(TEXT("restarting in USB mode")) != -1)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


void CWifiDlg::OnBnClickedCheckAdbWireless()
{
	BOOL bChecked = m_chkNoTcpPort.GetCheck() == BST_CHECKED;

	m_editTcpPort.EnableWindow(!bChecked);
}


void CWifiDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	SetInOperation(FALSE);
	CDialogEx::OnClose();
}


void CWifiDlg::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	// TODO: �ڴ˴������Ϣ����������
	delete this;
}
