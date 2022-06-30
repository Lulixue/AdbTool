
// AdbToolDlg.cpp : ÊµÏÖÎÄ¼þ
//

#include "stdafx.h"
#include "AdbTool.h"
#include "AdbToolDlg.h"
#include "afxdialogex.h"
#include "AdbInterface.h"
#include <DBT.h>
#include "input.h"
#include "ProcInterface.h"
#include "WifiDlg.h"
#include "InputLogDlg.h"
#include "DialogDebug.h"
#include "DialogSelectDevice.h"
#include "DlgFileManager.h"
#include "AllKeyboardDlg.h"
#include "DialogPackageManager.h"
#include "DialogProcessManager.h"
#include "DialogPhoneInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define POINT_RADIUS		10

vector<int> g_vecOtherCtrls;	// ³ýÏÔÊ¾½çÃæÍâ±ðµÄ¿Ø¼þ
vector<CRect> g_vecOtherCtrlsDefPos;
CCriticalSection g_cs;

CWifiDlg *g_pDlgWifi;
CInputLogDlg *g_pDlgInputLog;
CDialogDebug *g_pDlgDebug;
CAllKeyboardDlg *g_pDlgAllKeyboard;
CDlgFileManager *g_pDlgFileManager;
CDialogPackageManager *g_pDlgPackageManager;
CDialogProcessManager *g_pDlgProcessManager;
CDialogPhoneInfo *g_pDlgPhoneInfo;
HWND g_hMainWnd;

BOOL g_bDisableDeviceChange = FALSE;
CString g_strToolDate;
CString g_strCopyRight;

// ÓÃÓÚÓ¦ÓÃ³ÌÐò¡°¹ØÓÚ¡±²Ëµ¥ÏîµÄ CAboutDlg ¶Ô»°¿ò

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ¶Ô»°¿òÊý¾Ý
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö

// ÊµÏÖ
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);

    
    SetDlgItemTextW(IDC_STATIC_VERSION, g_strToolDate.GetString());
	SetDlgItemTextW(IDC_STATIC_COPYRIGHT, g_strCopyRight.GetString());
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CAdbToolDlg ¶Ô»°¿ò

CAdbToolDlg *g_pToolDlg;

CAdbToolDlg::CAdbToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAdbToolDlg::IDD, pParent), m_bSwipeFlag(FALSE), 
		m_bLoadScreenShot(FALSE), m_bMultiSwipeFlag(FALSE),
		m_bDisableMouse(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

#define DestroyToolDialog(pDlg) \
	do {\
		if (pDlg) { \
            /* to avoid memory leak */\
            ::SendMessage(pDlg->GetSafeHwnd(), WM_CLOSE, 0, 0);\
			pDlg->DestroyWindow();\
			pDlg = NULL;\
		}\
	} while(0)

CAdbToolDlg::~CAdbToolDlg() 
{
    TRACE("%s\n", __FUNCTION__);
	DestroyToolDialog(g_pDlgFileManager);
	DestroyToolDialog(g_pDlgWifi);
	DestroyToolDialog(g_pDlgInputLog);
	DestroyToolDialog(g_pDlgDebug);
	DestroyToolDialog(g_pDlgAllKeyboard);
	DestroyToolDialog(g_pDlgPackageManager);
	DestroyToolDialog(g_pDlgProcessManager);
	DestroyToolDialog(g_pDlgPhoneInfo);
}

void CAdbToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SCREEN, m_staticScreen);
	DDX_Control(pDX, IDC_STATIC_COORD, m_staticCoordinate);
	DDX_Control(pDX, IDC_STATIC_RESOLUTION, m_staticResolution);
	DDX_Control(pDX, IDC_STATIC_TP_COORD, m_staticTpCoord);
	DDX_Control(pDX, IDC_EDIT_X_RESOLUTION, m_editXRes);
	DDX_Control(pDX, IDC_EDIT_Y_RESOLUTION, m_editYRes);
	DDX_Control(pDX, IDC_CHECK_MULTI_SWIPE, m_chkMultiSwipe);
}

BEGIN_MESSAGE_MAP(CAdbToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BTN_UP, &CAdbToolDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_OK, &CAdbToolDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_VOLUMNPLUS, &CAdbToolDlg::OnBnClickedBtnVolumnplus)
	ON_BN_CLICKED(IDC_BTN_LEFT, &CAdbToolDlg::OnBnClickedBtnLeft)
	ON_BN_CLICKED(IDC_BTN_Reboot, &CAdbToolDlg::OnBnClickedBtnReboot)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CAdbToolDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BTN_BACK, &CAdbToolDlg::OnBnClickedBtnBack)
	ON_BN_CLICKED(IDC_BTN_HOME, &CAdbToolDlg::OnBnClickedBtnHome)
	ON_BN_CLICKED(IDC_BTN_MENU, &CAdbToolDlg::OnBnClickedBtnMenu)
	ON_BN_CLICKED(IDC_BTN_VOLUMNMINUS, &CAdbToolDlg::OnBnClickedBtnVolumnminus)
	ON_BN_CLICKED(IDC_BTN_SENDTEXT, &CAdbToolDlg::OnBnClickedBtnSendtext)
	ON_BN_CLICKED(IDC_BTN_RIGHT, &CAdbToolDlg::OnBnClickedBtnRight)
	ON_STN_DBLCLK(IDC_STATIC_SCREEN, &CAdbToolDlg::OnStnDblclickStaticScreen)
	ON_BN_CLICKED(IDC_BTN_AUTO_UNLOCK, &CAdbToolDlg::OnBnClickedBtnAutoUnlock)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_SET_RESOLUTION, &CAdbToolDlg::OnBnClickedBtnSetResolution)
	ON_BN_CLICKED(IDC_BTN_BACKSPACE, &CAdbToolDlg::OnBnClickedBtnBackspace)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BTN_IIC_OPERATOR, &CAdbToolDlg::OnBnClickedBtnIicOperator)
	ON_MESSAGE(UWM_PROC_AVAILABLE, &CAdbToolDlg::OnProcAvailable)
	ON_MESSAGE(UWM_RECONNECT_DEVICE, &CAdbToolDlg::OnReconnectDevice)
	ON_BN_CLICKED(IDC_BTN_RECONNECT, &CAdbToolDlg::OnBnClickedBtnReconnect)
	ON_BN_CLICKED(IDC_BTN_INSTALL_APK, &CAdbToolDlg::OnBnClickedBtnInstallApk)
	ON_BN_CLICKED(IDC_BTN_WIFI, &CAdbToolDlg::OnBnClickedBtnWifi)
	ON_BN_CLICKED(IDC_BTN_SCREEN_OFF, &CAdbToolDlg::OnBnClickedBtnScreenOff)
	ON_BN_CLICKED(IDC_BTN_SCREEN_ON, &CAdbToolDlg::OnBnClickedBtnScreenOn)
	ON_BN_CLICKED(IDC_BTN_INPUT_LOG, &CAdbToolDlg::OnBnClickedBtnInputLog)
	ON_BN_CLICKED(IDC_CHK_LOAD_SCREENSHOT, &CAdbToolDlg::OnBnClickedChkLoadScreenshot)
    ON_BN_CLICKED(IDC_BTN_MTK_LOGGER, &CAdbToolDlg::OnBnClickedBtnMtkLogger)
	ON_BN_CLICKED(IDC_BTN_KILL_ADB, &CAdbToolDlg::OnBnClickedBtnKillAdb)
	ON_BN_CLICKED(IDC_BTN_SAVE_SCREEN, &CAdbToolDlg::OnBnClickedBtnSaveScreen)
	ON_BN_CLICKED(IDC_CHECK_MULTI_SWIPE, &CAdbToolDlg::OnBnClickedCheckMultiSwipe)
	ON_BN_CLICKED(IDC_BTN_ADB_CONSOLE, &CAdbToolDlg::OnBnClickedBtnAdbConsole)
	ON_BN_CLICKED(IDC_BTN_FILE_MANAGER, &CAdbToolDlg::OnBnClickedBtnFileManager)
	ON_MESSAGE(UMSG_FILE_MANAGER_CLOSE, &CAdbToolDlg::OnUmsgFileManagerClose)
	ON_BN_CLICKED(IDC_BTN_PROCESS_MANAGER, &CAdbToolDlg::OnBnClickedBtnProcessManager)
	ON_WM_CLOSE()
	ON_COMMAND(ID_PHONE_RECONNECT, &CAdbToolDlg::OnPhoneReconnect)
	ON_COMMAND(ID_SCREEN_SCREENON, &CAdbToolDlg::OnScreenScreenon)
	ON_COMMAND(ID_SCREEN_SCREENOFF, &CAdbToolDlg::OnScreenScreenoff)
	ON_COMMAND(ID_VOLUMN_VOLUMN, &CAdbToolDlg::OnVolumnVolumn)
	ON_COMMAND(ID_VOLUMN_VOLUMN32791, &CAdbToolDlg::OnVolumnVolumn32791)
	ON_COMMAND(ID_OPERATIONS_REBOOT, &CAdbToolDlg::OnOperationsReboot)
	ON_COMMAND(ID_OPERATIONS_INPUT, &CAdbToolDlg::OnOperationsInput)
	ON_COMMAND(ID_PHONE_EXIT, &CAdbToolDlg::OnPhoneExit)
	ON_COMMAND(ID_MANAGERS_FILEMANAGER, &CAdbToolDlg::OnManagersFilemanager)
	ON_COMMAND(ID_MANAGERS_PACKAGEMANAGER, &CAdbToolDlg::OnManagersPackagemanager)
	ON_COMMAND(ID_MANAGERS_PROCESSMANAGER, &CAdbToolDlg::OnManagersProcessmanager)
	ON_COMMAND(ID_TOOLS_INPUTLOG, &CAdbToolDlg::OnToolsInputlog)
	ON_COMMAND(ID_TOOLS_ADBCONSOLE, &CAdbToolDlg::OnToolsAdbconsole)
	ON_COMMAND(ID_ABOUT_ABOUT, &CAdbToolDlg::OnAboutAbout)
	ON_COMMAND(ID_OPERATIONS_KILL, &CAdbToolDlg::OnOperationsKill)
	ON_COMMAND(ID_TOOLS_DEBUGLOGGER, &CAdbToolDlg::OnToolsDebuglogger)
	ON_COMMAND(ID_PHONE_PHONEINFO, &CAdbToolDlg::OnPhonePhoneinfo)
END_MESSAGE_MAP()


void CAdbToolDlg::RegisterDevice()
{
	const GUID GUID_DEVINTERFACE_LIST[] = { 
		{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
		{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
		{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },	/* HID */
		{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }};

		HDEVNOTIFY   hDevNotify;

		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
		ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
		NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		for(int i=0; i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID); i++)
		{
			NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
			hDevNotify = RegisterDeviceNotification(m_hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
			if( !hDevNotify ) {
				::MessageBox(m_hWnd, TEXT("×¢²áUSBÉè±¸Í¨ÖªÊ§°Ü"), TEXT("´íÎó"), MB_ICONERROR);
				break;
			}
		}
}

BOOL CAdbToolDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	switch (nEventType)
	{
	case DBT_DEVICEARRIVAL:
		if (!ADB.IsConnected()) 
		{
			UpdateStatus();
		}
		return TRUE;
	case DBT_DEVICEREMOVECOMPLETE:
		if (ADB.IsConnected()) 
		{
			UpdateStatus();
		}
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

UINT WINAPI ThreadUpdateStatus(LPVOID lP) 
{
	g_cs.Lock();
	CString strTitle;
	//static BOOL bInited = FALSE;

	ADB.Init();

	/*if (!bInited)  {
		bInited = TRUE;
	} */
	if (g_pDlgWifi && g_pDlgWifi->IsInOperation()) {
		g_cs.Unlock();
		return 0;
	}

	for (int i = 0; i < 3; i++)
	{
		ADB.GetDevices();
		if (ADB.IsMultiDevConnected()) {
			CDialogSelectDevice  sddlg(g_pToolDlg);
			sddlg.DoModal();
			Sleep(1000);
		}
		if (ADB.IsConnected())
		{
			g_pToolDlg->InitConnectedDevice();
			break;
		}
		Sleep(100);
	}

	if (ADB.IsConnected())
	{
		strTitle = TEXT("Android Dev: ");
		strTitle += ADB.GetModel();
		strTitle.AppendFormat(TEXT("(%s)"), ADB.GetSerialNo());
		strTitle += TEXT(" Connected");
		if (ADB.IsRooted()) {
			strTitle += TEXT("(Root)");
		}
	}
	else
	{
		strTitle = TEXT("Disconnected, Please Connect Android Device!");
	}

	g_pToolDlg->SetWindowText(strTitle);
	if (ADB.IsConnected()) 
	{
		g_pToolDlg->UpdateScreen(0);
	}
	
	g_cs.Unlock();
	return 0;
}

void CAdbToolDlg::SetCompileDateTime()
{
    const char *szEnglishMonth[12]= {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec",
    };
    char szTmpDate[100]={0}; 
    char szTmpTime[100]={0}; 
    char szMonth[10]={0}; 
    wchar_t szDateTime[250] = {0};
    int iYear, iMonth = 1, iDay;
    int iHour,iMin,iSec;

    //»ñÈ¡±àÒëÈÕÆÚ¡¢Ê±¼ä 
    sprintf_s(szTmpDate,"%s", __DATE__); //"Sep 18 2010" 
    sprintf_s(szTmpTime,"%s", __TIME__); //"10:59:19" 

	memcpy(szMonth, szTmpDate, 3);
	for (int i=0; i < 12; i++) 
	{ 
		if (strncmp(szMonth, szEnglishMonth[i],3) == 0) 
		{ 
			iMonth=i+1; 
			break; 
		} 
	} 
	memset(szMonth, 0, 10);
	memcpy(szMonth, szTmpDate+4, 2);
	iDay = atoi(szMonth);

	memset(szMonth, 0, 10);
	memcpy(szMonth, szTmpDate+7, 4);
	iYear = atoi(szMonth);


	memset(szMonth, 0, 10);
	memcpy(szMonth, szTmpTime, 2);
	iHour = atoi(szMonth);

	memset(szMonth, 0, 10);
	memcpy(szMonth, szTmpTime+3, 2);
	iMin = atoi(szMonth);


	memset(szMonth, 0, 10);
	memcpy(szMonth, szTmpTime+6, 2);
	iSec = atoi(szMonth);


    wsprintf(szDateTime,TEXT("Adb Tool Build@%4d-%02d-%02d_%02d:%02d:%02d"),
        iYear,iMonth,iDay,iHour,iMin,iSec); 

    TRACE(TEXT("%s"), szDateTime);
    TRACE("\n");
    g_strToolDate = szDateTime;


	sprintf_s(szTmpDate, "Lulixue Copyright(C) %d", iYear);
	g_strCopyRight = szTmpDate;

    //g_strToolDate = TEXT("AdbTool, V1.0");
}

void CAdbToolDlg::UpdateStatus()
{
	this->SetWindowTextW(TEXT("Connecting Device..."));
	_beginthreadex(NULL, 0, ThreadUpdateStatus, NULL, 0, NULL);
	
}


BOOL CAdbToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ½«¡°¹ØÓÚ...¡±²Ëµ¥ÏîÌí¼Óµ½ÏµÍ³²Ëµ¥ÖÐ¡£

	// IDM_ABOUTBOX ±ØÐëÔÚÏµÍ³ÃüÁî·¶Î§ÄÚ¡£
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// ÉèÖÃ´Ë¶Ô»°¿òµÄÍ¼±ê¡£µ±Ó¦ÓÃ³ÌÐòÖ÷´°¿Ú²»ÊÇ¶Ô»°¿òÊ±£¬¿ò¼Ü½«×Ô¶¯
	//  Ö´ÐÐ´Ë²Ù×÷

	SetIcon(m_hIcon, TRUE);			// ÉèÖÃ´óÍ¼±ê
	SetIcon(m_hIcon, FALSE);		// ÉèÖÃÐ¡Í¼±ê

	CAdbInterface::StaticInit();
	CAdbProcessManager::Init();

	LoadSettings();
	GetWindowRect(&m_rcOriginalWindow);
	//ScreenToClient(&m_rcOriginalWindow);

	m_staticScreen.GetWindowRect(&m_rcScreen);
	m_rcClientScreen = m_rcScreen;
	ScreenToClient(&m_rcClientScreen);

	wchar_t wszPath[MAX_PATH+1] = {0};
	GetTempPath(MAX_PATH, wszPath);
	m_strTmpDir = wszPath;
	m_strScreenPath = m_strTmpDir + TEXT("screen.png");

	RegisterDevice();
	//ADB.Init();

	g_pToolDlg = this;
	UpdateStatus();

	g_hMainWnd = this->GetSafeHwnd();


	CWnd* pWnd = GetWindow(GW_CHILD);   
	while (pWnd) 
	{   
		CRect rc;
		GetDlgItem(pWnd->GetDlgCtrlID())->GetWindowRect(&rc);
		ScreenToClient(&rc);
		g_vecOtherCtrls.push_back(pWnd->GetDlgCtrlID());
		g_vecOtherCtrlsDefPos.push_back(rc);
		
		pWnd = pWnd->GetWindow(GW_HWNDNEXT);
	}
    SetCompileDateTime();

	m_menuMain.LoadMenuW(IDR_MENU_MAIN);
	SetMenu(&m_menuMain);
	return TRUE;  // ³ý·Ç½«½¹µãÉèÖÃµ½¿Ø¼þ£¬·ñÔò·µ»Ø TRUE
}


void CAdbToolDlg::LoadSettings()
{
	CString strIniPath = TEXT(".\\adb\\Settings.ini");

	WCHAR strBuffer[MAX_PATH] = {0};
	int nRet;
	
	nRet = GetPrivateProfileString(TEXT("Settings"), TEXT("mode"), TEXT(""), 
		strBuffer, MAX_PATH, strIniPath);

	m_nWorkMode = MODE_NORMAL;
	if (nRet > 0) {
		CString strMode(strBuffer);

		if (!strMode.CompareNoCase(TEXT("fae")))
		{
			m_nWorkMode = MODE_FAE;
		}
		else if (!strMode.CompareNoCase(TEXT("dev")))
		{
			m_nWorkMode = MODE_DEVELOPER;
		}
	}
}


void CAdbToolDlg::InitConnectedDevice()
{
	if (IsDeveloperMode()) 
	{
		ADB.SetPrintkLevel(KERN_ALL);
	}
	InitResolution();

	if (IsNormalMode()) 
	{
		GetDlgItem(IDC_BTN_IIC_OPERATOR)->EnableWindow(FALSE);
		PostMessage(UWM_PROC_AVAILABLE, 1, 0);
	}
}

void CAdbToolDlg::InitResolution()
{
	static BOOL bShowed = FALSE;
	if (ADB.GetResolution(m_pntResolution))
	{
		if (m_pntResolution.x > m_pntResolution.y) {
			int tmp;
			CString strInfo;
			strInfo.Format(TEXT("Resolution is %d*%d (x*y)?"), m_pntResolution.y, m_pntResolution.x);
			
			if (bShowed || (IDYES == MessageBox(strInfo, TEXT("Warning"), MB_ICONQUESTION | MB_YESNO))) {
				tmp = m_pntResolution.y;
				m_pntResolution.y = m_pntResolution.x;
				m_pntResolution.x = tmp;
				bShowed = TRUE;
			}
		}

		SetDlgItemInt(IDC_EDIT_X_RESOLUTION, m_pntResolution.x);
		SetDlgItemInt(IDC_EDIT_Y_RESOLUTION, m_pntResolution.y);

		m_dblXFactor = (m_pntResolution.x * 1.0) / m_rcClientScreen.Width();
		m_dblYFactor = (m_pntResolution.y * 1.0) / m_rcClientScreen.Height();

		m_editXRes.SetReadOnly(TRUE);
		m_editYRes.SetReadOnly(TRUE);
	}
}

void CAdbToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// Èç¹ûÏò¶Ô»°¿òÌí¼Ó×îÐ¡»¯°´Å¥£¬ÔòÐèÒªÏÂÃæµÄ´úÂë
//  À´»æÖÆ¸ÃÍ¼±ê¡£¶ÔÓÚÊ¹ÓÃÎÄµµ/ÊÓÍ¼Ä£ÐÍµÄ MFC Ó¦ÓÃ³ÌÐò£¬
//  Õâ½«ÓÉ¿ò¼Ü×Ô¶¯Íê³É¡£

void CAdbToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ÓÃÓÚ»æÖÆµÄÉè±¸ÉÏÏÂÎÄ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ê¹Í¼±êÔÚ¹¤×÷Çø¾ØÐÎÖÐ¾ÓÖÐ
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// »æÖÆÍ¼±ê
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	DrawScreen();
}

//µ±ÓÃ»§ÍÏ¶¯×îÐ¡»¯´°¿ÚÊ±ÏµÍ³µ÷ÓÃ´Ëº¯ÊýÈ¡µÃ¹â±ê
//ÏÔÊ¾¡£
HCURSOR CAdbToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAdbToolDlg::DrawScreen()
{
	if (m_bLoadScreenShot && ADB.IsConnected())
	{
		if (_waccess(m_strScreenPath, 0)  != 0) 
		{
			return;
		}
		CImage img;
		img.Load(m_strScreenPath);
		if (img.IsNull() || 
			(img.GetHeight() <= 0) ||
			(img.GetWidth() <= 0) )
		{
			return;
		}
		RECT rc;
		m_staticScreen.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.top += 1;
		rc.left += 1;
		rc.bottom -= 1;
		rc.right -= 1;
		if ( (rc.right > rc.left) && (rc.bottom > rc.top)) 
		{
			img.Draw(GetDC()->m_hDC, rc);
		}
	}
}

void CAdbToolDlg::DrawSwipe(vector<CPoint> points)
{
	DrawScreen();
	if (!m_bLoadScreenShot || !m_bSwipeFlag) 
	{
		return;
	}
	CRect rcDrawWnd;
	m_staticScreen.GetWindowRect(&rcDrawWnd);
	ScreenToClient(&rcDrawWnd);

	CDC *pDc = GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDc);

	CImage img;
	img.Load(m_strScreenPath);


	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dcCompatible, rcDrawWnd.Width(), rcDrawWnd.Height());

	CRect rcComp;
	rcComp.left = 0;
	rcComp.top = 0;
	rcComp.bottom = rcDrawWnd.Height();
	rcComp.right = rcDrawWnd.Width();


	CBitmap *pOldBmp = dcCompatible.SelectObject(&bitmap);

	LOGBRUSH lb;  
	lb.lbColor = RGB(255, 255, 255);  
	lb.lbStyle = BS_SOLID;  
	lb.lbHatch = 1;  
	CBrush brush;
	brush.CreateBrushIndirect(&lb); 


	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

	CPen *pOldPen = dcCompatible.SelectObject(&pen);
	CPen *pDcOldPen = pDc->SelectObject(&pen);
	CBrush *pOldBrush = dcCompatible.SelectObject(&brush);


	vector<CPoint>::const_iterator cit = points.cbegin();
	for (; cit != points.cend();) {
		dcCompatible.MoveTo(*cit);

		CRect rc;
		rc.left = cit->x - POINT_RADIUS;
		rc.right = cit->x + POINT_RADIUS;
		rc.top = cit->y - POINT_RADIUS;
		rc.bottom = cit->y + POINT_RADIUS;

		dcCompatible.Ellipse(&rc);
		cit++;
		if (cit != points.cend()) {
			dcCompatible.LineTo(*cit);
		} 
	}

	TransparentBlt(pDc->GetSafeHdc(), rcDrawWnd.left, rcDrawWnd.top, rcDrawWnd.Width(), rcDrawWnd.Height(), 
		dcCompatible.GetSafeHdc(), 0, 0, rcDrawWnd.Width(), rcDrawWnd.Height(), SRCCOPY);


	dcCompatible.SelectObject(pOldBmp);
	dcCompatible.SelectObject(pOldPen);
	dcCompatible.SelectObject(pOldBrush);

	ReleaseDC(&dcCompatible);
	ReleaseDC(pDc);
}

void CAdbToolDlg::OnBnClickedBtnUp()
{
	ADB.SendKeyEvent(KEY_UP);
}


void CAdbToolDlg::OnBnClickedBtnOk()
{
	ADB.SendKeyEvent(KEY_OK);
}


void CAdbToolDlg::OnBnClickedBtnVolumnplus()
{
	ADB.SendKeyEvent(KEY_VOL_PLUS);
}


void CAdbToolDlg::OnBnClickedBtnLeft()
{
	
	ADB.SendKeyEvent(KEY_LEFT);
}


void CAdbToolDlg::OnBnClickedBtnReboot()
{
	
	ADB.Reboot();
}


void CAdbToolDlg::OnBnClickedBtnDown()
{
	
	ADB.SendKeyEvent(KEY_DOWN);
}


void CAdbToolDlg::OnBnClickedBtnBack()
{
	
	ADB.SendKeyEvent(KEY_BACK);
	UpdateScreen(800);
}


void CAdbToolDlg::OnBnClickedBtnHome()
{
	ADB.SendKeyEvent(KEY_HOME);
	UpdateScreen(800);
}



void CAdbToolDlg::OnBnClickedBtnMenu()
{
	ADB.SendKeyEvent(KEY_MENU);
	UpdateScreen(500);
}


void CAdbToolDlg::OnBnClickedBtnVolumnminus()
{
	
	ADB.SendKeyEvent(KEY_VOL_MINUS);
}


void CAdbToolDlg::OnBnClickedBtnSendtext()
{
	WCHAR wszText[251] = {0};
	GetDlgItemText(IDC_EDIT_TEXT, wszText, 250);

	CString txt(wszText);
	if (!txt.IsEmpty()) 
	{
		ADB.SendText(txt);
		return;
	}

	if ((g_pDlgAllKeyboard != NULL) && g_pDlgAllKeyboard->IsWindowVisible())
	{
		g_pDlgAllKeyboard->ShowWindow(SW_NORMAL);
	}
	else {
		g_pDlgAllKeyboard = new CAllKeyboardDlg();
		g_pDlgAllKeyboard->Create(IDD_DIALOG_ALL_KEYBOARD,  GetDesktopWindow());
		g_pDlgAllKeyboard->ShowWindow(SW_SHOWNORMAL);
	}
}


void CAdbToolDlg::OnBnClickedBtnRight()
{
	
	ADB.SendKeyEvent(KEY_RIGHT);
}


void CAdbToolDlg::OnUpdateScreen(int nSleepMs)
{

	CString strTitle;
	this->GetWindowTextW(strTitle);
	if (strTitle.Find(TEXT("Processing")) != -1) 
	{
		return;
	}
	g_cs.Lock();
	CString strProcessing = strTitle;
	strProcessing += TEXT(" (Processing...)");
	this->SetWindowTextW(strProcessing);

	DeleteFile(m_strScreenPath);
	Sleep(nSleepMs);
	ADB.GetScreenShot(m_strScreenPath);

	if (_waccess(m_strScreenPath, 0)  != 0) 
	{
		g_cs.Unlock();
		this->SetWindowTextW(strTitle);
		return;
	}
	CImage img;
	img.Load(m_strScreenPath);
	if (img.IsNull() || 
		(img.GetHeight() <= 0) ||
		(img.GetWidth() <= 0) )
	{
		g_cs.Unlock();
		this->SetWindowTextW(strTitle);
		return;
	}

	RECT rc;
	m_staticScreen.GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.top += 1;
	rc.left += 1;
	rc.bottom -= 1;
	rc.right -= 1;
	if ( (rc.right > rc.left) && (rc.bottom > rc.top)) 
	{
		img.Draw(GetDC()->m_hDC, rc);
	}

	g_cs.Unlock();
	this->SetWindowTextW(strTitle);
	return;
}


UINT WINAPI ThreadUpdateScreen(LPVOID lP) 
{
	if (ADB.IsConnected()) 
	{
		g_pToolDlg->OnUpdateScreen((int)lP);
	}

	return 0;
}

void CAdbToolDlg::UpdateScreen(int nSleepMs)
{
	if (m_bLoadScreenShot) 
	{
		_beginthreadex(NULL, 0, ThreadUpdateScreen,(LPVOID)nSleepMs, 0, NULL);
	}
}

void CAdbToolDlg::OnStnDblclickStaticScreen()
{
	

	UpdateScreen(0);
	
}

BOOL CAdbToolDlg::PreTranslateMessage(MSG* pMsg)
{
	UINT message = pMsg->message;
	if (message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_F4:
			m_chkMultiSwipe.SendMessage(BM_CLICK);
			break;
		case VK_LEFT:
			OnBnClickedBtnLeft();
			break;

		case VK_RIGHT:
			OnBnClickedBtnRight();
			break;

		case VK_UP:
			OnBnClickedBtnUp();
			break;

		case VK_DOWN:
			OnBnClickedBtnDown();
			break;

		case VK_CONTROL:
			OnBnClickedBtnOk();
			break;

		case VK_DELETE:
			OnBnClickedBtnBackspace();
			break;

		case VK_ESCAPE:
			OnBnClickedBtnBack();
			return FALSE;

		case VK_F5:
			OnDialogDebug();
			return FALSE;

		} 
	}
	
	if (m_bDisableMouse) 
	{
		if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
			(message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK))
		{
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CAdbToolDlg::OnBnClickedBtnMtkLogger()
{
   ADB.StartActivity(TEXT("com.mediatek.mtklogger/.MainActivity"));
}


void CAdbToolDlg::OnBnClickedBtnAutoUnlock()
{
	
	ADB.StartActivity(TEXT("com.h3c.ServiceAuto/.AutoUnlock"));
}


void CAdbToolDlg::OnMouseMove(UINT nFlags, CPoint point)
{

	if (!m_staticScreen.IsWindowEnabled()) {
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if (PtInRect(&m_rcClientScreen, point))
	{
		CString strCoord;
		CPoint pntTpCoord;
		pntTpCoord.x = (LONG)((point.x - m_rcClientScreen.left) * m_dblXFactor);
		pntTpCoord.y = (LONG)((point.y - m_rcClientScreen.top) * m_dblYFactor);

		if (!m_bSwipeFlag) 
		{
			//strCoord.Format(TEXT("Coordinate: (%d, %d)"), point.x, point.y);
			//m_staticCoordinate.SetWindowTextW(strCoord);

			m_staticCoordinate.SetWindowTextW(TEXT(""));
		}
		else 
		{
			vector<CPoint> points(m_vecMSwipeScrPoints);
			points.push_back(point);
			DrawSwipe(points);
		}
		strCoord.Format(TEXT("TP: (%d, %d)"), pntTpCoord.x, pntTpCoord.y);
		
		m_staticTpCoord.SetWindowTextW(strCoord);
	}
	else
	{
		if (!m_bSwipeFlag) 
		{
			ResetSwipe();
		}
		m_staticTpCoord.SetWindowTextW(TEXT(""));
	}
	CDialogEx::OnMouseMove(nFlags, point);
}



void CAdbToolDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CAdbToolDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	ResetSwipe();
	if (PtInRect(&m_rcClientScreen, point))
	{
		CPoint pntTpCoord;
		pntTpCoord.x = (LONG)((point.x - m_rcClientScreen.left) * m_dblXFactor);
		pntTpCoord.y = (LONG)((point.y - m_rcClientScreen.top) * m_dblYFactor);

		CString strCoord;

		strCoord.Format(TEXT("Click: (%d, %d)"), pntTpCoord.x, pntTpCoord.y);

		m_staticTpCoord.SetWindowTextW(strCoord);

		ADB.SendClick(pntTpCoord);

		UpdateScreen(1000);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CAdbToolDlg::ResetSwipe()
{
	m_bSwipeFlag = FALSE;
	m_vecMSwipeScrPoints.clear();
	m_vecMSwipeTpPoints.clear();
	//m_staticCoordinate.SetWindowTextW(TEXT(""));
}

void CAdbToolDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	if (PtInRect(&m_rcClientScreen, point))
	{
		UpdateScreen(0);
	}
	ResetSwipe();

	CDialogEx::OnRButtonDblClk(nFlags, point);
}

void CAdbToolDlg::SendSwipe()
{
	vector<CPoint> tpPoints = m_vecMSwipeTpPoints;
	int size = tpPoints.size();

	if (size < 2) {
		return;
	}
	else if (size == 2)
	{
		ADB.SendSwipe(tpPoints[0], tpPoints[1]);
		return;
	}

	//for (int i = 1; i < size; i++)
	//{
	//	ADB.SendSwipe(tpPoints[i-1], tpPoints[i]);
	//}
	int dev = ADB.GetTpDev();
	int pressure = 0x37;
	static int trkid = 0x10;
	int touchmajor = 0x0a;

	ADB.SendEvent(dev, EV_ABS, ABS_MT_SLOT, 0);
	ADB.SendEvent(dev, EV_ABS, ABS_MT_TRACKING_ID, trkid++);
	for (int i = 0; i < size; i++)
	{
		ADB.SendEvent(dev, EV_ABS, ABS_MT_POSITION_X, tpPoints[i].x);
		ADB.SendEvent(dev, EV_ABS, ABS_MT_POSITION_Y, tpPoints[i].y);
		ADB.SendEvent(dev, EV_ABS, ABS_MT_PRESSURE, pressure++);
		ADB.SendEvent(dev, EV_ABS, ABS_MT_TOUCH_MAJOR, touchmajor++);
		ADB.SendEvent(dev, EV_SYN, SYN_REPORT, 0);
	}
	ADB.SendEvent(dev, EV_ABS, ABS_MT_SLOT, 0);
	ADB.SendEvent(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	ADB.SendEvent(dev, EV_SYN, SYN_REPORT, 0);

}

void CAdbToolDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!ADB.IsConnected()) 
	{
		CDialogEx::OnRButtonDown(nFlags, point);
		return;
	}
	if (PtInRect(&m_rcClientScreen, point))
	{
		CPoint pntTpCoord;
		pntTpCoord.x = (LONG)((point.x - m_rcClientScreen.left) * m_dblXFactor);
		pntTpCoord.y = (LONG)((point.y - m_rcClientScreen.top) * m_dblYFactor);
	
		CString strCoord;
		if (!m_bSwipeFlag) 
		{
			m_vecMSwipeScrPoints.clear();
			m_vecMSwipeTpPoints.clear();

			m_bSwipeFlag = TRUE;

			strCoord.Format(TEXT("Swipe: (%d, %d)"), 
								pntTpCoord.x, pntTpCoord.y);
			m_staticCoordinate.SetWindowTextW(strCoord);
		
			m_vecMSwipeTpPoints.push_back(pntTpCoord);
			m_vecMSwipeScrPoints.push_back(point);
			DrawSwipe(m_vecMSwipeScrPoints);
		}
		else 
		{
			CPoint lastTpCoord; 
			lastTpCoord = *m_vecMSwipeTpPoints.rbegin();

			strCoord.Format(TEXT("Swipe:(%d,%d)->(%d,%d)"), 
								lastTpCoord.x, lastTpCoord.y, 
									pntTpCoord.x, pntTpCoord.y);
			m_staticCoordinate.SetWindowTextW(strCoord);

			m_vecMSwipeTpPoints.push_back(pntTpCoord);
			m_vecMSwipeScrPoints.push_back(point);
			DrawSwipe(m_vecMSwipeScrPoints);

			if (!m_bMultiSwipeFlag) {
				SendSwipe();
				m_bSwipeFlag = FALSE;
				UpdateScreen(900);
			}
		}
	}
	else 
	{
		ResetSwipe();
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CAdbToolDlg::OnBnClickedBtnSetResolution()
{
	
	if (m_editXRes.GetStyle() & ES_READONLY)
	{
		m_editXRes.SetReadOnly(FALSE);
		m_editYRes.SetReadOnly(FALSE);
		m_editXRes.ShowCaret();
		m_editYRes.ShowCaret();
	}
	else
	{
		m_pntResolution.x = GetDlgItemInt(IDC_EDIT_X_RESOLUTION);
		m_pntResolution.y = GetDlgItemInt(IDC_EDIT_Y_RESOLUTION);

		m_dblXFactor = (m_pntResolution.x * 1.0) / m_rcClientScreen.Width();
		m_dblYFactor = (m_pntResolution.y * 1.0) / m_rcClientScreen.Height();

		m_editXRes.SetReadOnly(TRUE);
		m_editYRes.SetReadOnly(TRUE);
		m_editXRes.HideCaret();
		m_editYRes.HideCaret();
	}
	
}


void CAdbToolDlg::OnBnClickedBtnBackspace()
{
	
	ADB.SendKeyEvent(KEY_DELETE);
}

void CAdbToolDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcNewWindows;
	GetWindowRect(&rcNewWindows);

	

	if (IsWindow(m_staticScreen.GetSafeHwnd()))
	{
		static CRect rcTmpScreen = m_rcClientScreen;
		int deltaX = rcNewWindows.Width() - m_rcOriginalWindow.Width();
		int deltaY = rcNewWindows.Height() - m_rcOriginalWindow.Height();
		m_rcClientScreen.bottom = rcTmpScreen.bottom + deltaY ;
		m_rcClientScreen.right = rcTmpScreen.right + deltaX;

		vector<int>::const_iterator cit = g_vecOtherCtrls.begin();
		vector<CRect>::const_iterator citRc = g_vecOtherCtrlsDefPos.begin();
		for (; cit != g_vecOtherCtrls.end(); cit++, citRc++) 
		{
			CRect rc = *citRc;
			rc.left += deltaX;
			rc.right += deltaX;

			GetDlgItem(*cit)->MoveWindow(&rc, 0);
		}

		m_staticScreen.MoveWindow(&m_rcClientScreen, 0);

		m_dblXFactor = (m_pntResolution.x * 1.0) / m_rcClientScreen.Width();
		m_dblYFactor = (m_pntResolution.y * 1.0) / m_rcClientScreen.Height();

		Invalidate();

		UpdateScreen(0);
	}

}


void CAdbToolDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);

	

}


void CAdbToolDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	lpMMI->ptMinTrackSize.x = 645;
	lpMMI->ptMinTrackSize.y = 461;
	//lpMMI->ptMaxTrackSize.x = 645;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void ThreadGetIICProc(LPVOID lP) {

	BOOL bShowWindow = *((BOOL *)lP);
	BOOL bRet = IPROC.OpenProc();

	if (bShowWindow) { 
		CString strInfo;
		strInfo += IPROC.GetAvailableI2CInterface();
		g_pToolDlg->MessageBox( strInfo);
	}
	g_pToolDlg->GetDlgItem(IDC_BTN_IIC_OPERATOR)->EnableWindow(TRUE);
}

LRESULT CAdbToolDlg::OnProcAvailable(WPARAM wParam, LPARAM lParam)
{
	BOOL bShowWindow = ((int)wParam) == 0;
	_beginthread(ThreadGetIICProc, 0, &bShowWindow);
	
	return 0;
}



LRESULT CAdbToolDlg::OnReconnectDevice(WPARAM wParam, LPARAM lParam)
{
	UpdateStatus();

	return 0;
}

void CAdbToolDlg::OnBnClickedBtnIicOperator()
{
	
	
	//if (g_pDlgOperator != NULL) 
	//{
	//	g_pDlgOperator->ShowWindow(SW_NORMAL);
	//}
	//else 
	{
		GetDlgItem(IDC_BTN_IIC_OPERATOR)->EnableWindow(FALSE);
		PostMessage(UWM_PROC_AVAILABLE, 0, 0);
	}
}


void CAdbToolDlg::OnBnClickedBtnReconnect()
{
	
	PostMessage(UWM_RECONNECT_DEVICE, 0, 0);
}



void CAdbToolDlg::OnBnClickedBtnInstallApk()
{

	DestroyToolDialog(g_pDlgPackageManager);
	g_pDlgPackageManager = new CDialogPackageManager();
	g_pDlgPackageManager->Create(IDD_DLG_PACKAGE_MANAGER,  GetDesktopWindow());
	g_pDlgPackageManager->ShowWindow(SW_SHOWNORMAL);
	return;

}


void CAdbToolDlg::OnBnClickedBtnWifi()
{
	if (!ADB.IsUsbConnected()) {
		UINT bRet = MessageBox(TEXT("Please connect device by USB first"), TEXT("Info"), 
						MB_ICONINFORMATION | MB_OKCANCEL);
		if (bRet != IDOK) {
			return;
		}
	}
	DestroyToolDialog(g_pDlgWifi);
	
	g_pDlgWifi = new CWifiDlg();
	g_pDlgWifi->Create(IDD_DLG_WIFI,  GetDesktopWindow());
	g_pDlgWifi->ShowWindow(SW_SHOWNORMAL);
	
}

BOOL g_bScreenOn = TRUE;

void ThreadScreenOnOff(LPVOID lP) 
{
	int nPowerDev = ADB.GetPowerDev();
	ADB.SendEvent(nPowerDev, EV_KEY, KEY_POWER, 1, CMD_JUST_RETURN);
	ADB.SendEvent(nPowerDev, EV_SYN, SYN_REPORT, 0, CMD_JUST_RETURN);
	if (g_bScreenOn) 
	{
		Sleep(3000);
	}
	else 
	{
		Sleep(100);
	}
	ADB.SendEvent(nPowerDev, EV_KEY, KEY_POWER, 0, CMD_JUST_RETURN);
	ADB.SendEvent(nPowerDev, EV_SYN, SYN_REPORT, 0, CMD_JUST_RETURN);
}


void CAdbToolDlg::OnBnClickedBtnScreenOff()
{
	
	g_bScreenOn = FALSE;
	_beginthread(ThreadScreenOnOff, 0, 0);
}


void CAdbToolDlg::OnBnClickedBtnScreenOn()
{
	
	g_bScreenOn = TRUE;
	_beginthread(ThreadScreenOnOff, 0, 0);
}


void CAdbToolDlg::OnDialogDebug()
{
	if ((g_pDlgDebug != NULL) && g_pDlgDebug->IsWindowVisible())
	{
		g_pDlgDebug->ShowWindow(SW_NORMAL);
	}
	else {
		g_pDlgDebug = new CDialogDebug();
		g_pDlgDebug->Create(IDD_DLG_DBG,  GetDesktopWindow());
		g_pDlgDebug->ShowWindow(SW_SHOWNORMAL);
	}
}

void CAdbToolDlg::OnBnClickedBtnInputLog()
{
	
	DestroyToolDialog(g_pDlgInputLog);
	g_pDlgInputLog = new CInputLogDlg();
	g_pDlgInputLog->Create(IDD_DLG_INPUT_LOG,  GetDesktopWindow());
	g_pDlgInputLog->ShowWindow(SW_SHOWNORMAL);
}



void ThreadReadFtDbg(LPVOID lP) 
{
	g_bDisableDeviceChange = TRUE;
	
	PARAM_T para;
	para.strCmd = TEXT("adb shell reboot");
	para.nType = CMD_INFINITE;


	TRACE(TEXT("> Reboot....\n"));
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	
	Sleep(5000);
	
	TRACE(TEXT("> Waiting for device\n"));
	while (!ADB.GetDevices()) 
	{
		Sleep(100);
	}


	TRACE(TEXT("> Open Interface..\n"));
	IPROC.OpenProc();


	while (!ADB.GetDevices()) 
	{
		Sleep(100);
	}

	para.strCmd = TEXT("adb shell su -c \"chmod 777 /data/ftdbg\"");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);


	para.strCmd = TEXT("adb shell su -c \"/data/ftdbg\"");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);


	int pos = para.strReturn.Find(TEXT('\n'));

	CString strFirstLine = para.strReturn.Mid(0, pos);
	TRACE(TEXT("FirstLine: "));
	TRACE(TEXT("%s"), strFirstLine);
	TRACE(TEXT("\n"));


	int value = _wtoi(strFirstLine);

	g_bDisableDeviceChange = FALSE;

}



void CAdbToolDlg::OnBnClickedChkLoadScreenshot()
{
	
	if (((CButton*)GetDlgItem(IDC_CHK_LOAD_SCREENSHOT))->GetCheck() == BST_CHECKED) {
		m_bLoadScreenShot = TRUE;
		UpdateScreen(0);
	}
	else {
		m_bLoadScreenShot = FALSE;
		Invalidate();
	}
}




void CAdbToolDlg::OnBnClickedBtnKillAdb()
{
	

	CAdbInterface::StopAllThread();
}


void CAdbToolDlg::OnBnClickedBtnSaveScreen()
{
	CString sFileName;
	sFileName.Format(TEXT("screen.png"));

	CFileDialog dlg(FALSE, TEXT("txt"), sFileName,
		OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		TEXT("photo(*.png)|*.png||"), this);  //OFN_HIDEREADONLY°ÑÖ»¶ÁÎÄ¼þÒþ²ØÆðÀ´

	dlg.m_ofn.lpstrTitle = _T("Save screenshot");

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString srcPath = m_strScreenPath;
	CString destPath = dlg.GetPathName();

	BOOL bRet = CopyFile(srcPath, destPath, FALSE);
	if (bRet) {
		MessageBox(TEXT("±£´æ³É¹¦"));
	} else {
		MessageBox(TEXT("±£´æÊ§°Ü"));
	}

}

unsigned WINAPI ThreadSendMultiSwipe(LPVOID lP) 
{
	CAdbToolDlg *pDlg = (CAdbToolDlg*)lP;
	pDlg->SendMultiSwipe();
	return 0;
}

void CAdbToolDlg::SendMultiSwipe()
{
	m_chkMultiSwipe.EnableWindow(FALSE);
	m_bDisableMouse = TRUE;
	SendSwipe();
	m_bSwipeFlag = FALSE;
	UpdateScreen(1000);
	m_bDisableMouse = FALSE;
	m_chkMultiSwipe.EnableWindow(TRUE);
}

void CAdbToolDlg::OnBnClickedCheckMultiSwipe()
{
	m_bMultiSwipeFlag = m_chkMultiSwipe.GetCheck() == BST_CHECKED;

	if (m_bMultiSwipeFlag) {
		ResetSwipe();
	} else {
		if (!m_vecMSwipeTpPoints.empty())
			_beginthreadex(NULL, 0, ThreadSendMultiSwipe, this, 0, NULL);
	}
}


void CAdbToolDlg::OnBnClickedBtnAdbConsole()
{
	PARAM_T para;
	para.strCmd = TEXT("adb version");
	para.nType = CMD_DIR_ADB;


	CAdbInterface::CreateCmdWindow(&para);
}


void CAdbToolDlg::OnBnClickedBtnFileManager()
{
	DestroyToolDialog(g_pDlgFileManager);
	g_pDlgFileManager = new CDlgFileManager();
	g_pDlgFileManager->Create(IDD_DLG_FILE_MANAGER, GetDesktopWindow());
	g_pDlgFileManager->ShowWindow(SW_SHOWNORMAL);
}


afx_msg LRESULT CAdbToolDlg::OnUmsgFileManagerClose(WPARAM wParam, LPARAM lParam)
{
	DestroyToolDialog(g_pDlgFileManager);
	return 0;
}


void CAdbToolDlg::OnBnClickedBtnProcessManager()
{
	DestroyToolDialog(g_pDlgProcessManager);
	g_pDlgProcessManager = new CDialogProcessManager();
	g_pDlgProcessManager->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());
	g_pDlgProcessManager->ShowWindow(SW_SHOWNORMAL);
}


void CAdbToolDlg::OnClose()
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	//CAdbInterface::StopAllSubThreads();
	CDialogEx::OnClose();
}


void CAdbToolDlg::OnPhoneReconnect()
{
	PostMessage(UWM_RECONNECT_DEVICE, 0, 0);
}

void CAdbToolDlg::OnScreenScreenon()
{
	g_bScreenOn = TRUE;
	_beginthread(ThreadScreenOnOff, 0, 0);
}


void CAdbToolDlg::OnScreenScreenoff()
{
	g_bScreenOn = FALSE;
	_beginthread(ThreadScreenOnOff, 0, 0);
}


void CAdbToolDlg::OnVolumnVolumn()
{
	ADB.SendKeyEvent(KEY_VOL_PLUS);
}


void CAdbToolDlg::OnVolumnVolumn32791()
{
	ADB.SendKeyEvent(KEY_VOL_MINUS);
}


void CAdbToolDlg::OnOperationsReboot()
{
	ADB.Reboot();
}


void CAdbToolDlg::OnOperationsInput()
{
	if ((g_pDlgAllKeyboard != NULL) && g_pDlgAllKeyboard->IsWindowVisible())
	{
		g_pDlgAllKeyboard->ShowWindow(SW_NORMAL);
	}
	else {
		g_pDlgAllKeyboard = new CAllKeyboardDlg();
		g_pDlgAllKeyboard->Create(IDD_DIALOG_ALL_KEYBOARD,  GetDesktopWindow());
		g_pDlgAllKeyboard->ShowWindow(SW_SHOWNORMAL);
	}
}


void CAdbToolDlg::OnPhoneExit()
{
	PostMessage(WM_CLOSE);
}


void CAdbToolDlg::OnManagersFilemanager()
{

	DestroyToolDialog(g_pDlgFileManager);
	g_pDlgFileManager = new CDlgFileManager();
	g_pDlgFileManager->Create(IDD_DLG_FILE_MANAGER, GetDesktopWindow());
	g_pDlgFileManager->ShowWindow(SW_SHOWNORMAL);
}


void CAdbToolDlg::OnManagersPackagemanager()
{
	DestroyToolDialog(g_pDlgPackageManager);
	g_pDlgPackageManager = new CDialogPackageManager();
	g_pDlgPackageManager->Create(IDD_DLG_PACKAGE_MANAGER,  GetDesktopWindow());
	g_pDlgPackageManager->ShowWindow(SW_SHOWNORMAL);
	return;
}


void CAdbToolDlg::OnManagersProcessmanager()
{

	DestroyToolDialog(g_pDlgProcessManager);
	g_pDlgProcessManager = new CDialogProcessManager();
	g_pDlgProcessManager->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());
	g_pDlgProcessManager->ShowWindow(SW_SHOWNORMAL);
}


void CAdbToolDlg::OnToolsInputlog()
{

	DestroyToolDialog(g_pDlgInputLog);
	g_pDlgInputLog = new CInputLogDlg();
	g_pDlgInputLog->Create(IDD_DLG_INPUT_LOG,  GetDesktopWindow());
	g_pDlgInputLog->ShowWindow(SW_SHOWNORMAL);
}


void CAdbToolDlg::OnToolsAdbconsole()
{
	PARAM_T para;
	para.strCmd = TEXT("adb version");
	para.nType = CMD_DIR_ADB;


	CAdbInterface::CreateCmdWindow(&para);
}


void CAdbToolDlg::OnAboutAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


void CAdbToolDlg::OnOperationsKill()
{
	CAdbInterface::StopAllThread();
}


void CAdbToolDlg::OnToolsDebuglogger()
{
	OnDialogDebug();
}


void CAdbToolDlg::OnPhonePhoneinfo()
{
	DestroyToolDialog(g_pDlgPhoneInfo);
	g_pDlgPhoneInfo = new CDialogPhoneInfo();
	g_pDlgPhoneInfo->Create(IDD_DIALOG_PHONE_INFO,  GetDesktopWindow());
	g_pDlgPhoneInfo->ShowWindow(SW_SHOWNORMAL);
}
