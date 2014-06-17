#include "StdAfx.h"
#include "DrawRGB24.h"

CDrawRGB24::CDrawRGB24(void):
	m_bInit(false),
	m_lpBmpInfo(NULL),
	_hBm1(NULL),
	_hdc_buffer1(NULL),
	_hBm2(NULL),
	_hdc_buffer2(NULL)
{
}

CDrawRGB24::~CDrawRGB24(void)
{
	if(m_lpBmpInfo)
		delete m_lpBmpInfo;
}


void CDrawRGB24::SetVertial()
{
	if(m_lpBmpInfo!=NULL)
		m_lpBmpInfo->bmiHeader.biHeight = 0-m_lpBmpInfo->bmiHeader.biHeight;
}


void CDrawRGB24::CreateDoubleBuffer(HDC hdc1, int cxClient1 ,int cyClient1,HDC hdc2,int cxClient2,int cyClient2)
{
	//创建虚拟位图
	if(hdc1!=NULL)
	{
		_hBm1 = CreateCompatibleBitmap(hdc1,cxClient1,cyClient1);     
		//创建和hdc兼容的设备
		_hdc_buffer1 = CreateCompatibleDC(hdc1);                                    
		SelectObject(_hdc_buffer1,_hBm1);		
	}
	if(hdc2!=NULL)
	{
		_hBm2 = CreateCompatibleBitmap(hdc2,cxClient2,cyClient2);     
		//创建和hdc兼容的设备
		_hdc_buffer1 = CreateCompatibleDC(hdc2);                                    
		SelectObject(_hdc_buffer2,_hBm2);		
	}
}






void CDrawRGB24::Draw2(HWND hWnd, HWND hWnd2,unsigned char * buffer, int SrcW, int SrcH)
{
	HDC hDCDst1 = NULL;
	HDC hDCDst2 = NULL;
	RECT destRect1;
	RECT destRect2;
	if(hWnd!=NULL)
	{
		hDCDst1 = GetDC(hWnd);
		GetClientRect(hWnd,&destRect1);
	}
	if(hWnd2!=NULL)
	{
		hDCDst2 = GetDC(hWnd2);
		GetClientRect(hWnd2,&destRect2);
	}

	if(!m_bInit)
	{
		m_bInit = true;
		m_lpBmpInfo=new BITMAPINFO;
		m_lpBmpInfo->bmiHeader.biSize  = sizeof(BITMAPINFOHEADER);
		m_lpBmpInfo->bmiHeader.biWidth =   SrcW;
		m_lpBmpInfo->bmiHeader.biHeight=   -SrcH;
		m_lpBmpInfo->bmiHeader.biPlanes= 1;
		m_lpBmpInfo->bmiHeader.biBitCount      = 24;
		m_lpBmpInfo->bmiHeader.biCompression   = 0;
		m_lpBmpInfo->bmiHeader.biSizeImage     = 0;
		m_lpBmpInfo->bmiHeader.biXPelsPerMeter = 0;
		m_lpBmpInfo->bmiHeader.biYPelsPerMeter = 0;
		m_lpBmpInfo->bmiHeader.biClrUsed=0;
		m_lpBmpInfo->bmiHeader.biClrImportant  = 0;

		//CDC * dc =  CDC::FromHandle(hDCDst);
		//m_pMemDC = new CMemDC(*dc,DestRect);
	}

	if(hDCDst1!=NULL)
	{
		int DstWidth  = destRect1.right-destRect1.left;
		int DstHeight = destRect1.bottom- destRect1.top;
		SetStretchBltMode(hDCDst1,STRETCH_HALFTONE);
		::StretchDIBits(
			//m_pMemDC->GetDC().GetSafeHdc(),
			hDCDst1,
			0, 0, DstWidth, DstHeight,
			0, 0, SrcW, SrcH,
			buffer, m_lpBmpInfo, DIB_RGB_COLORS, SRCCOPY );
		ReleaseDC(hWnd,hDCDst1);
	}
	if(hDCDst2!=NULL)
	{
		int DstWidth  = destRect2.right-destRect2.left;
		int DstHeight = destRect2.bottom- destRect2.top;
		SetStretchBltMode(hDCDst2,STRETCH_HALFTONE);
		::StretchDIBits(
			//m_pMemDC->GetDC().GetSafeHdc(),
			hDCDst2,
			0, 0, DstWidth, DstHeight,
			0, 0, SrcW, SrcH,
			buffer, m_lpBmpInfo, DIB_RGB_COLORS, SRCCOPY );
		ReleaseDC(hWnd2,hDCDst2);
	}

}






#if 0


#include <stdio.h>
#include "ddraw.h"       // DirectDraw interfaces
#include "mmstream.h"    // multimedia stream interfaces
#include "amstream.h"    // DirectShow multimedia stream interfaces
#include "ddstream.h"    // DirectDraw multimedia stream interfaces

HRESULT RenderStreamToSurface(IDirectDrawSurface *pSurface, 
    IMultiMediaStream *pMMStream)
{    
    IMediaStream *pPrimaryVidStream;    
    IDirectDrawMediaStream *pDDStream;
    IDirectDrawStreamSample *pSample;
    RECT rect;
    DDSURFACEDESC ddsd;

    HRESULT hr;
    hr = pMMStream->GetMediaStream(MSPID_PrimaryVideo, &pPrimaryVidStream);
    if (FAILED(hr))
    {
        return hr;
    }
    pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&pDDStream);

    ddsd.dwSize = sizeof(ddsd);
    hr = pDDStream->GetFormat(&ddsd, NULL, NULL, NULL);
    if (SUCCEEDED(hr))
    {
        rect.top = rect.left = 0;
        rect.bottom = ddsd.dwHeight;
        rect.right = ddsd.dwWidth;
        hr = pDDStream->CreateSample(pSurface, &rect, 0, &pSample);
        if (SUCCEEDED(hr))
        {
            pMMStream->SetState(STREAMSTATE_RUN);
            while (pSample->Update(0, NULL, NULL, NULL) == S_OK)
            {
                // Empty loop.
            }
            pMMStream->SetState(STREAMSTATE_STOP);
            pSample->Release();    
        }
    }
    pDDStream->Release();
    pPrimaryVidStream->Release();
    return hr;
}

HRESULT RenderFileToMMStream(
    const char * szFileName, 
    IMultiMediaStream **ppMMStream,
    IDirectDraw *pDD)
{
    if (strlen(szFileName) > MAX_PATH)
    {
        return E_INVALIDARG;
    }

    IAMMultiMediaStream *pAMStream;
    HRESULT hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, 
        CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, 
        (void **)&pAMStream);
    if (FAILED(hr)
    {
        return hr;
    }

    WCHAR wPath[MAX_PATH + 1];
    MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wPath, MAX_PATH + 1);

    pAMStream->Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, NULL);
    pAMStream->AddMediaStream(pDD, &MSPID_PrimaryVideo, 0, NULL);
    pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL);
    hr = pAMStream->OpenFile(wPath, 0);
    if (SUCCEEDED(hr))
    {
        hr = pAMStream->QueryInterface(IID_IMultiMediaStream, 
            (void**)ppMMStream);
    }
    pAMStream->Release();
    return hr;
}

int __cdecl main(int argc, char *argv[])    
{    
    if (argc < 2) 
    {
        printf("Usage : showstrm movie.ext\n");
        exit(0);
    }    

    DDSURFACEDESC ddsd;
    IDirectDraw *pDD;    
    IDirectDrawSurface *pPrimarySurface;
    IMultiMediaStream *pMMStream;

    CoInitialize(NULL);

    DirectDrawCreate(NULL, &pDD, NULL);
    pDD->SetCooperativeLevel(GetDesktopWindow(), DDSCL_NORMAL);

    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    pDD->CreateSurface(&ddsd, &pPrimarySurface, NULL);

    HRESULT hr = RenderFileToMMStream(argv[1], &pMMStream, pDD);
    if (SUCCEEDED(hr))
    {
        RenderStreamToSurface(pPrimarySurface, pMMStream);    
        pMMStream->Release();
    }
    pPrimarySurface->Release();    
    pDD->Release(); 
    
    CoUninitialize();
    return 0;
}
#endif


#if 0
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <amstream.h>

class CWaveBuffer {
    public:
        ~CWaveBuffer();
        BOOL Init(HWAVEOUT hWave, int Size);
        BOOL Write(PBYTE pData, int nBytes, int& BytesWritten);
        void Flush();
    private:
        WAVEHDR      m_Hdr;
        HWAVEOUT     m_hWave;
        int          m_nBytes;
};

class CWaveOut {
    public:
        CWaveOut(LPCWAVEFORMATEX Format, int nBuffers, int BufferSize);
        ~CWaveOut();
        void Write(PBYTE Data, int nBytes);
        void Flush();
        void Wait();
        void Reset();
    private:
        const HANDLE   m_hSem;
        const int      m_nBuffers;
        int            m_CurrentBuffer;
        BOOL           m_NoBuffer;
        CWaveBuffer   *m_Hdrs;
        HWAVEOUT       m_hWave;
};

BOOL CWaveBuffer::Init(HWAVEOUT hWave, int Size)
{
    m_hWave  = hWave;
    m_nBytes = 0;

    /*  Allocate a buffer and initialize the header. */
    m_Hdr.lpData = (LPSTR)LocalAlloc(LMEM_FIXED, Size);
    if (m_Hdr.lpData == NULL) 
    {
        return FALSE;
    }
    m_Hdr.dwBufferLength  = Size;
    m_Hdr.dwBytesRecorded = 0;
    m_Hdr.dwUser = 0;
    m_Hdr.dwFlags = 0;
    m_Hdr.dwLoops = 0;
    m_Hdr.lpNext = 0;
    m_Hdr.reserved = 0;

    /*  Prepare it. */
    waveOutPrepareHeader(hWave, &m_Hdr, sizeof(WAVEHDR));
    return TRUE;
}

CWaveBuffer::~CWaveBuffer() 
{
    if (m_Hdr.lpData) 
    {
        waveOutUnprepareHeader(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        LocalFree(m_Hdr.lpData);
    }
}

void CWaveBuffer::Flush()
{
    // ASSERT(m_nBytes != 0);
    m_nBytes = 0;
    waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
}

BOOL CWaveBuffer::Write(PBYTE pData, int nBytes, int& BytesWritten)
{
    // ASSERT((DWORD)m_nBytes != m_Hdr.dwBufferLength);
    BytesWritten = min((int)m_Hdr.dwBufferLength - m_nBytes, nBytes);
    CopyMemory((PVOID)(m_Hdr.lpData + m_nBytes), (PVOID)pData, BytesWritten);
    m_nBytes += BytesWritten;
    if (m_nBytes == (int)m_Hdr.dwBufferLength) 
    {
        /*  Write it! */
        m_nBytes = 0;
        waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        return TRUE;
    }
    return FALSE;
}

void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, 
                           DWORD dw1, DWORD dw2)
{
    if (uMsg == WOM_DONE) 
    {
        ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
    }
}

CWaveOut::CWaveOut(LPCWAVEFORMATEX Format, int nBuffers, int BufferSize) :
    m_nBuffers(nBuffers),
    m_CurrentBuffer(0),
    m_NoBuffer(TRUE),
    m_hSem(CreateSemaphore(NULL, nBuffers, nBuffers, NULL)),
    m_Hdrs(new CWaveBuffer[nBuffers]),
    m_hWave(NULL)
{
    /*  Create wave device. */
    waveOutOpen(&m_hWave,
                WAVE_MAPPER,
                Format,
                (DWORD)WaveCallback,
                (DWORD)m_hSem,
                CALLBACK_FUNCTION);

    /*  Initialize the wave buffers. */
    for (int i = 0; i < nBuffers; i++) 
    {
        m_Hdrs[i].Init(m_hWave, BufferSize);
    }
}

CWaveOut::~CWaveOut()
{
    /*  First, get the buffers back. */
    waveOutReset(m_hWave);
    /*  Free the buffers. */
    delete [] m_Hdrs;
    /*  Close the wave device. */
    waveOutClose(m_hWave);
    /*  Free the semaphore. */
    CloseHandle(m_hSem);
}

void CWaveOut::Flush()
{
    if (!m_NoBuffer) 
    {
        m_Hdrs[m_CurrentBuffer].Flush();
        m_NoBuffer = TRUE;
        m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
    }
}

void CWaveOut::Reset()
{
    waveOutReset(m_hWave);
}

void CWaveOut::Write(PBYTE pData, int nBytes)
{
    while (nBytes != 0) 
    {
        /*  Get a buffer if necessary. */
        if (m_NoBuffer) 
        {
            WaitForSingleObject(m_hSem, INFINITE);
            m_NoBuffer = FALSE;
        }

        /*  Write into a buffer. */
        int nWritten;
        if (m_Hdrs[m_CurrentBuffer].Write(pData, nBytes, nWritten)) 
        {
            m_NoBuffer = TRUE;
            m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
            nBytes -= nWritten;
            pData += nWritten;
        } 
        else 
        {
            // ASSERT(nWritten == nBytes);
            break;
        }
    }
}

void CWaveOut::Wait()
{
    /*  Send any remaining buffers. */
    Flush();
    /*  Wait for the buffers back. */
    for (int i = 0; i < m_nBuffers; i++) 
    {
        WaitForSingleObject(m_hSem, INFINITE);
    }
    LONG lPrevCount;
    ReleaseSemaphore(m_hSem, m_nBuffers, &lPrevCount);
}

HRESULT RenderStreamToDevice(IMultiMediaStream *pMMStream)
{
    WAVEFORMATEX wfx;
    #define DATA_SIZE 5000

    IMediaStream        *pStream = NULL;
    IAudioStreamSample  *pSample = NULL;
    IAudioMediaStream   *pAudioStream = NULL;
    IAudioData          *pAudioData = NULL;

    HRESULT hr = pMMStream->GetMediaStream(MSPID_PrimaryAudio, &pStream);
    if (FAILED(hr))
    {
        return hr;
    }

    pStream->QueryInterface(IID_IAudioMediaStream, (void **)&pAudioStream);
    pStream->Release();

    hr = CoCreateInstance(CLSID_AMAudioData, NULL, 
        CLSCTX_INPROC_SERVER, IID_IAudioData, (void **)&pAudioData);
    if (FAILED(hr))
    {
        pAudioStream->Release();
        return hr;
    }

    PBYTE pBuffer = (PBYTE)LocalAlloc(LMEM_FIXED, DATA_SIZE);
    if (pBuffer == NULL)
    {
        pAudioStream->Release();
        pAudioData->Release();
        return E_OUTOFMEMORY;
    }

    pAudioStream->GetFormat(&wfx);
    pAudioData->SetBuffer(DATA_SIZE, pBuffer, 0);
    pAudioData->SetFormat(&wfx);
    hr = pAudioStream->CreateSample(pAudioData, 0, &pSample);
    pAudioStream->Release();
    if (FAILED(hr))
    {
        LocalFree((HLOCAL)pBuffer);
        pAudioData->Release();
        pSample->Release();
        return hr;
    }

    CWaveOut WaveOut(&wfx, 4, 2048);
    HANDLE hEvent = CreateEvent(FALSE, NULL, NULL, FALSE);
    if (hEvent != 0)
    {
        int iTimes;
        for (iTimes = 0; iTimes < 3; iTimes++) 
        {
            DWORD dwStart = timeGetTime();
            for (; ; ) 
            {
                hr = pSample->Update(0, hEvent, NULL, 0);
                if (FAILED(hr) || hr == MS_S_ENDOFSTREAM) 
                {
                    break;
                }
                WaitForSingleObject(hEvent, INFINITE);
                DWORD dwTimeDiff = timeGetTime() - dwStart;
                // Limit to 10 seconds
                if (dwTimeDiff > 10000) {
                    break;
                }
                DWORD dwLength;
                pAudioData->GetInfo(NULL, NULL, &dwLength);
                WaveOut.Write(pBuffer, dwLength);
            }
            pMMStream->Seek(0);
        }
    }

    pAudioData->Release();
    pSample->Release();
    LocalFree((HLOCAL)pBuffer);
    return S_OK;
}

HRESULT RenderFileToMMStream(
    const char * szFileName, 
    IMultiMediaStream **ppMMStream)
{
    if (strlen(szFileName) > MAX_PATH)
    {
        return E_INVALIDARG;
    }

    IAMMultiMediaStream *pAMStream;
    HRESULT hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, 
        CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, 
        (void **)&pAMStream);
    if (FAILED(hr))
    { 
        return hr;
    }

    WCHAR wszName[MAX_PATH + 1];
    MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wszName,
        MAX_PATH + 1);
    
    pAMStream->Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, NULL);
    pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, 0, NULL);
    hr = pAMStream->OpenFile(wszName, AMMSF_RUN);
    {
        if (SUCCEEDED(hr))
        {
            hr = pAMStream->QueryInterface(IID_IMultiMediaStream, 
                (void**)ppMMStream);
        }
    }
    pAMStream->Release();
    return hr;
}

int __cdecl main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Specify a file name.\n");
        exit(0);
    }

    IMultiMediaStream *pMMStream;
    CoInitialize(NULL);
    HRESULT hr = RenderFileToMMStream(argv[1], &pMMStream);
    if (SUCCEEDED(hr))
    {
        RenderStreamToDevice(pMMStream);
        pMMStream->Release();
    }
    
    CoUninitialize();
    return 0;
}



#endif


#if 0 //hdc 保存文件


BOOL WriteBmp(const TSTRING &strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg);   
BOOL WriteBmp(const TSTRING &strFile,HDC hdc);   
BOOL WriteBmp(const TSTRING &strFile,HDC hdc,const RECT &rcDC);   
  
BOOL WriteBmp(const TSTRING &strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg)    
{      
  
    BITMAPINFOHEADER bmInfoHeader = {0};   
    bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);   
    bmInfoHeader.biWidth = sizeImg.cx;   
    bmInfoHeader.biHeight = sizeImg.cy;   
    bmInfoHeader.biPlanes = 1;   
    bmInfoHeader.biBitCount = 24;   
  
    //Bimap file header in order to write bmp file   
    BITMAPFILEHEADER bmFileHeader = {0};   
    bmFileHeader.bfType = 0x4d42;  //bmp     
    bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);   
    bmFileHeader.bfSize = bmFileHeader.bfOffBits + ((bmInfoHeader.biWidth * bmInfoHeader.biHeight) * 3); ///3=(24 / 8)   
  
    HANDLE hFile = CreateFile(strFile.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);   
    if(hFile == INVALID_HANDLE_VALUE)   
    {   
        return FALSE;   
    }   
  
    DWORD dwWrite = 0;   
    WriteFile(hFile,&bmFileHeader,sizeof(BITMAPFILEHEADER),&dwWrite,NULL);   
    WriteFile(hFile,&bmInfoHeader, sizeof(BITMAPINFOHEADER),&dwWrite,NULL);   
    WriteFile(hFile,&vtData[0], vtData.size(),&dwWrite,NULL);   
  
  
    CloseHandle(hFile);   
  
    return TRUE;   
}    
  
  
BOOL WriteBmp(const TSTRING &strFile,HDC hdc)   
{   
    int iWidth = GetDeviceCaps(hdc,HORZRES);   
    int iHeight = GetDeviceCaps(hdc,VERTRES);   
    RECT rcDC = {0,0,iWidth,iHeight};   
  
    return WriteBmp(strFile,hdc,rcDC);     
}   
  
BOOL WriteBmp(const TSTRING &strFile,HDC hdc,const RECT &rcDC)   
{   
    BOOL bRes = FALSE;   
    BITMAPINFO bmpInfo = {0};   
    BYTE *pData = NULL;   
    SIZE sizeImg = {0};   
    HBITMAP hBmp = NULL;   
    std::vector<BYTE> vtData;   
    HGDIOBJ hOldObj = NULL;   
    HDC hdcMem = NULL;   
  
    //Initilaize the bitmap information    
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);   
    bmpInfo.bmiHeader.biWidth = rcDC.right - rcDC.left;   
    bmpInfo.bmiHeader.biHeight = rcDC.bottom - rcDC.top;   
    bmpInfo.bmiHeader.biPlanes = 1;   
    bmpInfo.bmiHeader.biBitCount = 24;   
  
    //Create the compatible DC to get the data   
    hdcMem = CreateCompatibleDC(hdc);   
    if(hdcMem == NULL)   
    {   
        goto EXIT;   
    }   
  
    //Get the data from the memory DC      
    hBmp = CreateDIBSection(hdcMem,&bmpInfo,DIB_RGB_COLORS,reinterpret_cast<VOID **>(&pData),NULL,0);   
    if(hBmp == NULL)   
    {   
        goto EXIT;   
    }   
    hOldObj = SelectObject(hdcMem, hBmp);   
       
    //Draw to the memory DC   
    sizeImg.cx = bmpInfo.bmiHeader.biWidth;   
    sizeImg.cy = bmpInfo.bmiHeader.biHeight;   
    StretchBlt(hdcMem,   
                0,   
                0,   
                sizeImg.cx,   
                sizeImg.cy,   
                hdc,   
                rcDC.left,   
                rcDC.top,   
                rcDC.right - rcDC.left + 1,   
                rcDC.bottom - rcDC.top + 1,   
                SRCCOPY);   
       
  
    vtData.resize(sizeImg.cx * sizeImg.cy * 3);   
    memcpy(&vtData[0],pData,vtData.size());   
    bRes = WriteBmp(strFile,vtData,sizeImg);   
  
    SelectObject(hdcMem, hOldObj);   
       
  
EXIT:   
    if(hBmp != NULL)   
    {   
        DeleteObject(hBmp);   
    }   
  
    if(hdcMem != NULL)   
    {   
        DeleteDC(hdcMem);   
    }   
  
    return bRes;   
}  


#endif 