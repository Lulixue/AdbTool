#pragma once
#include "AdbInterface.h"
#include <vector>
using std::vector;


/*
ps �������:-P -p -t -x -c [pid] [name]

-P ��ʾ���Ȳ��ԣ�ͨ����bg��fg������ȡʧ�ܽ�����un��er
-p ��ʾ���̵����ȼ���nice�ȼ�
-t ��ʾ�����µ��߳��б�
-x ��ʾ���̺ķѵ��û�ʱ���ϵͳʱ�䣬��ʽ:(u:0, s:0)����λ:��(s)
-c ��ʾ���̺ķѵ�CPUʱ�� (���ܲ�����Android 4.0��ǰ���ϰ汾ϵͳ)
[pid] ����ָ���Ľ���PID
[name] ����ָ���Ľ���NAME
ps xxx ��ʾ����ָ�����ƵĽ���
��Linux��ps��ͬ��Linux��ps������������ӣ�ps -aux��Android��ps�Ĳ���������һ��ʹ�ã���ps -txPc.

���ߣ�IT��
���ӣ�https://www.jianshu.com/p/d18ed08b40c4
��Դ������
����Ȩ���������С���ҵת������ϵ���߻����Ȩ������ҵת����ע��������
*/

enum PS_PT_ORDER {
	PSPT_USER = 0,
	PSPT_PID,
	PSPT_TID,
	PSPT_PPID,
	PSPT_VSIZE,
	PSPT_RSS,
	PSPT_PCY,
	PSPT_WCHAN,
	PSPT_ADDR_PC,
	PSPT_STATUS,
	PSPT_NAME,
	PSPT_SIZE,
};

class CAndroidProcess 
{
public:
	CString		USER;
	CString		PID;   /* process ID */
	CString		PPID;  /* parent-PID */
	CString		TID;  /* thread-ID */
	CString		VSZ;   /* vitural size  ��������ڴ�*/
	CString		RSS;   /* Resident Set Size ʵ��ʹ�������ڴ� */
	/* 
	��������˯�ߵ��ں˺������ƣ�
	�ú����������Ǵ�/root/system.map�ļ��л�õ�
	*/
	CString		WCHAN; 
	/* 
	Android���̵���״̬��
	1.foreground process (fg)
	2.visible process (ui)
	3.Service process 
	4.background process (bg)
	5.empty process
	*/
	CString		PCY; 
	/* �������е�ַ */
	CString		ADDR_PC;	
	/*����״̬:
	D - �����жϵ�˯��̬��
	R �C ����̬
	S �C ˯��̬
	T �C �����ٻ���ֹͣ
	Z �C ��ʬ̬
	W - �����ڴ潻�������ں�2.6��ʼ��Ч��
	X - �����Ľ���
	< - �����ȼ�
	N - �����ȼ�
	L - ��Щҳ�������ڴ�
	s - �����ӽ���
	l - ���̣߳���¡�߳�
	*+ - λ�ں�̨�Ľ����� *
	*/
	CString STATUS;
	CString NAME;
	CString m_strLine;
	vector<CAndroidProcess> m_vecSubThreads;
	BOOL IsSystemPID() const {
		return PPID == TEXT('0');
	}
public:
	void Reset();
	void ClearSubThreads() {
		m_vecSubThreads.clear();
	}
	~CAndroidProcess() {
		ClearSubThreads();
	}
	CAndroidProcess() {
		Reset();
	}
	CAndroidProcess(CString line) {
		Reset();
		ParsePsLine(line);
	}
	CString GetValue(vector<CString> &values, int type, CString default = TEXT(""));
	void ParsePsLine(CString strLine);
	void ParsePsPieces(vector<CString> &pieces);
	CString ToHReadMemory() const;
	CString ToHReadVirtualMemory() const;
	long GetMemory() const { return _wtol(RSS); }
	long GetVirtMemory() const { return _wtol(VSZ); }
	BOOL HasSubThreads() const { return m_vecSubThreads.size() != 0; }
	static void ParseTitleLine(CString line);
	static BOOL HasItemType(int type);
	static map<UINT, int> s_mapAttrOrders;
private:
	CString _ToHReadMemory(CString size) const;
};


class CAdbProcessManager
{
public:
	CAndroidProcess GetDroidProcess(CString PID) const;
	BOOL ProcessHasChildren(CString PID) const;
	BOOL GetChildren(CString PID, vector<CAndroidProcess> &children) const;
	void InitProcesses();
	CAdbProcessManager(void);
	~CAdbProcessManager(void);
public:
	static map<CString, int> MAP_PS_ATTRS;
	static void Init();
private:
	BOOL IsPackage(CString name) const;
	BOOL IsTitleLine(CString &line) const;
	void GenerateProcessTree();
	vector<CAndroidProcess> m_vecAllProcess;
	vector<CAndroidProcess> m_vecTreeProcess;
	vector<CAndroidProcess> m_vecPackageProcess;
public:
	vector<CAndroidProcess> GetAllProcess() const { return m_vecAllProcess; }
	vector<CAndroidProcess> GetTreeProcess() const { return m_vecTreeProcess; }
	vector<CAndroidProcess> GetPackageProcess() const { return m_vecPackageProcess; }

};

