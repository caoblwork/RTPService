//////////////////////////////////////////////////////////////////////
// Video Capture using DirectShow
// Author: qianbo (418511899.@qq.com)
//////////////////////////////////////////////////////////////////////
// 使用说明：
//1. 将DSCamera.h DSCamera.cpp复制到你的项目中
//////////////////////////////////////////////////////////////////////
// CameraDS.cpp: implementation of the CCameraDS class.
//2013-3-18 日 钱波修改最后一次
//
//////////////////////////////////////////////////////////////////////
#include <stdafx.h>
#include "DSCamera.h"
#include <wmsdkidl.h>
#pragma comment(lib,"Strmiids.lib") 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CCameraDS::CCameraDS()
{
	m_bConnected = false;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bLock = false;
	m_bChanged = false;
	m_pFrame = NULL;
	m_nBufferSize = 0;

	m_pNullFilter = NULL;
	m_pMediaEvent = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pGraph = NULL;
	_IsI420 = false;
	YUVBuffer = NULL;
	CoInitialize(NULL);
}

CCameraDS::~CCameraDS()
{
	CloseCamera();
	CoUninitialize();
	if(YUVBuffer!=NULL)
		delete []YUVBuffer;
}

void CCameraDS::CloseCamera()
{
	if(m_bConnected)
		m_pMediaControl->Stop();

	m_pGraph = NULL;
	m_pDeviceFilter = NULL;
	m_pMediaControl = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pSampleGrabber = NULL;
	m_pGrabberInput = NULL;
	m_pGrabberOutput = NULL;
	m_pCameraOutput = NULL;
	m_pMediaEvent = NULL;
	m_pNullFilter = NULL;
	m_pNullInputPin = NULL;


	if (m_pFrame)
		delete []m_pFrame;
		//cvReleaseImage(&m_pFrame);

	m_bConnected = false;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bLock = false;
	m_bChanged = false;
	m_nBufferSize = 0;
}






#define FOURCC_I420        mmioFOURCC('I','4','2','0') 
//sample grabber 并不会改变接口采集数据的格式，因此一开始可以不用设置格式

//crossbar 0：web usb camera 1  2：标清接口 3： 高清接口
bool CCameraDS::OpenCameraYUV420(string CameraName_Friend,bool isI420,int nWidth ,int nHeight,int crossbar)
{
	HRESULT hr = S_OK;

	_IsI420 = isI420;
	CoInitialize(NULL);
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **)&m_pGraph);

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, 
		IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pMediaControl);
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **) &m_pMediaEvent);

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (LPVOID*) &m_pNullFilter);


	hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);


	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	//mt.majortype = MEDIATYPE_Video;
	//mt.subtype = MEDIASUBTYPE_RGB24;
	//mt.formattype = FORMAT_VideoInfo; 
	//hr = m_pSampleGrabber->SetMediaType(&mt);
	//MYFREEMEDIATYPE(mt);

	m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");

	int nCamID = 0; //GetCameraIDFromFriendName(CameraName_Friend);
	// Bind Device Filter.  We know the device because the id was passed in
	BindFilter(nCamID, &m_pDeviceFilter);
	m_pGraph->AddFilter(m_pDeviceFilter, NULL);

	CComPtr<IEnumPins> pEnum;
	m_pDeviceFilter->EnumPins(&pEnum);

	hr = pEnum->Reset();
	hr = pEnum->Next(1, &m_pCameraOutput, NULL); 

	pEnum = NULL; 
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pGrabberInput, NULL); 

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	pEnum->Skip(1);
	hr = pEnum->Next(1, &m_pGrabberOutput, NULL); 

	pEnum = NULL;
	m_pNullFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pNullInputPin, NULL);

	SetCrossBar(crossbar);


	int _Width = nWidth, _Height = nHeight;
	IAMStreamConfig*   iconfig; 
	iconfig = NULL;
	hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig,   (void**)&iconfig);   

	

	int iCount = 0, iSize = 0;
	hr = iconfig->GetNumberOfCapabilities(&iCount, &iSize); //得到格式能力

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		//MEDIASUBTYPE_YUV MEDIASUBTYPE_YUV2
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = iconfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER *pVihtemp = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->subtype == WMMEDIASUBTYPE_I420) &&// MEDIASUBTYPE_YV12) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL))

				{
				    
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					// pVih contains the detailed format information.
					int mWidth   = pVih->bmiHeader.biWidth;
					int mHeight  = pVih->bmiHeader.biHeight;
					int compress = pVih->bmiHeader.biCompression;
					int I420 = FOURCC_I420; 
					//int type = pmtConfig->subtype;
					if(compress== I420 && mWidth==nWidth && mHeight == nHeight)
					{
						hr = iconfig->SetFormat(pmtConfig);
						if(FAILED(hr))
						{
							return false;
						}
						//else 
						//	break;
					}
				}
			
				MYFREEMEDIATYPE(*pmtConfig);
				//DeleteMediaType(pmtConfig);
			}

		}//for end
	}//if end


	iconfig->Release();   
	iconfig=NULL;   
//	MYFREEMEDIATYPE(*pmt);

	hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);
	hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);

	if (FAILED(hr))
	{
		switch(hr)
		{
		case VFW_S_NOPREVIEWPIN :
			break;
		case E_FAIL :
			break;
		case E_INVALIDARG :
			break;
		case E_POINTER :
			break;
		}
	}

	m_pSampleGrabber->SetBufferSamples(TRUE);
	m_pSampleGrabber->SetOneShot(TRUE);

	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if(FAILED(hr))
		return false;

	VIDEOINFOHEADER *videoHeader;
	videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	m_nWidth = videoHeader->bmiHeader.biWidth;
	m_nHeight = videoHeader->bmiHeader.biHeight;
	m_bConnected = true;

	pEnum = NULL;
	return true;
}

int CCameraDS::FindCameraByName(string name)
{
	int num = CameraCount();
	char nametmp[128];
	memset(nametmp,0,sizeof(nametmp));
	for(int i =0;i<num;i++)
	{
		CameraName(i,nametmp,sizeof(nametmp));
		if(name.compare(nametmp)==0)
		{
			return i;
		}
		memset(nametmp,0,sizeof(nametmp));
	}
	return -1;
}


bool CCameraDS::OpenCamera(string name,int nWidth, int nHeight,int CrossBar)
{
	int id = FindCameraByName(name);
	if(id==-1)
		return false;
	return OpenCamera(id,nWidth,nHeight);

	return false;
}




bool CCameraDS::OpenCamera(int nCamID,  int nWidth, int nHeight,int CrossBar)
{
	
	HRESULT hr = S_OK;

	CoInitialize(NULL);
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
							IID_IGraphBuilder, (void **)&m_pGraph);

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, 
							IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pMediaControl);
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **) &m_pMediaEvent);

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
							IID_IBaseFilter, (LPVOID*) &m_pNullFilter);


	hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);

	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.formattype = FORMAT_VideoInfo; 
	hr = m_pSampleGrabber->SetMediaType(&mt);
	MYFREEMEDIATYPE(mt);

	m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");
 
	// Bind Device Filter.  We know the device because the id was passed in
	BindFilter(nCamID, &m_pDeviceFilter);
	m_pGraph->AddFilter(m_pDeviceFilter, NULL);

	CComPtr<IEnumPins> pEnum;
	m_pDeviceFilter->EnumPins(&pEnum);
 
	hr = pEnum->Reset();
	hr = pEnum->Next(1, &m_pCameraOutput, NULL); 

	pEnum = NULL; 
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pGrabberInput, NULL); 

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	pEnum->Skip(1);
	hr = pEnum->Next(1, &m_pGrabberOutput, NULL); 

	pEnum = NULL;
	m_pNullFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pNullInputPin, NULL);

	SetCrossBar(2);



	//////////////////////////////////////////////////////////////////////////////
	// 加入由 lWidth和lHeight设置的摄像头的宽和高 的功能，默认720*576
	// by qianbo @2012-08-10
	//////////////////////////////////////////////////////////////////////////////
	int _Width = nWidth, _Height = nHeight;
	IAMStreamConfig*   iconfig; 
	iconfig = NULL;
	hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig,   (void**)&iconfig);   

	AM_MEDIA_TYPE* pmt;    
	if(iconfig->GetFormat(&pmt) !=S_OK) 
	{
		//printf("GetFormat Failed ! \n");
		return   false;   
	}

	VIDEOINFOHEADER*   phead;
	


	if ( pmt->formattype == FORMAT_VideoInfo)   
	{   
		phead=( VIDEOINFOHEADER*)pmt->pbFormat;   
		phead->bmiHeader.biWidth = _Width;   
		phead->bmiHeader.biHeight = _Height;   
		if(( hr=iconfig->SetFormat(pmt)) != S_OK )   
		{
			return   false;
		}

	}   

	iconfig->Release();   
	iconfig=NULL;   
	MYFREEMEDIATYPE(*pmt);
	

	hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);
	hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);

	if (FAILED(hr))
	{
		switch(hr)
		{
			case VFW_S_NOPREVIEWPIN :
				break;
			case E_FAIL :
				break;
			case E_INVALIDARG :
				break;
			case E_POINTER :
				break;
		}
	}

	m_pSampleGrabber->SetBufferSamples(TRUE);
	m_pSampleGrabber->SetOneShot(TRUE);
    
	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if(FAILED(hr))
		return false;

	VIDEOINFOHEADER *videoHeader;
	videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	m_nWidth = videoHeader->bmiHeader.biWidth;
	m_nHeight = videoHeader->bmiHeader.biHeight;
	m_bConnected = true;

	pEnum = NULL;
	return true;
}


bool CCameraDS::BindFilter(int nCamID, IBaseFilter **pFilter)
{
	if (nCamID < 0)
		return false;
 
    // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
								IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
		return false;
	}

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
        &pEm, 0);
    if (hr != NOERROR) 
	{
		return false;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
	int index = 0;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= nCamID)
    {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				if (index == nCamID)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
    }

	pCreateDevEnum = NULL;
	return true;
}

//钱波增加 2012-08-30 增加一个函数得到能采集I420格式的
#if 0
void CCameraDS::ConfigOutputFormat()
{
	IAMStreamConfig *pConfig = NULL;
	HRESULT hr = capture_builder_->FindInterface(
		&PIN_CATEGORY_CAPTURE, // Preview pin.
		0, // Any media type.
		source_filter_, // Pointer to the capture filter.
		IID_IAMStreamConfig, (void**)&pConfig);

	int iCount = 0, iSize = 0;
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		//MEDIASUBTYPE_YUV MEDIASUBTYPE_YUV2
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->subtype == MEDIASUBTYPE_YV12) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL))

				{
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					// pVih contains the detailed format information.
					int mWidth = pVih->bmiHeader.biWidth;
					int mHeight = pVih->bmiHeader.biHeight;

					hr = pConfig->SetFormat(pmtConfig);
					if(FAILED(hr))
					{
						return;
					}
				}
				DeleteMediaType(pmtConfig);
			}

		}//for end
	}//if end
}
#endif
//钱波修改
//cb mean 
//0 webcap
//1 tune
//2 composite
//3 s-video 高清
void CCameraDS::SetCrossBar(int sc)
{
	int i;
	if(sc == 0)//如果是usb摄像头
		return;
	IAMCrossbar *pXBar1 = NULL;
		ICaptureGraphBuilder2 *pBuilder = NULL;


	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, 
		(void **)&pBuilder);

	if (SUCCEEDED(hr))
	{
		hr = pBuilder->SetFiltergraph(m_pGraph); 
	}

	hr = pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, 
		m_pDeviceFilter,IID_IAMCrossbar, (void**)&pXBar1);


	if (SUCCEEDED(hr)) 
	{
		long OutputPinCount;
		long InputPinCount;
		long PinIndexRelated;
		long PhysicalType;
		long inPort = 0;
		long outPort = 0;

		long phtype = PhysConn_Video_Composite;
		if(sc ==2)
			phtype = PhysConn_Video_Composite;
		else if(sc ==3)
			phtype = PhysConn_Video_SVideo; // 高清输入 有可能是HDMI


		pXBar1->get_PinCounts(&OutputPinCount,&InputPinCount);
		for( i =0;i<InputPinCount;i++)
		{
			pXBar1->get_CrossbarPinInfo(TRUE,i,&PinIndexRelated,&PhysicalType);
			if(phtype==PhysicalType) 
			{
				inPort = i;
				break;
			}
		}
		for( i =0;i<OutputPinCount;i++)
		{
			pXBar1->get_CrossbarPinInfo(FALSE,i,&PinIndexRelated,&PhysicalType);
			if(PhysConn_Video_VideoDecoder==PhysicalType) 
			{
				outPort = i;
				break;
			}
		}

		if(S_OK==pXBar1->CanRoute(outPort,inPort))
		{
			pXBar1->Route(outPort,inPort);
		}
		pXBar1->Release();  
	}
}






/*
The returned image can not be released.
*/
#ifdef NEED_OPENCV
IplImage * CCameraDS::QueryFrameYUV()
#else
   char * CCameraDS::QueryFrameYUV()
#endif
{
	long evCode;
	long size = 0;

	m_pMediaControl->Run();
	m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);

	m_pSampleGrabber->GetCurrentBuffer(&size, NULL);

	//if the buffer size changed
	if (size != m_nBufferSize)
	{
		if (m_pFrame)
#ifdef NEED_OPENCV
			cvReleaseImage(&m_pFrame);
#else
			delete []m_pFrame;
#endif

		m_nBufferSize = size;
#ifdef NEED_OPENCV
		m_pFrame = cvCreateImage(cvSize(m_nWidth, m_nHeight),IPL_DEPTH_8U,1);
#else
		m_pFrame = new char[m_nWidth*m_nHeight*3];
#endif
		if(YUVBuffer!=NULL)
			delete []YUVBuffer;
		YUVBuffer = new char[size];
	}
	long ysize = m_nWidth*m_nHeight;
		//灰度图像
	//345600
	m_pSampleGrabber->GetCurrentBuffer((long*)YUVBuffer,&m_nBufferSize);


   	//memcpy(m_pFrame->imageData,YUVBuffer,ysize);
	//cvFlip(m_pFrame);
	//Sleep(20);
	return m_pFrame;
}


char* CCameraDS::QueryFrame(long& len)
//IplImage* CCameraDS::QueryFrame()
{

	long evCode;
	long size = 0;

	m_pMediaControl->Run();
	m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);
 
	m_pSampleGrabber->GetCurrentBuffer(&size, NULL);

	//if the buffer size changed
	if (size != m_nBufferSize)
	{
		if (m_pFrame)
		{
			delete []m_pFrame;
			m_pFrame = NULL;
		}
		m_nBufferSize = size;
		m_pFrame = new char[m_nWidth*m_nHeight*3];
	}
		//灰度图像
	HRESULT s = m_pSampleGrabber->GetCurrentBuffer(&m_nBufferSize, (long*)m_pFrame);//  (long*)m_pFrame->imageData);
	if(s!=S_OK)
	{
		len = 0;
		return NULL;
	}
	len = m_nBufferSize;
	//cvFlip(m_pFrame);
    //Sleep(20);

	return m_pFrame;
}

int CCameraDS::CameraCount()
{

	int count = 0;
 	CoInitialize(NULL);

   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
        &pEm, 0);
    if (hr != NOERROR) 
	{
		return count;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		count++;
    }

	pCreateDevEnum = NULL;
	pEm = NULL;
	return count;
}

int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize)
{
	int count = 0;
 	CoInitialize(NULL);

   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
        &pEm, 0);
    if (hr != NOERROR) return 0;


    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		if (count == nCamID)
		{
			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
	            if(hr == NOERROR)
		        {
			        //获取设备名称			
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,sName, nBufferSize ,"",NULL);

	                SysFreeString(var.bstrVal);				
		        }
			    pBag->Release();
			}
			pM->Release();

			break;
		}
		count++;
    }

	pCreateDevEnum = NULL;
	pEm = NULL;

	return 1;
}




