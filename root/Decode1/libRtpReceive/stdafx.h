// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ



#include <windows.h>
#ifdef _DEBUG 
 
#define DP0(fmt) {char sOut[256];sprintf(sOut,(fmt));OutputDebugString(sOut);} 
#define DP1(fmt,var) {char sOut[256];sprintf(sOut,(fmt),var);OutputDebugString(sOut);} 
#define DP2(fmt,var1,var2) {char sOut[256];sprintf(sOut,(fmt),var1,var2);OutputDebugString(sOut);} 
#define DP3(fmt,var1,var2,var3) {char sOut[256];sprintf(sOut,(fmt),var1,var2,var3);OutputDebugString(sOut);} 
 
#endif 
 
#ifndef _DEBUG 
 
#define DP0(fmt) ; 
#define DP1(fmt, var) ; 
#define DP2(fmt,var1,var2) ; 
#define DP3(fmt,var1,var2,var3) ; 
 
#endif 
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
