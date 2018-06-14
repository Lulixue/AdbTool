#pragma once

#define UWM_PROC_AVAILABLE (WM_USER + 0x0010)
#define SYMBOL_NOT_FOUND	(-1)
typedef unsigned char u8;



const int PROC_UPGRADE			= 0;
const int PROC_READ_REGISTER	= 1;
const int PROC_WRITE_REGISTER 	= 2;
const int PROC_RAWDATA			= 3;
const int PROC_AUTOCLB 			= 4;
const int PROC_UPGRADE_INFO 	= 5;
const int PROC_WRITE_DATA 		= 6;
const int PROC_READ_DATA 		= 7;


class CProcInterface
{
public:
	enum 
	{
		OPERATE_READ = 0x01,
		OPERATE_WRITE = 0x02,
	};

	CProcInterface(void);
	~CProcInterface(void);
	static void Init();

	BOOL OpenProc();
	CString GetAvailableI2CInterface();

	BOOL Write(u8 *data, int len);
	BOOL Read(int addr, u8 *buffer, int len);
	BOOL Read(u8* wrBuffer, int wrLen, u8 *rdBuffer, int rdLen);

	BOOL WriteRegister(int addr, int value);
	int ReadRegister(int addr);

private:
	BOOL WriteCmdFile(u8 *data, int len, int type);
	BOOL SendCmdFile();
	BOOL GetEchoData(u8 *buffer, int len);
	BOOL AnaylseEcho(u8 *buffer, int len);

private:
	CString m_strProcPath;

	static CString CMD_PATH;
	static CString ECHO_PATH;
	static CString APK_PATH;
};


extern CProcInterface IPROC;