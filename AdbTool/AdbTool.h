
// AdbTool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

extern HWND g_hMainWnd;
// CAdbToolApp:
// �йش����ʵ�֣������ AdbTool.cpp
//

class CAdbToolApp : public CWinApp
{
public:
	CAdbToolApp();
	~CAdbToolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAdbToolApp theApp;