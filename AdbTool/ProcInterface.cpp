#include "StdAfx.h"
#include "ProcInterface.h"
#include "AdbInterface.h"

CString CProcInterface::CMD_PATH;
CString CProcInterface::ECHO_PATH;
CString CProcInterface::APK_PATH;

void CProcInterface::Init()
{
	wchar_t buffer[MAX_PATH] = {0};
	::GetTempPath(MAX_PATH, buffer);

	CMD_PATH = buffer; 
	CMD_PATH += "cmd";

	ECHO_PATH = buffer;
	ECHO_PATH += "echo";


	memset(buffer, 0, sizeof(wchar_t) * MAX_PATH);
	::GetModuleFileName(NULL, buffer, MAX_PATH);

	CString strPath = buffer;
	int pos = strPath.ReverseFind(TEXT('\\'));
	strPath = strPath.Mid(0, pos);
	
	APK_PATH = strPath;
	APK_PATH += "\\adb\\IICOperator.apk";


}


CProcInterface::CProcInterface(void)
{
	Init();
}


CProcInterface::~CProcInterface(void)
{
}

CString CProcInterface::GetAvailableI2CInterface()
{
	PARAM_T para;

	CString strBuses(TEXT("I2C Devices: \n\n"));
	for (int i = 0; i < 10; i++) 
	{
		para.strCmd.Format(TEXT("adb shell cat /sys/bus/i2c/devices/i2c-%d/%d*/name"), i, i);
		para.nType = CMD_INFINITE;

		para.bRet = CAdbInterface::CreateAdbProcess(&para);

		if (para.strReturn.GetLength() > 2) {
			if (para.strReturn.Find(TEXT("No such")) != -1) 
			{
				continue;
			}
			para.strReturn.Replace(TEXT('\r'), TEXT('\n'));
			int pos = para.strReturn.Find(TEXT('\n'));

			strBuses.AppendFormat(TEXT("i2c-%d: %s\n"), i, para.strReturn.Mid(0, pos));
				
		}
	}
	
	return strBuses;
}

BOOL CProcInterface::OpenProc() 
{
	BOOL bSuccess = FALSE;
	PARAM_T para;

	para.strCmd = TEXT("adb root");

	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (para.strReturn.Find(TEXT("already")) == SYMBOL_NOT_FOUND) 
	{
		Sleep(2000);
		//para.strCmd = TEXT("adb root");
		//para.nType = CMD_INFINITE;
		//para.bRet = CAdbInterface::CreateAdbProcess(&para);
	}
	para.strCmd = TEXT("adb shell setenforce 0");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	para.strCmd = TEXT("adb shell chmod 777 /dev/i2c*");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (para.strReturn.GetLength() < 2) {
		bSuccess = TRUE;
	}

	if (!bSuccess) {

		para.strCmd = TEXT("adb shell su -c \"setenforce 0\"");
		para.nType = CMD_INFINITE;
		para.bRet = CAdbInterface::CreateAdbProcess(&para);

		para.strCmd = TEXT("adb shell su -c \"chmod 777 /dev/i2c*\"");
		para.nType = CMD_INFINITE;
		para.bRet = CAdbInterface::CreateAdbProcess(&para);

		if (para.strReturn.GetLength() < 2) {
			bSuccess = TRUE;
		}

	}

	return bSuccess;
}


int CProcInterface::ReadRegister(int addr) 
{
	u8 data[1] = {0};
	data[0] = (u8)(addr & 0xFF);
	if (WriteCmdFile(data, 1, PROC_READ_REGISTER)) 
	{
		if (SendCmdFile()) 
		{
			if (GetEchoData(data, 1)) 
			{
				TRACE(TEXT("Read 0x%02X: 0x%02X\n"), addr, data[0]);
				return (int)data[0];
			}
		}
	}

	TRACE(TEXT("Read 0x%02X failed\n"), addr);
	return 0xFFFF;
}



BOOL CProcInterface::WriteRegister(int addr, int value) 
{
	u8 data[2];
	data[0] = (u8)(addr & 0xFF);
	data[1] = (u8)(value & 0xFF);

	TRACE(TEXT("Write 0x%02X 0x%02X\n"), addr, value);
	if (WriteCmdFile(data, 2, PROC_WRITE_REGISTER)) 
	{
		return SendCmdFile();
	}
	return FALSE;
}

BOOL CProcInterface::Write(u8 *data, int len) 
{
	if (WriteCmdFile(data, len, PROC_WRITE_DATA)) 
	{
		return SendCmdFile();
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// 底层驱动限制, 读操作暂时受限
BOOL CProcInterface::Read(int addr, u8 *buffer, int len) 
{
	u8 cmdBuf[1];
	cmdBuf[0] = (u8)(addr & 0xFF);

	return Read(cmdBuf, 1, buffer, len);
}

BOOL CProcInterface::Read(u8* wrBuffer, int wrLen, u8 *rdBuffer, int rdLen)
{
	if (WriteCmdFile(wrBuffer, wrLen, PROC_READ_DATA)) 
	{
		if (SendCmdFile()) 
		{
			return GetEchoData(rdBuffer, rdLen);
		}
	}

	return FALSE;
}


BOOL CProcInterface::WriteCmdFile(u8 *data, int len, int cmdFlag)
{
	DeleteFile(CMD_PATH);

	FILE *pFile;
	_wfopen_s(&pFile, CMD_PATH, TEXT("wb"));

	if (pFile == NULL) {
		return FALSE;
	}

	u8 cmdBuffer[1] = {0};
	cmdBuffer[0] = (u8)(cmdFlag & 0xFF);

	//////////////////////////////////////////////////////////////////////////
	// for debug
	CString strContents;
	strContents.Format(TEXT("%02X"), cmdBuffer[0]);

	for (int i = 0; i < len; i++) 
	{
		strContents.AppendFormat(TEXT(",%02X"), data[i]);
	}
	strContents += TEXT("\n");
	TRACE(TEXT("%s"), strContents);
	//////////////////////////////////////////////////////////////////////////

	fwrite(cmdBuffer, 1, 1, pFile);
	fwrite(data, 1, len, pFile);

	fclose(pFile);

	return TRUE;
}

BOOL CProcInterface::SendCmdFile() 
{
	PARAM_T para;
	
	para.strCmd = TEXT("adb push \"");
	para.strCmd += CMD_PATH;
	para.strCmd += TEXT("\" /sdcard/cmd");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);
	
	para.strCmd = TEXT("adb shell cat /sdcard/cmd > ");
	para.strCmd += m_strProcPath;
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	return TRUE;
}

BOOL CProcInterface::GetEchoData(u8 *buffer, int len) 
{
	PARAM_T para;

	para.strCmd = TEXT("adb shell cat ");
	para.strCmd += m_strProcPath;
	para.strCmd += " > /sdcard/echo";
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	//DeleteFile(ECHO_PATH);
	para.strCmd = TEXT("adb pull /sdcard/echo \"");
	para.strCmd += ECHO_PATH;
	para.strCmd += TEXT("\"");
	para.nType = CMD_INFINITE;
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	return AnaylseEcho(buffer, len);

}


BOOL CProcInterface::AnaylseEcho(u8 *buffer, int len) 
{
	FILE *pFile;
	_wfopen_s(&pFile, ECHO_PATH, TEXT("rb"));

	if (pFile == NULL) 
	{
		return FALSE;
	}

	int size = fread(buffer, 1, len, pFile);

	return (size == len);
}


CProcInterface IPROC;