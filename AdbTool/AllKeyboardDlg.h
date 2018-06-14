#pragma once
#include "afxwin.h"
#include "keycode.h"

// CAllKeyboardDlg �Ի���

class CAllKeyboardDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAllKeyboardDlg)

public:
	CAllKeyboardDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAllKeyboardDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
// �Ի�������
	enum { IDD = IDD_DIALOG_ALL_KEYBOARD };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbSpecialCharacters;
	CComboBox m_cbSpecialOperations;
	afx_msg void OnBnClickedBtnKeyboardSendText();
	afx_msg void OnCbnSelchangeComboSpecialChar();
	afx_msg void OnCbnSelchangeComboSpecialOperations();
	afx_msg void OnBnClickedCheckCaptions();
	afx_msg void OnNcDestroy();
};
