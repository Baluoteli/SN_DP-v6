// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <atlstr.h>

#include <IAgoraRtcEngine.h>
#include <IAgoraMediaEngine.h>
#pragma comment(lib, "agora_rtc_sdk.lib")
using namespace agora::util;
using namespace agora::media;

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
// #include "IAgoraMediaEngine.h"
// #include "IAgoraRtcEngine.h"
#include "AGEngineEventHandler.h"
#define APP_ID "570465840e604903811de2f3a72d174b"		//���䷿��ʽID
//#define APP_ID "f4637604af81440596a54254d53ade20"
//#define APP_ID "aab8b8f5a8cd4469a63042fcfafe7063"	//��������ID
#define NEED_PING


#define NCLIENT_WIDTH 640
#define NCLIENT_HEIGHT 510
#define NTITLE_HEIGHT	30

void writeFormatLog(const char* flist, ...);
#define writelog(flist,...) writeFormatLog(flist,__VA_ARGS__);

extern bool bIsDebugMode;
extern bool bIsSaveDumpPcm;


