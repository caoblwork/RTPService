//////////////////////////////////////////////////////////////////////
// Video Capture using DirectShow
// Author: qianbo (418511899@qq.com)


#ifndef CCAMERA_H
#define CCAMERA_H



#include <atlbase.h>
#pragma include_alias( "dxtrans.h", "qedit.h" )

#define __IDxtCompositor_INTERFACE_DEFINED__

#define __IDxtAlphaSetter_INTERFACE_DEFINED__

#define __IDxtJpeg_INTERFACE_DEFINED__

#define __IDxtKey_INTERFACE_DEFINED__
#include "qedit.h"
#include "dshow.h"
#include <windows.h>
//#include "DS_VideoInputDevices.h"
#include <string>
using namespace std;

#define MYFREEMEDIATYPE(mt)	{if ((mt).cbFormat != 0)		\
					{CoTaskMemFree((PVOID)(mt).pbFormat);	\
					(mt).cbFormat = 0;						\
					(mt).pbFormat = NULL;					\
				}											\
				if ((mt).pUnk != NULL)						\
				{											\
					(mt).pUnk->Release();					\
					(mt).pUnk = NULL;						\
				}}									


class CCameraDS  
{
private:
	//如果使用opencv
    //IplImage * m_pFrame;
	//不使用opencv
	char * m_pFrame;
	bool m_bConnected;
	int m_nWidth;
	int m_nHeight;
	bool m_bLock;
	bool m_bChanged;
	long m_nBufferSize;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<IBaseFilter> m_pDeviceFilter;
	CComPtr<IMediaControl> m_pMediaControl;
	CComPtr<IBaseFilter> m_pSampleGrabberFilter;
	CComPtr<ISampleGrabber> m_pSampleGrabber;
	CComPtr<IPin> m_pGrabberInput;
	CComPtr<IPin> m_pGrabberOutput;
	CComPtr<IPin> m_pCameraOutput;
	CComPtr<IMediaEvent> m_pMediaEvent;
	CComPtr<IBaseFilter> m_pNullFilter;
	CComPtr<IPin> m_pNullInputPin;

	int FindCameraByName(string name);

private:
	bool BindFilter(int nCamIDX, IBaseFilter **pFilter);

	//钱波增加 根据
	void SetCrossBar(int sc);

	bool _IsI420;
public:
	CCameraDS();
	virtual ~CCameraDS();

	//打开摄像头，nCamID指定打开哪个摄像头，取值可以为0,1,2,...
	//nWidth和nHeight设置的摄像头的宽和高，如果摄像头不支持所设定的宽度和高度，则返回false
	bool OpenCamera(int nCamID, int nWidth=352, int nHeight=288,int CrossBar = 2);
	bool OpenCamera(string name,int nWidth=352, int nHeight=288,int CrossBar = 2);
	bool OpenCameraYUV420(string CameraName_Friend,bool isI420,int nWidth = 352,int nHeight=288,int CrossBar=2);
	//关闭摄像头，析构函数会自动调用这个函数
	void CloseCamera();

	//返回摄像头的数目
	//可以不用创建CCameraDS实例，采用int c=CCameraDS::CameraCount();得到结果。
	static int CameraCount(); 

	//根据摄像头的编号返回摄像头的名字
	//nCamID: 摄像头编号
	//sName: 用于存放摄像头名字的数组
	//nBufferSize: sName的大小
	//可以不用创建CCameraDS实例，采用CCameraDS::CameraName();得到结果。
	static int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize);

	//返回图像宽度
	int GetWidth(){return m_nWidth;} 

	//返回图像高度
	int GetHeight(){return m_nHeight;}

	char * QueryFrame(long& len);
	char * QueryFrameYUV();
	void ConfigOutputFormat();
	char * YUVBuffer;
};

#endif 
