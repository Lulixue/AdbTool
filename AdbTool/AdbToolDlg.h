
// AdbToolDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include <vector>
using std::vector;


// CAdbToolDlg 对话框
class CAdbToolDlg : public CDialogEx
{
// 构造
public:
	CAdbToolDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CAdbToolDlg();

// 对话框数据
	enum { IDD = IDD_ADBTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);


private:
	enum {
		MODE_DEVELOPER = 0,
		MODE_FAE,
		MODE_NORMAL,
	};
	int m_nWorkMode;
	void LoadSettings();

public:
	void SendMultiSwipe();
private:
	void InitResolution();
	void RegisterDevice();
	void UpdateStatus();
	void DrawScreen();
	void DrawSwipe(vector<CPoint> points);
    void OnDialogDebug();
	void SendSwipe();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnVolumnplus();
	afx_msg void OnBnClickedBtnLeft();
	afx_msg void OnBnClickedBtnReboot();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnBnClickedBtnBack();
	afx_msg void OnBnClickedBtnHome();
	afx_msg void OnBnClickedBtnMenu();
	afx_msg void OnBnClickedBtnVolumnminus();
	afx_msg void OnBnClickedBtnSendtext();
	afx_msg void OnBnClickedBtnRight();
	afx_msg void OnStnDblclickStaticScreen();


private:
	CString m_strTmpDir;
	CString m_strScreenPath;
	CRect m_rcScreen;
	CRect m_rcClientScreen;
	CRect m_rcOriginalWindow;
	CPoint m_pntResolution;
	double m_dblXFactor;
	double m_dblYFactor;
	CRect m_rcSwipePoint;
	CEdit m_editXRes;
	CEdit m_editYRes;
	CStatic m_staticScreen;
	CStatic m_staticResolution;
	CStatic m_staticTpCoord;
	CStatic m_staticCoordinate;
	CMenu m_menuMain;

	BOOL m_bSwipeFlag;
	BOOL m_bMultiSwipeFlag;

	BOOL m_bDisableMouse;
	BOOL m_bLoadScreenShot;
	vector<CPoint> m_vecMSwipeTpPoints;
	vector<CPoint> m_vecMSwipeScrPoints;
public:
	BOOL IsNormalMode() const { return m_nWorkMode == MODE_NORMAL; }
	BOOL IsDeveloperMode() const { return m_nWorkMode == MODE_DEVELOPER; }
    void SetCompileDateTime();
	void ResetSwipe();
	void InitConnectedDevice();
	afx_msg void OnBnClickedBtnAutoUnlock();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnSetResolution();
	afx_msg void OnBnClickedBtnBackspace();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedBtnIicOperator();


	void UpdateScreen(int nSleepMs);
	LRESULT OnProcAvailable(WPARAM wParam, LPARAM lParam);
	LRESULT OnReconnectDevice(WPARAM wParam, LPARAM lParam);
	void OnUpdateScreen(int nSleepMs);
	afx_msg void OnBnClickedBtnReconnect();
	afx_msg void OnBnClickedBtnInstallApk();
	afx_msg void OnBnClickedBtnWifi();
	afx_msg void OnBnClickedBtnScreenOff();
	afx_msg void OnBnClickedBtnScreenOn();
	afx_msg void OnBnClickedBtnInputLog();
	afx_msg void OnBnClickedChkLoadScreenshot();
    afx_msg void OnBnClickedBtnMtkLogger();
	afx_msg void OnBnClickedBtnKillAdb();
	afx_msg void OnBnClickedBtnSaveScreen();
	afx_msg void OnBnClickedCheckMultiSwipe();
	CButton m_chkMultiSwipe;
	afx_msg void OnBnClickedBtnAdbConsole();
	afx_msg void OnBnClickedBtnFileManager();
protected:
	afx_msg LRESULT OnUmsgFileManagerClose(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedBtnProcessManager();
	afx_msg void OnClose();
	afx_msg void OnPhoneReconnect();
	afx_msg void OnScreenScreenon();
	afx_msg void OnScreenScreenoff();
	afx_msg void OnVolumnVolumn();
	afx_msg void OnVolumnVolumn32791();
	afx_msg void OnOperationsReboot();
	afx_msg void OnOperationsInput();
	afx_msg void OnPhoneExit();
	afx_msg void OnManagersFilemanager();
	afx_msg void OnManagersPackagemanager();
	afx_msg void OnManagersProcessmanager();
	afx_msg void OnToolsInputlog();
	afx_msg void OnToolsAdbconsole();
	afx_msg void OnAboutAbout();
	afx_msg void OnOperationsKill();
	afx_msg void OnToolsDebuglogger();
	afx_msg void OnPhonePhoneinfo();
};
