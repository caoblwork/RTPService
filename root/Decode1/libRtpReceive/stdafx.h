// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息



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
// TODO: 在此处引用程序需要的其他头文件
