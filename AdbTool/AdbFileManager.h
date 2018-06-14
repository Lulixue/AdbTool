#pragma once
#include "AdbInterface.h"
#include <set>
using std::set;
//////////////////////////////////////////////////////////////////////////
//struct dentry {
//	struct qstr d_name;
//	unsigned long d_time;		/* used by d_revalidate */
//	unsigned int d_count;		/* protected by d_lock */
//};
/*
typedef unsigned __bitwise__ fmode_t;
struct file {
	atomic_long_t		f_count;
	unsigned int 		f_flags;
	fmode_t			f_mode;
	loff_t			f_pos;
	struct fown_struct	f_owner;
	const struct cred	*f_cred;
	struct file_ra_state	f_ra;
};
*/
//////////////////////////////////////////////////////////////////////////
enum AND_FILE_TYPE {
	FTYPE_FILE = 0,
	FTYPE_DIR = 1,
	FTYPE_CHAR = 2,
	FTYPE_BLOCK = 3,
	FTYPE_LINK = 4,
};
enum LL_FILE_PROP {
	FPROP_BEGIN = -1,
	FPROP_MODE,
	FPROP_DEPTH,
	FPROP_OWNER,
	FPROP_GROUP,
	FPROP_SIZE,  /* dir: [EMPTY], file: size */
	FPROP_DATE, 
	FPROP_TIME,
	FPROP_NAME,
	FPROP_DIR_TO,
	FPROP_LINK,
	FPROP_END,
};

enum {
	NOTIFY_ADD = 0,
	NOTIFY_REMOVE = 1,
	NOTIFY_RESET = 2,
};

CString _ToHReadMemoryByte(long sizeByte);
CString _ToHReadMemoryKB(CString sizeKB);
typedef void (*NotifyFinish)(int type);

class CAndroidFile
{
public:
#define ROOT_INIT_DEPTH		1
#define NORMAL_DEPTH		1

	typedef struct {
		CAndroidFile *file;
		int depth;
	} SUBFILE_PARA_T, *P_SUBFILE_PARA_T;
public:
	int			m_nMode;
	CString		m_strMode;   /* -rwxrwxrwx */
	long		m_lCount;  /*子文件夹及子文件个数*/
	CString		m_strOwner; 
	CString		m_strGroup;		
	long long	m_llSize; /* size in Byte */
	CString		m_strTime;
	CString		m_strName;
	CString		m_strPath;
	CString		m_strLink;

	AND_FILE_TYPE	m_nType;
	CString		m_strParent;
	int			m_nDepth;
	BOOL		m_bExpanded;
	vector<CAndroidFile*> m_vecSubfiles;
	CAndroidFile *m_pParent;
	CString		m_strLine;
	vector<CString> m_vecLinkStack;
public:
	void ParseLsPieces(vector<CString> &pieces);
	void PrintInfo();
	CAndroidFile(CString llLine, int depth, CAndroidFile *pParent) 
		: m_bExpanded(FALSE) 
	{
		Reset();
		m_pParent = pParent;
		this->m_strParent = pParent->m_strPath;
		ParseLlLine(llLine);
		this->m_nDepth = depth+1;
	}
	CAndroidFile(): m_bExpanded(FALSE) {
		Reset();
	}
	void ClearSubfiles() {
		// release memory switch to static functions 
		/* 
		int size = m_vecSubfiles.size();
		for (int i = 0; i < size; i++) {
			delete m_vecSubfiles.at(i);
		}
		*/
		m_vecSubfiles.clear();
	}
	~CAndroidFile() {
		ClearSubfiles();	
	}

	void Reset();
	void Expand(int depth = NORMAL_DEPTH);
	void ParseLlLine(CString llLine);
	BOOL IsDirectory() const { return m_nType == FTYPE_DIR;}
	BOOL IsCurrentDir() const { return IsDirectory() && (m_strName == TEXT("."));}
	BOOL IsParentDir() const { return IsDirectory() && (m_strName == TEXT(".."));}
	BOOL IsLink() const { return !m_strLink.IsEmpty(); }
	BOOL HasLinkStack() const { return m_vecLinkStack.size() > 0;}

private:
	CString ParseToRealPath(CString strPath);
	BOOL AssignParentLinks(CString strLink, CAndroidFile *pDestFile, CAndroidFile *pFile) const;
	BOOL ValueIsNumbers(CString &strValue);
	BOOL IsTotalLine(CString &strLine);
	void InitSubFiles(int depth = NORMAL_DEPTH);
	BOOL LinkPathIsDir(CString path);
	void ParseMode(CString strMode);
	int  ModeToValue(CString strMode);
	BOOL ValueIsDate(CString strValue);
	void GeneratePath();

public:
	static CString TypeToString(int type);
	static set<CAndroidFile*> S_SET_FILES;
	static CCriticalSection S_CS;
	static void AddFile(CAndroidFile *pFile);
	static void ReleaseAllFile();
	static BOOL IsFirstLoadDir(CAndroidFile *pFile);
	static set<CString> FIRST_LOAD_DIRS;
	static void AndroidFileInit();

	static const CString ROOT_DIR;
	static const CString SDCARD_DIR;
	static const CString EXT_SDCARD_DIR;
	static const CString STORAGE_DIR;

};

typedef vector<unsigned> FileIndex;
class CAdbFileManager
{
public:
	CAdbFileManager(void);
	~CAdbFileManager(void);

	void Refresh();
	BOOL IsRoot(CAndroidFile *pFile) const { 
		return !pFile->m_strPath.CompareNoCase(CAndroidFile::ROOT_DIR); 
	}
	CAndroidFile* GetRoot() { return &m_fileRoot; }
	CAndroidFile* GetFile(const FileIndex &indexes, BOOL bForceUpdate = FALSE);

public:
	
private:
	FileIndex m_vecIndex; 
	CAndroidFile m_fileRoot;
};

