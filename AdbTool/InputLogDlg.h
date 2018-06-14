#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "InterruptManager.h"
// CInputLogDlg 对话框

#define KERN_EMERG 0 /* system is unusable */
#define KERN_ALERT 1 /* action must be taken immediately */
#define KERN_CRIT 2 /* critical conditions */
#define KERN_ERR 3 /* error conditions */
#define KERN_WARNING 4 /* warning conditions */
#define KERN_NOTICE 5 /* normal but significant condition */
#define KERN_INFO 6 /* informational */
#define KERN_DEBUG 7 /* debug-level messages */
#define KERN_ALL 8 /* debug-level messages */

class CInputLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputLogDlg)

public:
	CInputLogDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputLogDlg();

// 对话框数据
	enum { IDD = IDD_DLG_INPUT_LOG };

protected:
	void SelectCpu(int cpu);
	void SelectInterrupt(int pos);
	void RefreshInterrupt();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnGetEvent();
	afx_msg void OnBnClickedBtnCatProcKmsg();
	afx_msg void OnBnClickedBtnDmesg();


private:
	CString m_strAllAttrs;
	CComboBox m_cbInputDevices;
	CRichEditCtrl m_richInputAttrs;
	CInterruptManager m_mgrInterrupts;
	CAndroidIntrrupt m_droidCurrentInt;
public:
	afx_msg void OnCbnSelchangeComboInput();
	CComboBox m_cbLogLevel;
	afx_msg void OnCbnSelchangeComboLoglevels();
	afx_msg void OnBnClickedBtnAdbLogcat();
	CComboBox m_cbLogcatFilterLevel;
	CEdit m_editLogcatFilter;
	afx_msg void OnNcDestroy();
	afx_msg void OnBnClickedBtnIntRefresh();
	CComboBox m_cbIntCpus;
	CComboBox m_cbIntNames;
	CEdit m_editDetails;
	afx_msg void OnCbnSelchangeComboIntName();
	afx_msg void OnCbnSelchangeComboIntCpu();
};
