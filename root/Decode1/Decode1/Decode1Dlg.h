
// Decode1Dlg.h : ͷ�ļ�
//

#pragma once

#include "../librtprece/CodeReceive3.h"
#include "decode.h"
// CDecode1Dlg �Ի���
class CDecode1Dlg : public CDialogEx
{
// ����


private:
	CodeReceive3 _receive;

public:
	//void Draw(AVFrame* frame,int pix_fmt,int srcW,int srcH);
public:
	CDecode1Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DECODE1_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();


};
