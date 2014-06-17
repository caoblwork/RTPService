#pragma once
#include <windows.h>
#include <mmsystem.h>
//#include "vfw.h"
//#pragma comment(lib,"vfw32")



class CDrawRGB24
{
public:
	CDrawRGB24(void);
	~CDrawRGB24(void);
protected:
	void Initialize(int SrcW,int SrcH);
//	void Initialize1280_720();
#if _MFC_DEFINED_
	//创建双缓冲
	void CreateDoubleBuffer(CDC& DC,HWND hWnd);
#endif

    void CreateDoubleBuffer(HDC hdc1, int cxClient1 ,int cyClient1,HDC hdc2,int cxClient2,int cyClient2);


public:
//Draw方法已经被废弃，但是可用，图像倒立
	//void Draw(HWND hWnd,unsigned char * buffer,int SrcW,int SrcH);

//画原始图像，图像正立
#if _MFC_DEFINED_
	void DrawOriginal(HWND hWnd, unsigned char * buffer, int SrcW,int SrcH);
#endif


    void Draw2(HWND hWnd,HWND hWnd2, unsigned char * buffer, int SrcW, int SrcH);

	//void DrawPInP(HWND hWnd 
public:
//图像翻转
	void SetVertial();
private:
	LPBITMAPINFO m_lpBmpInfo;
	
//	HDRAWDIB m_hDIB;
	
//	HWND     m_hWnd;
	bool     m_bInit;

#if _MFC_DEFINED_
	//大背景画板
	CMemDC   *m_pMemDC;
#endif
    HBITMAP _hBm1;
	HDC _hdc_buffer1;
	HBITMAP _hBm2;
	HDC _hdc_buffer2;

};
