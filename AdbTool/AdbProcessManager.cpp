#include "StdAfx.h"
#include "AdbProcessManager.h"
#include <set>
#include <regex>
using std::set;

map<UINT, int> CAndroidProcess::s_mapAttrOrders;
map<CString, int> CAdbProcessManager::MAP_PS_ATTRS;

void CAdbProcessManager::Init()
{
	MAP_PS_ATTRS[TEXT("USER")] = PSPT_USER;
	MAP_PS_ATTRS[TEXT("PID")] = PSPT_PID;
	MAP_PS_ATTRS[TEXT("TID")] = PSPT_TID;
	MAP_PS_ATTRS[TEXT("PPID")] = PSPT_PPID;
	MAP_PS_ATTRS[TEXT("VSZ")] = PSPT_VSIZE;
	MAP_PS_ATTRS[TEXT("VSIZE")] = PSPT_VSIZE;
	MAP_PS_ATTRS[TEXT("RSS")] = PSPT_RSS;
	MAP_PS_ATTRS[TEXT("PCY")] = PSPT_PCY;
	MAP_PS_ATTRS[TEXT("WCHAN")] = PSPT_WCHAN;
	MAP_PS_ATTRS[TEXT("ADDR")] = PSPT_ADDR_PC;
	MAP_PS_ATTRS[TEXT("PC")] = PSPT_ADDR_PC;
	MAP_PS_ATTRS[TEXT("S")] = PSPT_STATUS;
	MAP_PS_ATTRS[TEXT("NAME")] = PSPT_NAME;
	MAP_PS_ATTRS[TEXT("CMD")] = PSPT_NAME;
}

void CAndroidProcess::Reset()
{
	TID = TEXT("-");
	PID = TEXT("-");
	PPID = TEXT("-");
	VSZ = TEXT("0");
	RSS = TEXT("0");
	PCY = TEXT("-");
	ADDR_PC = TEXT("-");
	STATUS = TEXT("-");
	USER = TEXT("-");
	NAME = TEXT("-");
	WCHAN = TEXT("-");
	ClearSubThreads();
}

CString CAndroidProcess::GetValue(vector<CString> &values, int type, CString default)
{
	map<UINT, int>::const_iterator cit = s_mapAttrOrders.begin();
	for (; cit != s_mapAttrOrders.end(); cit++)
	{
		if (cit->second == type) 
		{
			if (cit->first < values.size()) {
				return values[cit->first];
			}
			break;
		}
	}
	return default;
}

void CAndroidProcess::ParsePsPieces(vector<CString> &pieces)
{

	USER = GetValue(pieces, PSPT_USER);
	PID = GetValue(pieces, PSPT_PID);
	TID = GetValue(pieces, PSPT_TID, PID);
	PPID = GetValue(pieces, PSPT_PPID);
	VSZ = GetValue(pieces, PSPT_VSIZE);
	RSS = GetValue(pieces, PSPT_RSS);
	PCY = GetValue(pieces, PSPT_PCY);
	WCHAN = GetValue(pieces, PSPT_WCHAN);
	ADDR_PC = GetValue(pieces, PSPT_ADDR_PC);
	STATUS = GetValue(pieces, PSPT_STATUS);
	NAME = GetValue(pieces, PSPT_NAME);
}
/* 
5.0/6.0/7.0:
USER     PID   PPID  VSIZE  RSS    PCY  WCHAN    PC        NAME
root      1     0     17060  652   fg  c0116cc4 000369c4 S /init 

8.0:
USER           PID   TID  PPID     VSZ    RSS WCHAN            ADDR S CMD          
root             1     1     0   19080   3324 SyS_epoll_wait 521354 S init
*/
void CAndroidProcess::ParsePsLine(CString strLine)
{
	int length = strLine.GetLength();
	wchar_t single;
	CString strPart;
	m_strLine = strLine;
	vector<CString> vecPieces;

	for (int i = 0; i < length; i++) 
	{
		single = strLine.GetAt(i);
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

	ParsePsPieces(vecPieces);

}

void CAndroidProcess::ParseTitleLine(CString line)
{
	int length = line.GetLength();
	wchar_t single;
	CString strPart;
	vector<CString> vecPieces;

	for (int i = 0; i < length; i++) 
	{
		single = line.GetAt(i);
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

			if (i == (length-1))
			{
				if (vecPieces.at(vecPieces.size()-1) == TEXT("PC")) {
					vecPieces.push_back(TEXT("S"));
				}
			}

			vecPieces.push_back(strPart);

			strPart.Empty();
		}
	}



	map<CString, int>::const_iterator cit;
	s_mapAttrOrders.clear();
	length = vecPieces.size();
	for (int i = 0; i < length; i++)
	{
		cit = CAdbProcessManager::MAP_PS_ATTRS.find(vecPieces[i]);
		if (cit != CAdbProcessManager::MAP_PS_ATTRS.end()) {
			s_mapAttrOrders[i] = cit->second;
		}
	}

}



long long GetMemory(CString size)  
{ 
	return _wtoi64(size); 
}

CString _ToHReadMemoryByte(long long sizeByte)
{
	long sizeKB = (long)(sizeByte / 1024);
	double sizeMB = sizeKB / 1024.0;
	double sizeGB = sizeMB /1024.0;
	CString strRet;
	if (sizeGB > 1) {
		strRet.Format(TEXT("%.1fG"), sizeGB);
	}
	else if (sizeMB > 10) 
	{
		strRet.Format(TEXT("%.1fM"), sizeMB);	
	}
	else if (sizeKB > 1) {
		strRet.Format(TEXT("%ldK"), sizeKB);
	} else {
		strRet.Format(TEXT("%ldB"), sizeByte);
	}
	return strRet;
}

CString _ToHReadMemoryKB(CString sizeKB)
{
	return _ToHReadMemoryByte(GetMemory(sizeKB) * 1024);
}

CString CAndroidProcess::ToHReadMemory() const
{
	return _ToHReadMemoryKB(RSS);
}
CString CAndroidProcess::ToHReadVirtualMemory() const
{
	return _ToHReadMemoryKB(VSZ);
}

CAdbProcessManager::CAdbProcessManager(void)
{
}


CAdbProcessManager::~CAdbProcessManager(void)
{
}
/* 
5.0,6.0,7.0: USER     PID   PPID  VSIZE  RSS    PCY  WCHAN    PC        NAME 
8.0:
USER           PID   TID  PPID     VSZ    RSS WCHAN            ADDR S CMD   

*/
BOOL CAdbProcessManager::IsTitleLine(CString &line) const
{
	if (line.Find(TEXT("USER")) != -1) {
		return TRUE;
	}
	return FALSE;
}

void CAdbProcessManager::InitProcesses()
{
	PARAM_T para;
	if (ADB.OsHigher(ANDROID_O)) {
		/* -T -O PCY  */
		para.strCmd.Format(TEXT("adb shell ps -A"));
	}
	else {
		para.strCmd.Format(TEXT("adb shell ps -t -P"));
	}
	para.nType = CMD_INFINITE | CMD_READ_BACK_FILE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (!para.bRet) {
		return;
	}
	m_vecAllProcess.clear();
	m_vecTreeProcess.clear();
	m_vecPackageProcess.clear();
	int length = para.strReturn.GetLength();
	wchar_t single;
	CString strLine;
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
			if (IsTitleLine(strLine)){
				CAndroidProcess::ParseTitleLine(strLine);
				strLine.Empty();
				continue;
			}
			m_vecAllProcess.push_back(CAndroidProcess(strLine));
			strLine.Empty();
		} 
	}
	GenerateProcessTree();
}

BOOL CAndroidProcess::HasItemType(int type)
{
	map<UINT, int>::const_iterator cit = s_mapAttrOrders.begin();
	for (; cit != s_mapAttrOrders.end(); cit++) {
		if (cit->second == type) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CAdbProcessManager::IsPackage(CString name) const
{
	static const std::wstring pkgPat(TEXT("(^[a-z0-9]{2,})\\.([a-z0-9]{2,})"));
	static const std::wregex pktRegex(pkgPat);

	std::wstring line(name.GetString());

	std::wsmatch results;
	std::regex_search(line, results, pktRegex);

	if (results.size() > 0) {
		return TRUE;
	}
	return FALSE;
}

void CAdbProcessManager::GenerateProcessTree()
{
	map<CString, BOOL> ppids;
	map<CString, BOOL> pids;
	vector<CAndroidProcess>::const_iterator cit;
	vector<CAndroidProcess>::iterator tit;
	vector<CAndroidProcess> vecTmpTree;
	map<CString, BOOL> *pParentID;
	for (cit = m_vecAllProcess.begin(); cit != m_vecAllProcess.end(); cit++)
	{
		if (IsPackage(cit->NAME)) {
			m_vecPackageProcess.push_back(*cit);
		}
		pids[cit->PID] = FALSE;
		if (cit->IsSystemPID()) 
		{
			continue;
		}
		ppids[cit->PPID] = FALSE;
	}

	if (CAndroidProcess::HasItemType(PSPT_TID)) {
		pParentID = &pids;
	} else {
		pParentID = &ppids;
	}

	map<CString, BOOL>::iterator idit;
	for (cit = m_vecAllProcess.begin(); cit != m_vecAllProcess.end(); cit++)
	{
		idit = pParentID->find(cit->PID);
		if (idit != pParentID->end()) {
			if (!idit->second) {
				idit->second = TRUE;
				vecTmpTree.push_back(*cit);
			}
		}
	}

	for (cit = m_vecAllProcess.begin(); cit != m_vecAllProcess.end(); cit++)
	{
		for (tit = vecTmpTree.begin(); tit != vecTmpTree.end();
			tit++) 
		{
			if ((tit->PID == cit->PPID)) 
			{
				tit->m_vecSubThreads.push_back(*cit);
			} 
			else if ((tit->PID == cit->PID) && (tit->TID != cit->TID)) 
			{
				CAndroidProcess proc = *cit;
				proc.PID = proc.TID;
				tit->m_vecSubThreads.push_back(proc);
			}
		}
	}
	m_vecTreeProcess = vecTmpTree;
}

BOOL CAdbProcessManager::GetChildren(CString PID, vector<CAndroidProcess> &children) const
{
	vector<CAndroidProcess>::const_iterator cit;
	for (cit = m_vecTreeProcess.begin(); cit != m_vecTreeProcess.end(); cit++)
	{
		if (cit->PID == PID) {
			children = cit->m_vecSubThreads;
			return TRUE;
		}
	}
	return FALSE;
}

CAndroidProcess CAdbProcessManager::GetDroidProcess(CString pid) const
{
	vector<CAndroidProcess>::const_iterator cit;
	for (cit = m_vecAllProcess.begin(); cit != m_vecAllProcess.end(); cit++)
	{
		if (cit->PID == pid) {
			return *cit;
		}
	}
	return CAndroidProcess();
}

BOOL CAdbProcessManager::ProcessHasChildren(CString PID) const
{
	vector<CAndroidProcess>::const_iterator tit;
	for (tit = m_vecTreeProcess.begin(); tit != m_vecTreeProcess.end();
		tit++) 
	{
		if (tit->PID == PID) {
			return TRUE;
		}
	}
	return FALSE;
}