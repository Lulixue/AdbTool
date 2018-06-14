#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>

using std::vector;


typedef struct 
{
	CString name;
	CString ip;
	CString mac;
	CString desc;
}NETWORK_T, *P_NETWORK_T;

// CWifiDlg �Ի���

class CWifiDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWifiDlg)

public:
	CWifiDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWifiDlg();

// �Ի�������
	enum { IDD = IDD_DLG_WIFI };

protected:
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	void Init(); 

private:
	vector<NETWORK_T> m_vecNetworks;
	BOOL m_bInOperation;


public:
	void SetInOperation(BOOL bOp) { m_bInOperation = bOp; }
	BOOL IsInOperation() const { return m_bInOperation; }
	BOOL RestartUSB();
	void EnableCtrls(BOOL bEnable);
	void AppendText(CString log);
	CString GetPhoneIP() const;
	UINT GetTcpPort() const;
	BOOL IsNoTcpPort() const ;
	CComboBox m_cbNetworks;
	afx_msg void OnBnClickedBtnConnect();
	CRichEditCtrl m_richeditLog;
	CIPAddressCtrl m_ctrlIp;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboNetwork();
	CIPAddressCtrl m_ctrlPhoneIp;
	afx_msg void OnBnClickedBtnDisconnect();
	CEdit m_editTcpPort;
	afx_msg void OnBnClickedCheckAdbWireless();
	CButton m_chkNoTcpPort;
	afx_msg void OnClose();
	afx_msg void OnNcDestroy();
};
