#pragma once
#include <windows.h>
#include <mmsystem.h>

class CDrawRGB24
{
public:
	CDrawRGB24(void);
	~CDrawRGB24(void);
protected:



    void CreateDoubleBuffer(HDC hdc1, int cxClient1 ,int cyClient1,HDC hdc2,int cxClient2,int cyClient2);


public:


    void Draw2(HWND hWnd,HWND hWnd2, unsigned char * buffer, int SrcW, int SrcH);

	//void DrawPInP(HWND hWnd 
public:
//Í¼Ïñ·­×ª
	void SetVertial();
private:
	LPBITMAPINFO m_lpBmpInfo;
	
	bool     m_bInit;

    HBITMAP _hBm1;
	HDC _hdc_buffer1;
	HBITMAP _hBm2;
	HDC _hdc_buffer2;

};
