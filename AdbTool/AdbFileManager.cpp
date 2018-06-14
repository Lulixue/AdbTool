#include "StdAfx.h"
#include "AdbFileManager.h"
#include <regex>
#include <string>

/* 用文件 5s
 * 不用: 20s 
*/
#define MANAGER_LS_FILE			1
const CString CAndroidFile::ROOT_DIR = TEXT("/");
const CString CAndroidFile::SDCARD_DIR = TEXT("/sdcard");
const CString CAndroidFile::EXT_SDCARD_DIR = TEXT("/storage/extsd");
const CString CAndroidFile::STORAGE_DIR = TEXT("/storage");
CCriticalSection CAndroidFile::S_CS;
set<CAndroidFile*> CAndroidFile::S_SET_FILES;
set<CString> CAndroidFile::FIRST_LOAD_DIRS;

BOOL CAndroidFile::IsFirstLoadDir(CAndroidFile *pFile)
{
	return FIRST_LOAD_DIRS.find(pFile->m_strPath) != FIRST_LOAD_DIRS.end();
}

void CAndroidFile::AndroidFileInit()
{
	FIRST_LOAD_DIRS.clear();
	FIRST_LOAD_DIRS.insert(STORAGE_DIR);
	FIRST_LOAD_DIRS.insert(SDCARD_DIR);
}

CString CAndroidFile::TypeToString(int type)
{
	switch (type) {
	case FTYPE_DIR:
		return TEXT("d");
	case FTYPE_CHAR:
		return TEXT("c");
	case FTYPE_LINK:
		return TEXT("l");
	case FTYPE_BLOCK:
		return TEXT("b");
	case FTYPE_FILE:
	default:
		return TEXT("f");	
	}
}

void CAndroidFile::AddFile(CAndroidFile *pFile)
{
	S_CS.Lock();
	S_SET_FILES.insert(pFile);
	S_CS.Unlock();
}
void CAndroidFile::ReleaseAllFile()
{
	S_CS.Lock();
	TRACE("Release All Android Files\n");
	set<CAndroidFile*>::const_iterator cit = S_SET_FILES.begin();
	for (; cit != S_SET_FILES.end(); cit++) 
	{
		delete *cit;
	}
	S_SET_FILES.clear();
	S_CS.Unlock();
}

CAdbFileManager::CAdbFileManager(void)
{
	CAndroidFile::AndroidFileInit();
}

CAdbFileManager::~CAdbFileManager(void)
{

}

void CAdbFileManager::Refresh()
{
	m_fileRoot.Reset();

	m_fileRoot.m_strName = TEXT("File System");
	m_fileRoot.m_strPath = CAndroidFile::ROOT_DIR;
	m_fileRoot.m_nDepth = 0;
	m_fileRoot.m_nType = FTYPE_DIR;
}


CAndroidFile* CAdbFileManager::GetFile(const FileIndex &indexes, BOOL bForceUpdate /* = FALSE */)
{
	CAndroidFile *pfile = NULL;
	const int size = indexes.size();
	for (int i = 0; i < size; i++)
	{
		unsigned sub = indexes[i];
		if ((i == sub) && (sub == 0)) {
			pfile = &m_fileRoot;
		} else {
			if (pfile == NULL) {
				break;
			}
			if (pfile->m_vecSubfiles.empty()) {
				if (bForceUpdate) {
					pfile->Expand();
				}
			}
			if (pfile->m_vecSubfiles.size() <= sub) {
				return NULL;
			}
			pfile = pfile->m_vecSubfiles[sub];
		}
	}
	return pfile;
}


void CAndroidFile::Expand(int depth/* = 2*/)
{
	InitSubFiles(depth);
	m_bExpanded = TRUE;
}

void CAndroidFile::PrintInfo()
{
	CString strLink;
	strLink.Format(TEXT("(->%s)"), m_strLink);
	TRACE(TEXT("name: %s, path:%s%s, size:%ld, count:%ld\n"), 
		m_strName, m_strPath, IsLink() ? strLink : TEXT(""), m_llSize, m_lCount);
}

BOOL CAndroidFile::IsTotalLine(CString &strLine)
{
	static const std::wstring totalPat(TEXT("total ([0-9]+)"));
	static const std::wregex totalPattern(totalPat);

	std::wstring line(strLine.GetString());

	std::wsmatch results;
	std::regex_search(line, results, totalPattern);

	if (results.size() > 0) {
		return TRUE;
	}
	return FALSE;
}

void CAndroidFile::InitSubFiles(int depth/* = 2*/)
{
	if (!IsDirectory()) 
	{
		return;
	}
	if (depth <= 0) {
		if ((depth == 0) && 
			IsFirstLoadDir(this))
		{
			// 强制加载部分重要目录
		} else {
			return;
		}
	}
	ClearSubfiles();
	int parDepth = m_nDepth;
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell ls -l %s"), IsLink() ? m_strLink : m_strPath);
	para.nType = CMD_INFINITE;
	if (MANAGER_LS_FILE) {
		para.nType |= CMD_READ_BACK_FILE;
	}
	
	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (!para.bRet) {
		return;
	}
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
			if (IsTotalLine(strLine)){
				strLine.Empty();
				continue;
			}
			CAndroidFile *pNewFile = new CAndroidFile(strLine, parDepth, this);
			if (pNewFile->IsCurrentDir()) {
				m_lCount = pNewFile->m_lCount;
				m_llSize = pNewFile->m_llSize;
				delete pNewFile;
				continue;
			} else if (!pNewFile->IsParentDir()) {
				AddFile(pNewFile);
				pNewFile->Expand(depth-1);
				m_vecSubfiles.push_back(pNewFile);
			}
			strLine.Empty();
		} 
	}
}

void CAndroidFile::Reset()
{
	m_pParent = NULL;
	m_lCount = 0L;
	m_llSize = 0L;
	m_nDepth = 0;
	m_nMode = 0;
	m_nType = FTYPE_FILE;

	m_strPath.Empty();
	m_strParent.Empty();
	m_strName.Empty();
	m_strOwner.Empty();
	m_strGroup.Empty();
	m_strTime.Empty();
	m_strMode.Empty();
	m_strLink.Empty();
	m_vecLinkStack.clear();
	ClearSubfiles();
}
void CAndroidFile::GeneratePath()
{
	int length = m_strParent.GetLength();
	ASSERT(length > 0);
	if (m_strParent.GetAt(length-1) == TEXT('/')) {
		m_strPath.Format(TEXT("%s%s"), m_strParent, m_strName);
	} else {
		m_strPath.Format(TEXT("%s/%s"), m_strParent, m_strName);
	}
}
BOOL CAndroidFile::ValueIsDate(CString strValue)
{
	static const std::wstring wsFacPat(TEXT("^[0-9]{4}\\-[0-9]{2}\\-[0-9]{2}$"));
	static const std::wregex facPattern(wsFacPat);

	std::wstring line(strValue.GetString());

	std::wsmatch results;
	std::regex_search(line, results, facPattern);

	if (results.size() > 0) {
		return TRUE;
	}
	return FALSE;
}
BOOL CAndroidFile::ValueIsNumbers(CString &strValue)
{
	WCHAR single;
	int size = strValue.GetLength();
	for (int i = 0; i < size; i++) {
		single = strValue.GetAt(i);
		if (single < TEXT('0')) {
			return FALSE;
		} else if (single > TEXT('9')) {
			return FALSE;
		}
	}
	return TRUE;
}
/*
 * Dir:
 *		drwxrwx--- root     sdcard_r          1970-01-02 02:01 obb
 * File:
 *		-rw-rw---- root     sdcard_r    52725 1970-01-01 09:00 focal_lastkmsg.txt
 * size can be [EMPTY] if it's dir
 */
void CAndroidFile::ParseLlLine(CString llLine)
{
	int length = llLine.GetLength();
	wchar_t single;
	CString strPart;
	m_strLine = llLine;
	vector<CString> vecPieces;

	for (int i = 0; i < length; i++) 
	{
		single = llLine.GetAt(i);
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

	ParseLsPieces(vecPieces);
	GeneratePath();

	if (IsLink()) {
		/* 如果当前文件夹有同样的link文件则直接赋值 */
		if (!AssignParentLinks(m_strLink, this, m_pParent)) {
			m_nType = LinkPathIsDir(m_strPath) ? FTYPE_DIR : FTYPE_FILE;
			m_strLink = ParseToRealPath(m_strLink);
		}
	}
}
/* 
	before: drwxrwx--x root sdcard_rw 4096 1970-01-01 00:08 Android
	oreo: drwxrwx--x *3* root sdcard_rw 4096 1970-01-01 00:08 Android
	lrwxr-xr-x 1 root   shell          6 2018-03-06 11:19 chcon -> toybox
*/

void CAndroidFile::ParseLsPieces(vector<CString> &vecPieces)
{
	int prop = FPROP_BEGIN;
	BOOL bLink = FALSE;
	int size = vecPieces.size();
	if (size <= 0) {
		return;
	}
	CString strPart;
	for (int i = 0; i < size; i++) {
		strPart = vecPieces.at(i);
		prop++;
		if ((prop < FPROP_DATE) && ValueIsDate(strPart)) {
			prop = FPROP_DATE;
		}
		switch (prop) {
		case FPROP_MODE:
			{
				ParseMode(strPart);
				break;
			}
		case FPROP_NAME:
			{
				m_strName = strPart;
				break;
			}
		case FPROP_LINK:
			{
				if (bLink) 
				{
					m_strLink = strPart;
				}
				break;
			}
		case FPROP_DIR_TO:
			{
				if (!strPart.CompareNoCase(TEXT("->"))) {
					bLink = TRUE;
				}
				break;
			}
		case FPROP_DEPTH:
			if (ValueIsNumbers(strPart)) {
				break;
			}
		case FPROP_OWNER:
			{
				m_strOwner = strPart;
			}
			break;
		case FPROP_GROUP:
			{
				m_strGroup = strPart;
			}
			break;
		case FPROP_SIZE:
			{
				m_llSize = _wtol(strPart.GetString());
			}
			break;
		case FPROP_DATE:
			{
				m_strTime = strPart;
			}
			break;
		case FPROP_TIME:
			{
				m_strTime.Append(TEXT(" "));
				m_strTime.Append(strPart);
			}
			break;
		default:
			break;
		}
	}
	if (m_strName.IsEmpty()) {
		m_strName = vecPieces.at(size-1);
	}
	if (IsLink()) {
		if (m_strLink.GetAt(0) != TEXT('/')) {
			m_strLink.Format(TEXT("%s/%s"), m_strParent, m_strLink);
		}
		m_vecLinkStack.push_back(m_strLink);
	}

}

BOOL CAndroidFile::LinkPathIsDir(CString strPath)
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell cd %s"), strPath);
	para.nType = CMD_INFINITE;

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	if (para.strReturn.Find(TEXT("No such")) != -1) {
		return FALSE;
	}
	return TRUE;

}

BOOL CAndroidFile::AssignParentLinks(CString strLink, CAndroidFile *pDestFile, CAndroidFile *pFile) const
{
	if (!pFile) {
		return FALSE;
	}
	vector<CAndroidFile*>::const_iterator cit = pFile->m_vecSubfiles.begin();
	vector<CString>::const_iterator lcit;
	vector<CString>::const_reverse_iterator rlcit;

	for (; cit != pFile->m_vecSubfiles.end(); cit++)
	{
		CAndroidFile *subfile = (*cit);
		if (subfile->HasLinkStack()) {
			lcit = subfile->m_vecLinkStack.begin();
			for (; lcit != subfile->m_vecLinkStack.end(); lcit++) {
				if (!lcit->Compare(strLink)) {
					pDestFile->m_strLink = subfile->m_strPath;
					pDestFile->m_nType = subfile->m_nType;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

CString CAndroidFile::ParseToRealPath(CString strPath)
{
	PARAM_T para;
	para.strCmd.Format(TEXT("adb shell ls -l %s"), strPath);
	para.nType = CMD_INFINITE;
	if (MANAGER_LS_FILE) {
		para.nType |= CMD_READ_BACK_FILE;
	}

	para.bRet = CAdbInterface::CreateAdbProcess(&para);

	CAndroidFile newFile;

	int length = para.strReturn.GetLength();
	wchar_t single;
	CString strPart;
	vector<CString> vecPieces;

	for (int i = 0; i < length; i++) 
	{
		single = para.strReturn.GetAt(i);
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
	newFile.m_strPath = strPath;
	newFile.ParseLsPieces(vecPieces);
	if (newFile.IsLink()) { {
			return ParseToRealPath(newFile.m_strLink);
		}
	}
	return newFile.m_strPath;
}

int CAndroidFile::ModeToValue(CString strMode)
{
	int ret = 0;
	if (strMode.GetAt(0) != TEXT('-')) { ret += 4;}
	if (strMode.GetAt(1) != TEXT('-')) { ret += 2;}
	if (strMode.GetAt(2) != TEXT('-')) { ret += 1;}

	return ret;
}

void CAndroidFile::ParseMode(CString strMode)
{
	if (strMode.GetLength() < 9) {
		return;
	}
	m_strMode = strMode;

	wchar_t ftype = strMode.GetAt(0);
	switch (ftype) {
	case TEXT('d'):
		m_nType = FTYPE_DIR;
		break;
	case TEXT('l'):
		m_nType = FTYPE_LINK;
		break;
	case TEXT('c'):
		m_nType = FTYPE_CHAR;
		break;
	case TEXT('b'):
		m_nType = FTYPE_BLOCK;
		break;
	case TEXT('-'):
	default:
		m_nType = FTYPE_FILE;
	}

	CString rootMode = strMode.Mid(1, 3);
	CString groupMode = strMode.Mid(4, 3);
	CString othersMode = strMode.Mid(7, 3);
	int value = ModeToValue(rootMode) * 100;
	value += ModeToValue(groupMode) * 10;
	value += ModeToValue(othersMode);

	m_nMode = value;
}