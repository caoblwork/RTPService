
// UnitTest_Receive264.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once





#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUnitTest_Receive264App:
// �йش����ʵ�֣������ UnitTest_Receive264.cpp
//

class CUnitTest_Receive264App : public CWinAppEx
{
public:
	CUnitTest_Receive264App();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUnitTest_Receive264App theApp;