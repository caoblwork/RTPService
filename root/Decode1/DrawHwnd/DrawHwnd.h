
// DrawHwnd.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDrawHwndApp:
// �йش����ʵ�֣������ DrawHwnd.cpp
//

class CDrawHwndApp : public CWinApp
{
public:
	CDrawHwndApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDrawHwndApp theApp;