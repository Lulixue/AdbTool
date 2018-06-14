#include "StdAfx.h"
#include "InterruptManager.h"


CInterruptManager::CInterruptManager(void)
{
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

void CInterruptManager::ParseIntLine(const CString intline)
{
	int length = intline.GetLength();
	wchar_t single;
	CString strPart;
	vector<CString> vecPieces;

	for (int i = 0; i < length; i++) 
	{
		single = intline.GetAt(i);
		if ((single != TEXT(' ')) &&
			(single != TEXT('\t')))
		{
			strPart.AppendChar(single);
		}

		if ((single == TEXT(' ')) ||
			(single == TEXT('\t')) ||
			(i == (length-1)))
		{
			if (strPart.IsEmpty())
			{
				continue;
			}
			vecPieces.push_back(strPart);
			strPart.Empty();
		}
	}
	if (vecPieces.empty()) {
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	// 637:          0          0          0          0  smp2p_gpio   0  modem
	// System interrupt:
	// IPI0
	// IPI0:   1100952    1278965     603281     557013       Rescheduling interrupts
	//
	// 5.0:
	// 132:        0       GIC  PH
	// IPI0:       0        100         67         49         30          3
	//			   2          2  CPU wakeup interrupts


	CAndroidIntrrupt droidint;
	BOOL bSystemInt = FALSE;
	int order = DROID_INT_NO_COLON;
	int index = 0;
	CString title;

	droidint.line = intline;
	title = vecPieces[0];
	title.Trim(TEXT(':'));
	if (IsSystemInt(title)) {
		bSystemInt = TRUE;
		droidint.type = title;
		droidint.no = -1;
		droidint.system = TRUE;
		droidint.name.Format(TEXT("[Sys]%s"), title);
	} 
	
	if (vecPieces[0].Find(TEXT(":")) != -1) {
		droidint.no = _wtoi(title);
		index++;
	}
	order++;


	length = vecPieces.size();
	for (; index < length; index++) {
		switch (order) {
		case DROID_INT_CPUS:
			{
				map<int, int>::const_iterator cit = m_mapCpuIndexes.begin();
				int tmpIndex = index;
				/* cpu triggered */
				for (; (cit != m_mapCpuIndexes.end()) && 
					(index < length); cit++, index++)
				{
					droidint.cpu_triggered[cit->second] = _wtoi64(vecPieces[index]);
				}
				index = tmpIndex + m_mapCpuIndexes.size() - 1;
				if (bSystemInt) {
					order = DROID_INT_NAME;
				}
			}
			break;
		case DROID_INT_NO:
			{
				int tmp_no = _wtoi(vecPieces[index]);
				/* 5.0 */
				if (tmp_no != droidint.no) 
				{
					droidint.name = vecPieces[index];
				}
			}
			break;
		case DROID_INT_TYPE:
			droidint.type = vecPieces[index];
			break;
		case DROID_INT_NAME:
		default: 
			droidint.name.AppendFormat(TEXT("%s "), vecPieces[index]);
			break;
		}
		order++;
	}
	droidint.name = ADB.CleanString(droidint.name);


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

	for (int i = 0; i < length; i++) 
	{
		single = title.GetAt(i);
		if ((single != TEXT(' ')) &&
			(single != TEXT('\t')))
		{
			if ((single >= TEXT('0')) &&
				(single <= TEXT('9'))) {
				strPart.AppendChar(single);
			}
		}

		if ((single == TEXT(' ')) ||
			(single == TEXT('\t')) ||
			(i == (length-1)))
		{
			if (strPart.IsEmpty())
			{
				continue;
			}
			m_mapCpuIndexes[j++] = _wtoi(strPart);
			strPart.Empty();
		}
	}
}

void CInterruptManager::Refresh()
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell cat /proc/interrupts"));
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

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
		vec.push_back(cit->name);
	}
	return TRUE;
}