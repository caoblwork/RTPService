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
	//���ʹ��opencv
    //IplImage * m_pFrame;
	//��ʹ��opencv
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

	//Ǯ������ ����
	void SetCrossBar(int sc);

	bool _IsI420;
public:
	CCameraDS();
	virtual ~CCameraDS();

	//������ͷ��nCamIDָ�����ĸ�����ͷ��ȡֵ����Ϊ0,1,2,...
	//nWidth��nHeight���õ�����ͷ�Ŀ�͸ߣ��������ͷ��֧�����趨�Ŀ�Ⱥ͸߶ȣ��򷵻�false
	bool OpenCamera(int nCamID, int nWidth=352, int nHeight=288,int CrossBar = 2);
	bool OpenCamera(string name,int nWidth=352, int nHeight=288,int CrossBar = 2);
	bool OpenCameraYUV420(string CameraName_Friend,bool isI420,int nWidth = 352,int nHeight=288,int CrossBar=2);
	//�ر�����ͷ�������������Զ������������
	void CloseCamera();

	//��������ͷ����Ŀ
	//���Բ��ô���CCameraDSʵ��������int c=CCameraDS::CameraCount();�õ������
	static int CameraCount(); 

	//��������ͷ�ı�ŷ�������ͷ������
	//nCamID: ����ͷ���
	//sName: ���ڴ������ͷ���ֵ�����
	//nBufferSize: sName�Ĵ�С
	//���Բ��ô���CCameraDSʵ��������CCameraDS::CameraName();�õ������
	static int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize);

	//����ͼ����
	int GetWidth(){return m_nWidth;} 

	//����ͼ��߶�
	int GetHeight(){return m_nHeight;}

	char * QueryFrame(long& len);
	char * QueryFrameYUV();
	void ConfigOutputFormat();
	char * YUVBuffer;
};

#endif 
