#include "StdAfx.h"
#include <TlHelp32.h>
#include "AdbInterface.h"
#include <fstream>

extern HWND g_hwndDebug;

vector<DWORD> CAdbInterface::s_vecAdbProcessIds;
BOOL CAdbInterface::m_bUseToolAdb = FALSE;

map<CString, ANDROID_OS_INFO> CAdbInterface::s_mapAndroidOsInfos;
CString CAdbInterface::CMD_ECHO_FILE = TEXT("adb\\result.echo");

ANDROID_OS_INFO::ANDROID_OS_INFO(CString release)
{
	map<CString, ANDROID_OS_INFO>::const_iterator cit = CAdbInterface::s_mapAndroidOsInfos.find(release);
	if (cit != CAdbInterface::s_mapAndroidOsInfos.end()) {
		*this = cit->second;
		return;
	}
	cit = CAdbInterface::s_mapAndroidOsInfos.begin();
	map<CString, ANDROID_OS_INFO>::const_iterator next = cit;
	for (next++; next != CAdbInterface::s_mapAndroidOsInfos.end(); next++)
	{
		/* cit < release < next */
		if ((next->first.CompareNoCase(release) > 0)) {
			if (cit->first.CompareNoCase(release) < 0) {
				break;
			}
		}
		cit = next;
	}
	*this = cit->second;
}


CAdbInterface::CAdbInterface(void) 
	: m_bDevConnected(FALSE), 
	m_nFtInputDevNo(1), 
	m_nPowerDevice(0)
{
	m_curDevice.Reset();
}

void CAdbInterface::StaticInit()
{
	s_mapAndroidOsInfos[TEXT("8.1")] = ANDROID_OS_INFO(TEXT("8.1"), ANDROID_O_MR1, TEXT("O_MR1"));
	s_mapAndroidOsInfos[TEXT("8.0")] = ANDROID_OS_INFO(TEXT("8.0"), ANDROID_O, TEXT("O"));
	s_mapAndroidOsInfos[TEXT("7.1")] = ANDROID_OS_INFO(TEXT("7.1"), ANDROID_N_MR1, TEXT("N_MR1"));
	s_mapAndroidOsInfos[TEXT("7.1.1")] = ANDROID_OS_INFO(TEXT("7.1.1"), ANDROID_N_MR1, TEXT("N_MR1"));
	s_mapAndroidOsInfos[TEXT("7.0")] = ANDROID_OS_INFO(TEXT("7.0"), ANDROID_N, TEXT("N"));
	s_mapAndroidOsInfos[TEXT("6.0")] = ANDROID_OS_INFO(TEXT("6.0"), ANDROID_M, TEXT("M"));
	s_mapAndroidOsInfos[TEXT("5.1")] = ANDROID_OS_INFO(TEXT("5.1"), ANDROID_LOLLIPOP_MR1, TEXT("LOLLIPOP_MR1"));
	s_mapAndroidOsInfos[TEXT("5.0")] = ANDROID_OS_INFO(TEXT("5.0"), ANDROID_LOLLIPOP, TEXT("LOLLIPOP"));
	s_mapAndroidOsInfos[TEXT("4.4W")] = ANDROID_OS_INFO(TEXT("4.4W"), ANDROID_KITKAT_WATCH, TEXT("KITKAT_WATCH"));
	s_mapAndroidOsInfos[TEXT("4.4")] = ANDROID_OS_INFO(TEXT("4.4"), ANDROID_KITKAT, TEXT("KITKAT"));
	s_mapAndroidOsInfos[TEXT("4.3")] = ANDROID_OS_INFO(TEXT("4.3"), ANDROID_JELLY_BEAN_MR2, TEXT("JELLY_BEAN_MR2"));
	s_mapAndroidOsInfos[TEXT("4.2.2")] = ANDROID_OS_INFO(TEXT("4.2.2"), ANDROID_JELLY_BEAN_MR1, TEXT("JELLY_BEAN_MR1"));
	s_mapAndroidOsInfos[TEXT("4.2")] = ANDROID_OS_INFO(TEXT("4.2"), ANDROID_JELLY_BEAN_MR1, TEXT("JELLY_BEAN_MR1"));
	s_mapAndroidOsInfos[TEXT("4.1.1")] = ANDROID_OS_INFO(TEXT("4.1.1"), ANDROID_JELLY_BEAN, TEXT("JELLY_BEAN"));
	s_mapAndroidOsInfos[TEXT("4.1")] = ANDROID_OS_INFO(TEXT("4.1"), ANDROID_JELLY_BEAN, TEXT("JELLY_BEAN"));
	s_mapAndroidOsInfos[TEXT("4.0.4")] = ANDROID_OS_INFO(TEXT("4.0.4"), ANDROID_ICE_CREAM_SANDWICH_MR1, TEXT("ICE_CREAM_SANDWICH_MR1"));
	s_mapAndroidOsInfos[TEXT("4.0.3")] = ANDROID_OS_INFO(TEXT("4.0.3"), ANDROID_ICE_CREAM_SANDWICH_MR1, TEXT("ICE_CREAM_SANDWICH_MR1"));
	s_mapAndroidOsInfos[TEXT("4.0.1")] = ANDROID_OS_INFO(TEXT("4.0.1"), ANDROID_ICE_CREAM_SANDWICH, TEXT("ICE_CREAM_SANDWICH"));
	s_mapAndroidOsInfos[TEXT("4.0.2")] = ANDROID_OS_INFO(TEXT("4.0.2"), ANDROID_ICE_CREAM_SANDWICH, TEXT("ICE_CREAM_SANDWICH"));
	s_mapAndroidOsInfos[TEXT("4.0")] = ANDROID_OS_INFO(TEXT("4.0"), ANDROID_ICE_CREAM_SANDWICH, TEXT("ICE_CREAM_SANDWICH"));
	s_mapAndroidOsInfos[TEXT("3.2")] = ANDROID_OS_INFO(TEXT("3.2"), ANDROID_HONEYCOMB_MR2, TEXT("HONEYCOMB_MR2"));
	s_mapAndroidOsInfos[TEXT("3.1.x")] = ANDROID_OS_INFO(TEXT("3.1.x"), ANDROID_HONEYCOMB_MR1, TEXT("HONEYCOMB_MR1"));
	s_mapAndroidOsInfos[TEXT("3.0.x")] = ANDROID_OS_INFO(TEXT("3.0.x"), ANDROID_HONEYCOMB, TEXT("HONEYCOMB"));
	s_mapAndroidOsInfos[TEXT("2.3.4")] = ANDROID_OS_INFO(TEXT("2.3.4"), ANDROID_GINGERBREAD_MR1, TEXT("GINGERBREAD_MR1"));
	s_mapAndroidOsInfos[TEXT("2.3.3")] = ANDROID_OS_INFO(TEXT("2.3.3"), ANDROID_GINGERBREAD_MR1, TEXT("GINGERBREAD_MR1"));
	s_mapAndroidOsInfos[TEXT("2.3.2")] = ANDROID_OS_INFO(TEXT("2.3.2"), ANDROID_GINGERBREAD, TEXT("GINGERBREAD"));
	s_mapAndroidOsInfos[TEXT("2.3.1")] = ANDROID_OS_INFO(TEXT("2.3.1"), ANDROID_GINGERBREAD, TEXT("GINGERBREAD"));
	s_mapAndroidOsInfos[TEXT("2.3")] = ANDROID_OS_INFO(TEXT("2.3"), ANDROID_GINGERBREAD, TEXT("GINGERBREAD"));
	s_mapAndroidOsInfos[TEXT("2.2.x")] = ANDROID_OS_INFO(TEXT("2.2.x"), ANDROID_FROYO, TEXT("FROYO"));
	s_mapAndroidOsInfos[TEXT("2.1.x")] = ANDROID_OS_INFO(TEXT("2.1.x"), ANDROID_ECLAIR_MR1, TEXT("ECLAIR_MR1"));
	s_mapAndroidOsInfos[TEXT("2.0.1")] = ANDROID_OS_INFO(TEXT("2.0.1"), ANDROID_ECLAIR_0_1, TEXT("ECLAIR_0_1"));
	s_mapAndroidOsInfos[TEXT("2.0")] = ANDROID_OS_INFO(TEXT("2.0"), ANDROID_ECLAIR, TEXT("ECLAIR"));
	s_mapAndroidOsInfos[TEXT("1.6")] = ANDROID_OS_INFO(TEXT("1.6"), ANDROID_DONUT, TEXT("DONUT"));
	s_mapAndroidOsInfos[TEXT("1.5")] = ANDROID_OS_INFO(TEXT("1.5"), ANDROID_CUPCAKE, TEXT("CUPCAKE"));
	s_mapAndroidOsInfos[TEXT("1.1")] = ANDROID_OS_INFO(TEXT("1.1"), ANDROID_BASE_1_1, TEXT("BASE_1_1"));
	s_mapAndroidOsInfos[TEXT("1.0")] = ANDROID_OS_INFO(TEXT("1.0"), ANDROID_BASE, TEXT("BASE"));
}

CAdbInterface::~CAdbInterface(void)
{
}

BOOL CAdbInterface::SendKeyEvent(int nEvent)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell input keyevent %d"), nEvent);
	para.nType = CMD_JUST_RETURN;

	CAdbInterface::CreateAdbProcess(&para);
	return TRUE;
}
/* adb shell input keyevent KEY_POWER */
BOOL CAdbInterface::SendEvent(int dev, int type, int code, int value, int cmdType)
{
	if (!IsConnected())
	{
		return FALSE;
	}
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell sendevent /dev/input/event%d %d %d %d"), dev, type, code, value);
	para.nType = cmdType;

	CAdbInterface::CreateAdbProcess(&para);
	return TRUE;
}

void CAdbInterface::Init()
{
	PARAM_T para;
	para.strCmd = TEXT("adb version");
	para.nType = CMD_INFINITE;

	ClearTmps();
	CAdbInterface::CreateAdbProcess(&para);

	if (para.strReturn.Find(TEXT("Android Debug Bridge")) == -1)
	{
		m_bUseToolAdb = TRUE;
	}

	para.strCmd = TEXT("adb start-server");
	para.nType = CMD_INFINITE;

	CAdbInterface::CreateAdbProcess(&para);
}

void CAdbInterface::Deinit()
{
	PARAM_T para;
	para.strCmd = TEXT("adb kill-server");
	para.nType = CMD_INFINITE;

	CAdbInterface::CreateAdbProcess(&para);

}

void CAdbInterface::Reset()
{
	m_curDevice.Reset();
	m_bDevConnected = FALSE;
}

void CAdbInterface::Reboot()
{
	PARAM_T para;
	para.strCmd = TEXT("adb reboot");
	para.nType = CMD_JUST_RETURN;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (para.bRet)
	{
		Reset();
	}

}


void CAdbInterface::GetAdbProcessInfo()
{
	GetProcessIDs(TEXT("adb.exe"));
	GetProcessIDs(TEXT("360MobileMgr.exe"));
	GetProcessIDs(TEXT("360MobileLink.exe"));
}

void CAdbInterface::GetProcessIDs(CString strExe)
{
	CString processName = strExe;
	//创建进程快照(TH32CS_SNAPPROCESS表示创建所有进程的快照)
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	//PROCESSENTRY32进程快照的结构体       
	PROCESSENTRY32 pe;          
	//实例化后使用Process32First获取第一个快照的进程前必做的初始化操作
	pe.dwSize = sizeof(PROCESSENTRY32); 
	//下面的IF效果同:       
	//if(hProcessSnap == INVALID_HANDLE_VALUE)   无效的句柄  
	if(!Process32First(hSnapShot,&pe))  
	{          
		return;    
	}          
	processName.MakeLower();  
	BOOL clearprocess = FALSE;
	//如果句柄有效  则一直获取下一个句柄循环下去 
	while (Process32Next(hSnapShot,&pe))  
	{               
		//pe.szExeFile获取当前进程的可执行文件名称 
		CString scTmp = pe.szExeFile;    
		scTmp.MakeLower();             
		char modPath[MAX_PATH] = {0};
		if(!scTmp.Compare(processName))    
		{                   
			//从快照进程中获取该进程的PID(即任务管理器中的PID)
			DWORD dwProcessID = pe.th32ProcessID;   

			s_vecAdbProcessIds.push_back(dwProcessID);
		} 
	}      
	::CloseHandle(hSnapShot);
}

set<HANDLE> CAdbInterface::s_setAllSubThreads;
CCriticalSection g_csSubThread;
void CAdbInterface::AddSubThread(HANDLE hdl)
{
	g_csSubThread.Lock();
	TRACE("Add Thread: %d\n", GetThreadId(hdl));
	s_setAllSubThreads.insert(hdl);
	g_csSubThread.Unlock();
}
void CAdbInterface::RemoveSubThread(HANDLE hdl)
{
	g_csSubThread.Lock();
	TRACE("Remove Thread: %d\n", GetThreadId(hdl));
	s_setAllSubThreads.erase(hdl);
	g_csSubThread.Unlock();
}
void CAdbInterface::StopAllSubThreads()
{
	g_csSubThread.Lock();
	set<HANDLE>::const_iterator cit = s_setAllSubThreads.begin();
	for (; cit != s_setAllSubThreads.end(); cit++)
	{
		TRACE("Terminal Thread: %d\n", GetThreadId(*cit));
		WaitForSingleObject(*cit, 10000);
		::TerminateThread(*cit, -1);

	}
	s_setAllSubThreads.clear();
	g_csSubThread.Unlock();
}

void CAdbInterface::StopAllThread()
{
	GetAdbProcessInfo();
	vector<DWORD>::const_iterator cit = s_vecAdbProcessIds.begin();

	for (; cit != s_vecAdbProcessIds.end(); cit++) 
	{
		HANDLE hdl = OpenProcess(PROCESS_TERMINATE, FALSE, *cit); 
		if (hdl != NULL) 
		{
			BOOL bRet = TerminateProcess(hdl, 0);
			TRACE(TEXT("kill: %d, %s\n"), (int)hdl, bRet ? TEXT("Y") : TEXT("N"));
			CloseHandle(hdl);
		}
	}
	s_vecAdbProcessIds.clear();
}

int CAdbInterface::GetPrintkLevel()
{
	PARAM_T para;

	para.strCmd.Format(TEXT("adb shell \"cat /proc/sys/kernel/printk\""));
	para.nType = CMD_WAIT | CMD_READ_BYTES_COUNT;
	para.nWaitMs = 1000;
	para.nReadBytesCount = 256;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	CString consoleLevel;
	consoleLevel.AppendChar(para.strReturn.GetAt(0));

	int level = _wtoi(consoleLevel.GetString());

	return level;
}

BOOL ADB_DEVICE::IsValidDevice() const
{
	return state.Find(TEXT("device")) == 0;
}

BOOL CAdbInterface::SetPrintkLevel(int level)
{
	PARAM_T para;

	para.strCmd.Format(TEXT("adb shell \"echo %d > /proc/sys/kernel/printk\""), level);
	para.nType = CMD_WAIT | CMD_READ_BYTES_COUNT;
	para.nWaitMs = 1000;
	para.nReadBytesCount = 256;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	return level == GetPrintkLevel();
}

BOOL CAdbInterface::GetResolution(CPoint &resolution)
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell dumpsys window displays");
	para.nType = CMD_WAIT | CMD_READ_BYTES_COUNT;
	para.nWaitMs = 3000;
	para.nReadBytesCount = 256;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	int nPos = para.strReturn.Find(TEXT("cur="));
	if (nPos != -1)
	{
		nPos += 4;
		int nEndPos = para.strReturn.Find(TEXT(" "), nPos);
		if (nEndPos != -1)
		{
			CString strResolution = para.strReturn.Mid(nPos, nEndPos - nPos);
			nPos = strResolution.Find(TEXT("x"));
			if (nPos != -1)
			{
				resolution.x = _wtoi(strResolution.Mid(0, nPos));
				resolution.y = _wtoi(strResolution.Mid(nPos+1));
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CAdbInterface::SendSwipe(CPoint pntBegin, CPoint pntEnd)
{

	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell input swipe %d %d %d %d"), pntBegin.x, pntBegin.y,
		pntEnd.x, pntEnd.y);
	para.nType = CMD_JUST_RETURN;


	para.bRet = CAdbInterface::CreateAdbProcess(&para);
}

void CAdbInterface::SendClick(CPoint point)
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell input tap %d %d"), point.x, point.y);
	para.nType = CMD_JUST_RETURN;


	para.bRet = CAdbInterface::CreateAdbProcess(&para);

}

BOOL CAdbInterface::InstallApk(CString strApkPath) 
{
	PARAM_T para;
	para.strCmd = TEXT("adb install -r \"");
	para.strCmd += strApkPath;
	para.strCmd += TEXT("\"");

	para.nType = CMD_ABSOLUTE_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	para.strReturn.MakeLower();
	if (para.strReturn.Find(TEXT("fail")) == -1) 
	{
		return TRUE;
	}
	else 
	{
		m_strError = para.strReturn;
		return FALSE;
	}
}

BOOL CAdbInterface::StartActivity(CString strActivity)
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell am start -n ");
	para.strCmd += strActivity;
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);



	if ( (para.strReturn.Find(TEXT("not exist")) != -1) || 
		 (para.strReturn.Find(TEXT("Exception")) != -1) ||
		 (para.strReturn.Find(TEXT("Error")) != -1))
	{
		return FALSE;
	}
	
	return TRUE;
}

void CAdbInterface::SendText(CString strText)
{

	PARAM_T para;
	para.strCmd = TEXT("adb shell input text ");
	para.strCmd += strText;
	para.nType = CMD_JUST_RETURN;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

}

BOOL CAdbInterface::ToRoot()
{
	PARAM_T para;
	para.strCmd = TEXT("adb root");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	/* cannot root in production builds */
	if (para.strReturn.Find(TEXT("production builds")) != -1) {
		return FALSE;
	}
	/* adbd is already running as root */
	else if (para.strReturn.Find(TEXT("running as root")) != -1) {
		return TRUE;
	}
	/* restart adbd as root*/
	else if (para.strReturn.Find(TEXT("adbd as root")) != -1) {
		return TRUE;
	}

	return FALSE;
}

BOOL CAdbInterface::SelectDevice(int i)
{
	m_curDevice = m_vecAdbDevices[i];
	return SelectDevice(m_curDevice);
}

BOOL CAdbInterface::SelectDevice(ADB_DEVICE &dev)
{
	m_curDevice = dev;

	CString test = GetSNo();

	if (test.Find(TEXT("unknown")) != -1) {
		m_curDevice.Reset();
		m_bDevConnected = FALSE;
		return FALSE;
	}

	m_bDevConnected = TRUE;
	m_bDevRooted = ToRoot();
	m_bWifiConnected = FALSE;

	if ( (m_curDevice.serial_no.Find(TEXT(".")) != -1) &&
		(m_curDevice.serial_no.Find(TEXT(":")) != -1) )
	{
		m_bWifiConnected = TRUE;
	}
	GetInputDevices();
	GetOsInfo();
	return TRUE;
}

BOOL CAdbInterface::GetDevices()
{
	m_curDevice.Reset();
	m_vecAdbDevices.clear();

	PARAM_T para;
	para.strCmd = TEXT("adb devices -l");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	// 可能是360手机助手, 金山手机助手占用ADB端口
	/* protocol fault (couldn't read status length) 
	 * protocol fault (couldn't read status length)
	 * protocol fault (couldn't read status)
	 */

	if (para.strReturn.Find(TEXT("protocol fault")) != -1) {
		return FALSE;
	}
	/* 
	List of devices attached
	01225f833fb44843       device product:bullhead model:Nexus_5X device:bullhead
	234abd2e9804           device product:tiffany model:MI_5X device:tiffany
	 */
	int nPos = para.strReturn.Find(TEXT("attached"));

	CString strDevices = para.strReturn.Mid(nPos + 8);
	int len = strDevices.GetLength();

	WCHAR single;
	CString strSingle;
	int type = GD_SERIALNO;
	ADB_DEVICE adbdev;

	for (int i = 0; i < len; i++) 
	{
		single = strDevices.GetAt(i);
		if (!IsWhitespace(single)) 
		{
			strSingle.AppendChar(single);
		}

		if (IsWhitespace(single)) {
			if (!strSingle.IsEmpty())
			{
				
				switch (type) {
				case GD_SERIALNO:
					if (!adbdev.serial_no.IsEmpty()) {
						m_vecAdbDevices.push_back(adbdev);
						adbdev.Reset();
					}
					adbdev.serial_no = strSingle;
					break;

				case GD_CONN_STATE:
					adbdev.state = strSingle;
					break;

				case GD_PRODUCT:
					nPos = strSingle.Find(TEXT("product:"));
					adbdev.product = strSingle.Mid(nPos+8);
					break;

				case GD_MODEL:
					nPos = strSingle.Find(TEXT("model:"));
					adbdev.model = strSingle.Mid(nPos+6);
					break;

				case GD_DEVICE:
					nPos = strSingle.Find(TEXT("device:"));
					adbdev.device = strSingle.Mid(nPos+7);

					m_vecAdbDevices.push_back(adbdev);
					adbdev.Reset();
					break;
				default:
					break;

				}
				type++;
				if (type > GD_DEVICE) {
					type = GD_SERIALNO;
				}
				strSingle.Empty();
			}
		}
	}
	if (!adbdev.serial_no.IsEmpty()) {
		m_vecAdbDevices.push_back(adbdev);
		adbdev.Reset();
	}


	if (m_vecAdbDevices.size() > 0)
	{
		if (m_vecAdbDevices.size() == 1) 
		{
			return SelectDevice(0);
		}
	}
	else
	{
		m_bDevConnected = FALSE;
	}
	return m_bDevConnected;
}

void CAdbInterface::GetOsInfo()
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell getprop ro.build.version.release");
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	
	CString release = CleanString(para.strReturn);
	m_osAndroid = ANDROID_OS_INFO(release);
}

CString CAdbInterface::GetSNo()
{
	PARAM_T para;
	para.strCmd = TEXT("adb get-serialno");
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	return CleanString(para.strReturn);
}

BOOL CAdbInterface::GetScreenShot(CString strDstPath)
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell /system/bin/screencap -p /sdcard/screen.png");
	para.nType = CMD_ABSOLUTE_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	para.strCmd = TEXT("adb pull /sdcard/screen.png \"");
	para.strCmd += strDstPath;
	para.strCmd += TEXT("\"");

	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	if (para.strReturn.Find(TEXT("bytes in")))
	{
		return TRUE;
	}

	return FALSE;
}

void CAdbInterface::AnalyseInputDevices(CString strDevices)
{
	int nBegPos = strDevices.Find(TEXT("add device")); 
	int nEndPos;
	int nPos;
	CString strSingle;
	BOOL bPowerKey = FALSE;

	while (nBegPos != -1)
	{
		vector<CString> vecLines;
		nEndPos = strDevices.Find(TEXT("add device"), nBegPos+10);
		if (nEndPos != -1)
		{
			strSingle = strDevices.Mid(nBegPos, nEndPos);
		}
		else
		{
			strSingle = strDevices.Mid(nBegPos);
		}

		int nSize = strSingle.GetLength();
		for (int i = 0; i < nSize; i++)
		{
			nPos = strSingle.Find(TEXT('\n'), i);
			if (nPos == -1) 
			{
				CString strAttr = strSingle.Mid(i);
				vecLines.push_back(strAttr);
				break;
			}
			else if (nPos < nSize)
			{
				CString strAttr = strSingle.Mid(i, nPos-i);
				vecLines.push_back(strAttr);
				i = nPos;
			}
			else 
			{
				break;
			}
		}


		INPUT_EVENT_DEV_T ied;
		ied.vecAttrs.assign(vecLines.begin(), vecLines.end());

		vector<CString>::const_iterator cit = vecLines.begin();
		for ( ; cit != vecLines.end(); cit++)
		{
			if (cit->Find(TEXT("add device")) != -1)
			{
				nPos = cit->Find(TEXT("/dev/input/event"));
				TRACE(cit->Mid(nPos+16));
				TRACE(TEXT("\n"));
				ied.no = _wtoi(cit->Mid(nPos+16));
			}
			else if (cit->Find(TEXT("name:")) != -1)
			{
				int nRightQuote;
				nPos = cit->Find(TEXT('\"'));
				nRightQuote = cit->Find(TEXT('\"'), nPos+1);
				ied.strName = cit->Mid(nPos+1, nRightQuote-nPos-1);
			}

			if (cit->Find(TEXT("0074")) != -1) 
			{
				if (!bPowerKey)
				{
					bPowerKey = TRUE;
					m_nPowerDevice = ied.no;
				}
			}
		}
		m_vecInputDevices.push_back(ied);

		if (nEndPos == -1)
		{
			break;
		}
		strDevices = strDevices.Mid(nEndPos);
		nBegPos = strDevices.Find(TEXT("add device"));
	}
}

void CAdbInterface::GetInputDevices()
{
	PARAM_T para;
	para.strCmd = TEXT("adb shell getevent -p");
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	m_vecInputDevices.clear();
	
	AnalyseInputDevices(para.strReturn);

	vector<INPUT_EVENT_DEV_T>::const_iterator cit = m_vecInputDevices.begin();
	for (; cit != m_vecInputDevices.end(); cit++)
	{
		//if ( (cit->strName.Find(TEXT("ts")) != -1) 
		//	&& (cit->strName.Find(TEXT("ft")) != -1) )
		//{
		//	m_nFtInputDevNo = cit->no;
		//	break;
		//}
		CString allattrs;
		vector<CString>::const_iterator attrIt = cit->vecAttrs.begin();
		for (; attrIt != cit->vecAttrs.end(); attrIt++) {
			allattrs += *attrIt;
		}
		if ((allattrs.Find(TEXT("0035")) != -1) &&
			(allattrs.Find(TEXT("0036")) != -1) &&
			(allattrs.Find(TEXT("ABS")) != -1))
		{
			m_nFtInputDevNo = cit->no;
			break;
		}
	}
}

BOOL CAdbInterface::NeedAddSNo() const 
{
	//if (m_vecAdbDevices.size() <= 1) {
	//	return FALSE;
	//}
	return !m_curDevice.serial_no.IsEmpty();
}

CString CAdbInterface::AddCmdPrefix(CString strCmd)
{
	CString strRetCmd = TEXT("/k \"adb\\") ;

	CString strNewCmd = strCmd;
	if (ADB.NeedAddSNo())
	{
		int nPos = strCmd.Find(TEXT("adb "));
		strNewCmd.Format(TEXT("adb -s %s "), ADB.GetSerialNo());
		strNewCmd += strCmd.Mid(nPos+4);
	}
	
	strRetCmd += strNewCmd;
	strRetCmd += TEXT("\"");

	return strRetCmd;
}


CString CAdbInterface::AddAdbPrefix(CString strCmd)
{
	CString strRetCmd = TEXT("cmd.exe /c \"");

	
	if (m_bUseToolAdb)
	{
		strRetCmd.Append(TEXT("adb\\")) ;
	}

	CString strNewCmd = strCmd;
	if (ADB.NeedAddSNo())
	{
		int nPos = strCmd.Find(TEXT("adb "));
		strNewCmd.Format(TEXT("adb -s %s "), ADB.GetSerialNo());
		strNewCmd += strCmd.Mid(nPos+4);
	}

	strRetCmd += strNewCmd;
	strRetCmd += TEXT("\"");

	return strRetCmd;
}

CString CAdbInterface::CleanString(CString str)
{
	if (str.GetLength() < 2)
	{
		return str;
	}
	int length = str.GetLength();
	TCHAR ch = 0;
	int sIndex = 0;
	int eIndex = length-1;

	for (int i = 0; i < length; i++)
	{
		ch = str.GetAt(i);
		if ((ch != TEXT('\r')) &&
			(ch != TEXT('\n')) &&
			(ch != TEXT(' ')) &&
			(ch != TEXT('\t')) &&
			(ch != TEXT('')))
		{
			sIndex = i;
			break;
		}
	}

	for (int i = length-1; i >= 0; i--)
	{
		ch = str.GetAt(i);
		if ((ch != TEXT('\r')) &&
			(ch != TEXT('\n')) &&
			(ch != TEXT(' ')) &&
			(ch != TEXT('\t')) &&
			(ch != TEXT('')))
		{
			eIndex = i;
			break;
		}
	}

	if (eIndex <= sIndex)
	{
		return TEXT("XX");
	}

	str = str.Mid(sIndex, eIndex-sIndex+1);


	return str;
}

BOOL CAdbInterface::CreateCmdWindow(P_PARAM_T para)
{
	TRACE(TEXT("%s"), para->strCmd);
	TRACE(TEXT("\n"));

	if (para->nType == CMD_DIR_ADB) 
	{
		if (m_bUseToolAdb) 
		{
			ShellExecute(NULL, TEXT("open"), TEXT("cmd.exe"), para->strCmd, TEXT("adb"), SW_SHOW);
		} else {
			ShellExecute(NULL, TEXT("open"), TEXT("cmd.exe"), NULL, NULL, SW_SHOW);
		}
	}
	else 
	{
		para->strCmd = AddCmdPrefix(para->strCmd);
		ShellExecute(NULL, TEXT("open"), TEXT("cmd.exe"), para->strCmd, NULL, SW_SHOW);
	}

	return TRUE;
}

#define SendDebugMessage(dbginfo) do {\
	if (IsWindowVisible(g_hwndDebug)) \
	{\
		::PostMessage(g_hwndDebug, MSG_APPEND_ADB_CMD, \
			(WPARAM)(new CString(dbginfo)), 0);\
	}\
} while (0)

void CAdbInterface::ClearTmps()
{
	CFileFind finder;  
	CString strPath;
	BOOL bWorking = finder.FindFile(TEXT("adb\\*.*"));  
	while (bWorking)  
	{  
		bWorking = finder.FindNextFile();  
		strPath = finder.GetFilePath();  
		
		if (strPath.Find(CAdbInterface::CMD_ECHO_FILE) != -1) {
			DeleteFile(strPath);
		}

	}  
	finder.Close();  
}

BOOL CAdbInterface::CreateAdbProcess(P_PARAM_T para)
{
	DWORD begin = ::GetCurrentTime();
	BOOL bRet = _CreateAdbProcess(para);
	DWORD end = ::GetCurrentTime();

	//CTimeSpan span = end - begin;
	TRACE("Cmd Elaps: %dms\n", /*span.GetTimeSpan()*/end-begin);

	return bRet;
}
CCriticalSection g_CS;
#define CMD_ECHO_BUFFER_SIZE (CMD_READ_BUFFER_SIZE * 10)
wchar_t g_wszReadBuffer[CMD_ECHO_BUFFER_SIZE];
BOOL CAdbInterface::GetFileContents(CString path, CString &contents)
{
	g_CS.Lock();
	std::wifstream wifs;
	wifs.open(path);
	if (!wifs.is_open()) {
		TRACE(TEXT("read contents from %s failed!"), path);
		g_CS.Unlock();
		return FALSE;
	}

	CString line;
	while (!wifs.eof()) {
		memset(g_wszReadBuffer, 0, sizeof(wchar_t) * CMD_ECHO_BUFFER_SIZE);
		wifs.getline(g_wszReadBuffer, CMD_ECHO_BUFFER_SIZE);
		line = g_wszReadBuffer;
		contents += line;
		contents += TEXT("\n");
	}
	wifs.close();
	g_CS.Unlock();
	return TRUE;
}

BOOL CAdbInterface::_CreateAdbProcess(P_PARAM_T para)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	para->strReturn = TEXT("");
	if (!CreatePipe(&hRead,&hWrite,&sa,0)) {
		para->strReturn = TEXT("创建管道失败!");
		return FALSE;
	}

	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // 使用默认的IO句柄

	si.hStdOutput = hWrite;
	si.hStdError = hWrite;

	PROCESS_INFORMATION pi = {0};  
	si.wShowWindow = SW_HIDE;	

	SendDebugMessage(para->strCmd);
	CString strEchoFile;
	para->strCmd = AddAdbPrefix(para->strCmd);
	if (para->nType & CMD_READ_BACK_FILE) {
		strEchoFile.Format(TEXT("%s%d"), CMD_ECHO_FILE, CTime::GetCurrentTime());
		para->strCmd.AppendFormat(TEXT(" > %s"), strEchoFile); 

		//si.dwXCountChars = 1024;
		//si.dwYCountChars = 768;
		//si.dwXSize = 1440;
		//si.dwYSize = 1080;
		//si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW | STARTF_USESIZE;
	}
	TRACE(TEXT("%s"), para->strCmd);
	TRACE(TEXT("\n"));

	if (!CreateProcess(NULL, para->strCmd.GetBuffer(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))  
	{  
		para->strReturn = TEXT("创建进程失败!");
		return FALSE;
	}

	s_vecAdbProcessIds.push_back(pi.dwProcessId);
	// 线程不再被访问,关闭句柄,不影响句柄运行
	CloseHandle(pi.hThread);	


	if (para->nType & CMD_ABSOLUTE_INFINITE)
	{
		WaitForSingleObject(pi.hProcess,INFINITE);
	}
	else if (para->nType & CMD_JUST_RETURN)
	{
		CloseHandle(hRead);
		CloseHandle(hWrite);
		CloseHandle(pi.hProcess);

		char buffer[257] = {0};
		DWORD dwBytesReadCount = 0;
		DWORD dwBytesRead;
		while (ReadFile(hRead, buffer, 256, &dwBytesRead, NULL))
		{
		}
		return TRUE;
	}
	else if (para->nType & CMD_INFINITE) {
		WaitForSingleObject(pi.hProcess, 15000);
	}
	else 
	{
		WaitForSingleObject(pi.hProcess, para->nWaitMs);
	}

	DWORD dwExitCode;
	// 线程结束,获取返回码
	GetExitCodeProcess(pi.hProcess,
		&dwExitCode);

	// 关闭线程句柄
	CloseHandle(pi.hProcess);
	CloseHandle(hWrite);

	if (para->nType & CMD_READ_BACK_FILE) 
	{
		GetFileContents(strEchoFile, para->strReturn);
		DeleteFileW(strEchoFile);
		TRACE(TEXT("%s"), para->strReturn);
		TRACE(TEXT("\n"));
		SendDebugMessage(para->strReturn);
		return TRUE;
	}

	//char *buffer = new char[CMD_READ_BUFFER_SIZE];
	//memset(buffer, 0, CMD_READ_BUFFER_SIZE);
	char buffer[CMD_READ_BUFFER_SIZE+1] = {0};

	DWORD dwBytesReadCount = 0;
	DWORD dwBytesRead;
	while (ReadFile(hRead, buffer, CMD_READ_BUFFER_SIZE, &dwBytesRead, NULL))
	{
	
		para->strReturn += buffer;
		memset(buffer, 0, CMD_READ_BUFFER_SIZE);

		if (para->nType & CMD_READ_BYTES_COUNT) 
		{
			dwBytesReadCount += dwBytesRead;

			if (dwBytesReadCount >= para->nReadBytesCount) {
				break;
			}
		}
	}
	//delete buffer;
	/* TODO:* daemon not running. starting it now at tcp:5037 *
	* daemon started successfully * */

	CloseHandle(hRead);
	TRACE(TEXT("%s"), para->strReturn);
	TRACE(TEXT("\n"));

	SendDebugMessage(para->strReturn);

	return TRUE;
}




CAdbInterface ADB;