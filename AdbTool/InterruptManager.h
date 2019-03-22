#pragma once
#include "AdbInterface.h"


#define ALL_CPUS		0xFF

/*
adb shell cat /proc/interrupts
8.0:
481:        692          0          0          0   msmgpio 125  fts_ts
5.0:
CPU0       
1:         12  sunxi_gpio_irq_chip  PA3_EINT
*/

enum ANDROID_INT_ORDER {
	DROID_INT_NO_COLON = 0,
	DROID_INT_CPUS,
	DROID_INT_TYPE,
	DROID_INT_GPIO,
	DROID_INT_NAME,
};

class CAndroidIntrrupt 
{
public:
	CAndroidIntrrupt() : no(-1), gpio(-1), system(FALSE) {}

public:
	int no; /* irq no. */
	int gpio; /* gpio no. */
	/* cpu,triggered */
	map<int, long long> cpu_triggered;
	CString name;
	CString type;
	CString line;
	BOOL system;

public:
	BOOL IsSysInt() const {return system;}
};


class CInterruptManager
{
public:
	CInterruptManager(void);
	~CInterruptManager(void);
	void Refresh();

public:
	BOOL NameToInt(const CString name, CAndroidIntrrupt &droidint) const;
	BOOL GetIntNames(vector<CString> &vec) const;
private:
	BOOL IsSystemInt(CString noColon) const;
	void ParseTitleLine(CString title);
	void ParseIntLine(const CString intline);

private:
	vector<CAndroidIntrrupt> m_vecInterrupts;
	map<int, int> m_mapCpuIndexes;
};

