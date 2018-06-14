#pragma once
#include "AdbInterface.h"
#include <vector>
using std::vector;


/*
ps 命令参数:-P -p -t -x -c [pid] [name]

-P 显示调度策略，通常是bg或fg，当获取失败将会是un和er
-p 显示进程的优先级和nice等级
-t 显示进程下的线程列表
-x 显示进程耗费的用户时间和系统时间，格式:(u:0, s:0)，单位:秒(s)
-c 显示进程耗费的CPU时间 (可能不兼容Android 4.0以前的老版本系统)
[pid] 过滤指定的进程PID
[name] 过滤指定的进程NAME
ps xxx 显示过滤指定名称的进程
和Linux的ps不同，Linux的ps命令可以这样子：ps -aux，Android上ps的参数并不能一起使用，如ps -txPc.

作者：IT枫
链接：https://www.jianshu.com/p/d18ed08b40c4
來源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
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
	CString		VSZ;   /* vitural size  虚拟耗用内存*/
	CString		RSS;   /* Resident Set Size 实际使用物理内存 */
	/* 
	进程正在睡眠的内核函数名称；
	该函数的名称是从/root/system.map文件中获得的
	*/
	CString		WCHAN; 
	/* 
	Android进程调度状态：
	1.foreground process (fg)
	2.visible process (ui)
	3.Service process 
	4.background process (bg)
	5.empty process
	*/
	CString		PCY; 
	/* 进程运行地址 */
	CString		ADDR_PC;	
	/*进程状态:
	D - 不可中断的睡眠态。
	R – 运行态
	S – 睡眠态
	T – 被跟踪或已停止
	Z – 僵尸态
	W - 进入内存交换（从内核2.6开始无效）
	X - 死掉的进程
	< - 高优先级
	N - 低优先级
	L - 有些页被锁进内存
	s - 包含子进程
	l - 多线程，克隆线程
	*+ - 位于后台的进程组 *
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

