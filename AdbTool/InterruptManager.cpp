#include "StdAfx.h"
#include "InterruptManager.h"
#include <algorithm>
#include <regex>
using namespace std;


static std::wregex NONE_EMPTY_REGEX(TEXT("(\\S+)"));
void splitString(CString str, CString separator, vector<CString>& result) {
	 
	std::wstring wstr(str.GetString());
	wsmatch match; 

	wstring::const_iterator searchStart(wstr.cbegin());

	while (std::regex_search(searchStart, wstr.cend(), match, NONE_EMPTY_REGEX)) {  
		result.push_back(CString(match.str(1).c_str()));
		searchStart = match.suffix().first;
	}
 }

CInterruptManager::CInterruptManager(void)
{
	if (ADB.OsHigher(ANDROID_R)) {
		m_vecIntTypes.push_back(DROID_INT_NAME);
		m_vecIntTypes.push_back(DROID_INT_GPIO);
		m_vecIntTypes.push_back(DROID_INT_TYPE);
		m_vecIntTypes.push_back(DROID_INT_DTS_NAME);
	}
	else {
		m_vecIntTypes.push_back(DROID_INT_NAME);
		m_vecIntTypes.push_back(DROID_INT_GPIO);
		m_vecIntTypes.push_back(DROID_INT_DTS_NAME);
	}
}


CInterruptManager::~CInterruptManager(void)
{

}

BOOL CInterruptManager::IsSystemInt(CString noColon) const
{
	int length = noColon.GetLength();
	wchar_t single;
	for (int i = 0; i < length; i++)
	{
		single = noColon.GetAt(i);
		if (single == TEXT(':')) {
			continue;
		}
		if (single > TEXT('9')) {
			return TRUE;
		} else if (single < TEXT('0')) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL StringIsDigits(const CString line)
{
    int length = line.GetLength();
    if (length <= 0)
    {
        return FALSE;
    }
    WCHAR single;
    for (int i = 0; i < length; i++)
    {
        single = line[i];
        if ((single > L'9') ||
            (single < L'0'))
        {
            return FALSE;
        }
    }
    return TRUE;
}

void CInterruptManager::ParseIntLine(const CString intline)
{ 
	vector<CString> vecPieces;

	splitString(intline, TEXT(" "), vecPieces);
	 
	if (vecPieces.empty()) {
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	// --- 11.0 ---
	//   3:   20869956   19807750      9059662    7073026     GICv3  27 Level     arch_timer
    // ----8.0 ----
    //  1:          1          0          0          0          0          0  qpnp-int  usbin-uv
	// ----6.0----
	// 560:          0   mt-eint 176  pmic-eint
	// IPI0:     11447       Rescheduling interrupts
	// Err:          0
	//
	// 637:          0          0          0          0  smp2p_gpio   0  modem
	//
	// ----System interrupts---
	// IPI0
	// IPI0:   1100952    1278965     603281     557013       Rescheduling interrupts
	//
	// -----5.0-------------
	// 132:        0       GIC  PH
	// IPI0:       0        100         67         49         30          3
	//			   2          2  CPU wakeup interrupts


	CAndroidIntrrupt droidint; 
	int index = 0;
	CString title;

	droidint.line = intline;

	int length = vecPieces.size(); 
	// INT号
    title = vecPieces[0];
    title.Replace(L":", L"");
    if (IsSystemInt(title)) {
        droidint.type = title;
        droidint.system = TRUE;
        droidint.name.Format(L"[Sys-%s]", title.GetString());
    }
    else
    {
        droidint.no = _wtoi(title);
        droidint.name.Format(L"[%03d]", droidint.no);
    }
	// CPUS 
	map<int, int>::const_iterator cit = m_mapCpuIndexes.begin();
	index = 1;
	/* cpu triggered */
	for (; (cit != m_mapCpuIndexes.end()) && 
		(index < length); cit++, index++)
	{
		droidint.cpu_triggered[cit->second] = _wtoi64(vecPieces[index]);
	} 

	if (droidint.system) {
		CString name = droidint.name;
		for (; index < length; index++) {
			name += vecPieces[index] + TEXT(" ");
		}
		droidint.name = name;
	}
	else {
		droidint.controller = vecPieces[index++];
		do {
			if (index >= length) {
				break;
			}
			// 可能是gpio/dtsName
			auto value = vecPieces[index++];
			if (StringIsDigits(value))
			{
				droidint.gpio = _wtoi(value);
			}
			else {
				droidint.dtsName = value;
			}
			if (index >= length) {
				break;
			}
			// 可能是type/dtsName
			value = vecPieces[index];
			auto upper = value.MakeUpper();
			if (upper == "LEVEL" || upper == "EDGE") {
				droidint.type = upper;
				++index;
			}
			if (index >= length) {
				break;
			} 
			CString name;
			for (; index < length; index++) {
				name += vecPieces[index] + TEXT(" ");
			}
			droidint.dtsName = name.Trim();

		} while (0);
		if (!droidint.dtsName.IsEmpty()) {
			droidint.name.Append(droidint.dtsName);
		}
		else {
			droidint.name.Append(droidint.controller);
		}
	} 
	droidint.name = ADB.CleanString(droidint.name);
    TRACE(L"Line: %s\n", droidint.line);
    TRACE(L"Name: %s, no: %d, type: %s, gpio: %d\n", droidint.name, droidint.no, droidint.type, droidint.gpio);

	m_vecInterrupts.push_back(droidint);
}


void CInterruptManager::ParseTitleLine(CString title)
{
	title = title.MakeUpper();
	int length = title.GetLength();
	wchar_t single;
	CString strPart;
	vector<CString> vecPieces;
	int j = 0;
	vector<CString> result;
	splitString(title, TEXT(" "), result);

	for_each(result.begin(), result.end(), [this,&j](CString val) {
		TCHAR buffer[5] = { 0 };
		for (int i = 3, j = 0; i < val.GetLength(); i++, j++) {
			buffer[j] = val.GetAt(i);
		}
		m_mapCpuIndexes[j++] = _wtoi(buffer);
		});
	 
}

void CInterruptManager::Refresh()
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell cat /proc/interrupts"));
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

    TRACE(L"GetInterrupts: %s\n", para.strReturn);
	m_vecInterrupts.clear();
	m_mapCpuIndexes.clear();
	int length = para.strReturn.GetLength();
	wchar_t single;
	CString strLine;
	BOOL bFirstLine = TRUE;
	for (int i = 0; i < length; i++) {
		single = para.strReturn.GetAt(i);

		if ((single != TEXT('\n')) &&
			(single != TEXT('\r')))
		{
			strLine.AppendChar(single);
		}

		if ((single == TEXT('\n')) ||
			(single == TEXT('\r')) ||
			(i == (length-1)))
		{
			if (strLine.IsEmpty()) {
				continue;
			}

			if (bFirstLine){
				ParseTitleLine(strLine);
				strLine.Empty();
				bFirstLine = FALSE;
				continue;
			}
			ParseIntLine(strLine);
			strLine.Empty();
		} 
	}

}

BOOL CInterruptManager::NameToInt(const CString name, CAndroidIntrrupt &droidint) const
{
	vector<CAndroidIntrrupt>::const_iterator cit = m_vecInterrupts.begin();
	for (; cit != m_vecInterrupts.end(); cit++)
	{
		if (cit->name.Compare(name) == 0) 
		{
			droidint = *cit;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CInterruptManager::GetIntNames(vector<CString> &vec) const
{
	vector<CAndroidIntrrupt>::const_iterator cit = m_vecInterrupts.begin();
	for (; cit != m_vecInterrupts.end(); cit++)
	{
        if (!cit->name.IsEmpty())
        {
            vec.push_back(cit->name);
        }
	}
	return TRUE;
}