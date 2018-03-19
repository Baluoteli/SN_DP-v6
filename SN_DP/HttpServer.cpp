#pragma  once
#include "stdafx.h"
#include "HttpServer.h"
#include "rwlock.h"
#include <WindowsX.h>
#include "AgoraManager.h"
//jason 
#include "json.h"

using namespace std;

uint32_t nLastErrorCode;
uint32_t nActCode;
bool bInitOk;
extern AgoraManager* pAgoraManager;
extern HWND			hRenderWnd;
extern HWND			hRenderChildL;
extern HWND			hRenderChildR;

// extern uint32_t nOriginalX;
// extern uint32_t nOriginalY;
extern int nClientWidth;
extern int nClientHeight;

extern int nChildHeight;
extern int nLeftWidth;
extern int nRightWidth;

static rwlock g_lock;
bool bQuit = false;
struct _errmsg_setter
{
	_errmsg_setter(std::string& msg) : msg_(msg), err_(0) {}
	~_errmsg_setter()
	{
		if (err_ == 0)
		{
			msg_ = "[+OK]";
		}
		else
		{
			char s[256];
			sprintf_s(s, 256, "[-ERROR: #%d]", err_);
			msg_ = s;
		}
		if (!text_.empty()) msg_ += "+" + text_;
	}
	void set(int err)
	{
		err_ = err;
	}
	void set(const std::string& text)
	{
		text_ = text;
	}
private:
	std::string& msg_;
	std::string text_;
	int err_;
};

bool getcommand(const std::string& actin, std::string& command)
{
	std::string::size_type pos = actin.find('?');
	if (pos == std::string::npos)
	{
		std::string::size_type pos2 = actin.find(' ');
		if (pos2 == std::string::npos)
			return false;
		else
			command = actin.substr(0, pos2);
		return true;
	}

	command = actin.substr(0, pos);
	return true;
}

template <typename T>
bool getparam(const T& act, const T& key, T& value)
{
	size_t pos = act.find(key);
	if (pos == T::npos) return false;

	typedef typename T::value_type tvt;

	size_t tail = (pos += key.length());
	while ((tail < act.length()) && act[tail] != tvt('&') && act[tail] != tvt(' ')) ++tail;//|| act[tail] != tvt(' ')
	value = act.substr(pos, tail - pos);
	return true;
}

inline int _hex(char x)
{
	if (x >= '0' && x <= '9') return x - '0';
	if (x >= 'a' && x <= 'f') return x - 'a' + 10;
	if (x >= 'A' && x <= 'F') return x - 'A' + 10;
	return -1;
}

int hextochar(char a, char b)
{
	int ha = _hex(a);
	int hb = _hex(b);
	if (ha == -1 || hb == -1) return -1;
	return ((ha << 4) | hb);
}

void urldecode(const std::string& s, std::string& d)
{
	for (const char* buf = s.c_str(); *buf; )
	{
		if (*buf == '+')
		{
			d += ' ';
			++buf;
			continue;
		}
		if (*buf != '%')
		{
			d += *buf++;
			continue;
		}

		if (buf[1] == 0) break;
		if (buf[2] == 0) break;

		int c = hextochar(buf[1], buf[2]);
		if (c == -1) break;
		d += (char)c;
		buf += 3;
	}
}


BOOL comparePlayer(char * prossesname)
{
	if (strcmp(prossesname, ("QQMusic.exe")) == 0)
		return TRUE;
	if (strcmp(prossesname, ("KuGou.exe")) == 0)
		return TRUE;
	if (strcmp(prossesname, ("TTPlayer.exe")) == 0)
		return TRUE;
	if (strcmp(prossesname, ("BaiduMusic.exe")) == 0)
		return TRUE;
	if (strcmp(prossesname, ("6VirtualCamera.exe")) == 0)
		return TRUE;
	if (_stricmp(prossesname, ("null")) == 0)
		return TRUE;
	return FALSE;
}

bool handlegetversion(std::string &reply)
{
	//reply = APPVER;
	return true;
}

// bool handlegetsysteminfo(std::string &reply)
// {
// 	char buf[1024];
// //     const BOOL bWow64 = pSysInfoManager->IsWow64();
// //     const DWORD mem_size = pSysInfoManager->GetMemorySize();
// // 
// //     sprintf_s(buf, 1024, "IsWow64:%d\nTotalMemorySize:%dMB\nCpu_Name:%s\n",
// //         bWow64, mem_size, pSysInfoManager->GetCpuName());
//     reply = buf;
//     return true;
// }

bool handlegetrtmpstatus(std::string &reply)
{
	//      if (bQuit || !bInitOk)
	//      {
	//          char buf[128];
	//          sprintf_s(buf, 128, "exe quit\n");
	//          reply = buf;
	//          return true;
	//      }
	char buf[1024];
	const char *rtmp_status = pAgoraManager->ChatRoomInfo.bPublishing ? "ON" : "OFF";
	const char *player_startus = pAgoraManager->bFoundPlayer ? "Find" : "Not Find";
	const DWORD dwBytesInWriteBuffer = 1;
	uint32_t nError = 0;
	//     if (pMicCapManager->m_bUseJitter)
	//         nError = 0;
	//     else
	//         nError = 1;
	//     char buf[1024];

	sprintf_s(buf, 1024, "Sdk_version:%s\nPublish-Rtmp:%s\nlevel:%d\nserver:%s\nbmute:%d\nplayer:%s\n",
		pAgoraManager->SDK_Version, rtmp_status, /*pRTMPPubManager->m_CurrentEnergy*/1, pAgoraManager->RtmpPushInfo.sPublishAddr.c_str(), pAgoraManager->bAudioEnable, player_startus);

	reply = buf;
	return true;
}

bool handlemuteaudio(const std::string& act, std::string &reply)
{
	_errmsg_setter es(reply);
	char buff[64] = { 0 };
	sprintf_s(buff, 64, "%d", 1002);
	std::string trycode = buff;

	string smute;
	if (!getparam(act, std::string("bmute="), smute) || smute.length() == 0)
	{
		es.set(130);
		es.set(trycode);
	}
	int bmute = strtoul(smute.c_str(), NULL, 10);
	if (bmute)
		pAgoraManager->bAudioEnable = TRUE;
	else
		pAgoraManager->bAudioEnable = FALSE;
	pAgoraManager->muteLocalAudio(pAgoraManager->bAudioEnable);
	return true;
}

bool handlevideostart(const std::string& act, std::string &reply)
{
	char buff2[64];
	sprintf_s(buff2, sizeof(buff2), "%d", 1002);
	string errCode1002 = buff2;
	_errmsg_setter es(reply);
	if (pAgoraManager->ChatRoomInfo.bPublishing)
	{
		es.set(103);
		es.set(errCode1002);
		return true;
	}
	RECT RenderRect;
	SetRect(&RenderRect, 0, 0, pAgoraManager->ChatRoomInfo.display_width, pAgoraManager->ChatRoomInfo.display_height + NTITLE_HEIGHT);
	AdjustWindowRectEx(&RenderRect, WS_POPUP | WS_MINIMIZEBOX, FALSE, WS_EX_DLGMODALFRAME);
	
	nClientWidth = RenderRect.right - RenderRect.left;
	nClientHeight = RenderRect.bottom - RenderRect.top;

	nChildHeight = pAgoraManager->ChatRoomInfo.display_height;
	nLeftWidth = pAgoraManager->ChatRoomInfo.display_width / 2;
	nRightWidth = pAgoraManager->ChatRoomInfo.display_width;
#ifndef hideWindow
	SendMessage(hRenderWnd, WINDOW_HIDESHOW_SHOW, NULL, NULL);
#endif
	pAgoraManager->nStartState = -1;
	SendMessage(hRenderWnd, AGORA_WINDOW_START, NULL, NULL);
	
	DWORD retw = WaitForSingleObject(pAgoraManager->mevent_start, 10);
	ResetEvent(pAgoraManager->mevent_start);

	writelog("start:[%d]", pAgoraManager->nStartState);
	if (0 != pAgoraManager->nStartState)
	{
		char buff1[64];
		sprintf_s(buff1, sizeof(buff2), "%d", 1001);
		string errCode1001 = buff1;
		es.set(pAgoraManager->nStartState);
		es.set(errCode1001);
#ifndef hideWindow
		SendMessage(hRenderWnd, WINDOW_HIDESHOW_HIDE, NULL, NULL);
#endif
		return true;
	}

// #ifndef hideWindow
// 	SendMessage(hRenderWnd, WINDOW_HIDESHOW_SHOW, NULL, NULL);
// #endif

	pAgoraManager->ChatRoomInfo.bPublishing = TRUE;
#ifdef NEED_PING
	KillTimer(hRenderWnd, TimerID_SDPlay);
	SetTimer(hRenderWnd, TimerID_SDPlay, PING_TIME_FLASH, NULL);
#endif

	return true;
}

bool handlevideostop(std::string &reply)
{
	char buff2[64];
	sprintf_s(buff2, sizeof(buff2), "%d", 1002);
	string errCode1002 = buff2;

	_errmsg_setter es(reply);
	if (!pAgoraManager->ChatRoomInfo.bPublishing)	//no working
	{
		es.set(125);
		es.set(errCode1002);
		return true;
	}
	else
	{
		/// do stop act here
		pAgoraManager->stop();
#ifndef hideWindow
		SendMessage(hRenderWnd, WINDOW_HIDESHOW_HIDE, NULL, NULL);
#endif	

#ifdef NEED_PING
		KillTimer(hRenderWnd, TimerID_SDPlay);
#endif
	}
	pAgoraManager->ChatRoomInfo.bPublishing = FALSE;
	return true;
}

bool handlevideoquit(std::string &reply)
{
	bQuit = true;
	_errmsg_setter es(reply);
#ifndef hideWindow
	SendMessage(hRenderWnd, WINDOW_HIDESHOW_HIDE, NULL, NULL);
#endif
	PostMessage(hRenderWnd, WM_DESTROY, NULL, NULL);
	reply = "[+OK] [21]\n";
	return true;
}

bool handlevideoheartbeat(std::string &reply)
{
	_errmsg_setter es(reply);
	if (!pAgoraManager->ChatRoomInfo.bPublishing)// no working
	{
		char buff[64];

		if (nLastErrorCode == 10011)
		{
			sprintf_s(buff, sizeof(buff), "%d", 1001);
			string errCode1 = buff;
			es.set(nLastErrorCode);
			es.set(errCode1);
		}
		else
		{
			sprintf_s(buff, sizeof(buff), "%d", nActCode);
			string errCode = buff;
			es.set(48);
			es.set(errCode);
		}
		return true;
	}
#ifdef NEED_PING
	KillTimer(hRenderWnd, TimerID_SDPlay);
	SetTimer(hRenderWnd, TimerID_SDPlay, PING_TIME_FLASH, NULL);
#endif
	return true;
}

bool handlevideosetinitinfo(const std::string& act, std::string &reply)
{
	_errmsg_setter es(reply);
	char buff[64] = { 0 };
	sprintf_s(buff, 64, "%d", 1002);
	std::string trycode = buff;

	if (pAgoraManager->ChatRoomInfo.bPublishing)
	{
		es.set(101);
		es.set(trycode);
		return true;
	}
	std::string content, cont;
	if (!getparam(act, std::string("content="), content) || content.length() == 0)
	{
		es.set(102);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	urldecode(content, cont);
	char tempname[1024] = { 0 };
	wchar_t wstr[1024] = { 0 };
	int len = MultiByteToWideChar(CP_UTF8, 0, cont.c_str(), -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, cont.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, tempname, len, NULL, NULL);
	std::string abc = tempname;
	if (abc.empty())
	{
		es.set(103);// Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.sJasonData = abc;

	//content: {
	//ispk: 1, //是否来自PK邀请
	//	channel : "v123_123",  //频道号
	//	uploadip : "123.123.123.123 
//
	//	", //推流IP,为空不做旁路推流
	//	flvtitle : "v123_123", //流名,为空不做旁路推流
	//	channelKey : "cxxxx",  //用于加入声网频道的channel key
	//	layout : {
	//userlist: [{  //合图
	//uid: "1",  //UID
	//	x : 0,  //[0.1, 1.0] 屏幕该区域的横坐标
	//	y : 0,  //[0.1, 1.0] 屏幕该区域的纵坐标
	//	width : 0.5,  //[0.1, 1.0] 屏幕该区域的实际宽度
	//	height : 1,  //[0.1, 1.0] 屏幕该区域的实际高度
	//	zorder : 0,  //[0, 100] 自定义图层 0 最下 100 最上
	//	alpha : 1,  // 图层透明度 0 透明 1 完全不透明
	//	renderMode : 1  // 1 裁剪  2 缩放
	//},
	//{  //合图
	//uid: "1",  //UID
	//		x : 0.5,  //[0.1, 1.0] 屏幕该区域的横坐标
	//	y : 0,  //[0.1, 1.0] 屏幕该区域的纵坐标
	//	width : 0.5,  //[0.1, 1.0] 屏幕该区域的实际宽度
	//	height : 1,  //[0.1, 1.0] 屏幕该区域的实际高度
	//	zorder : 0,  //[0, 100] 自定义图层 0 最下 100 最上
	//	alpha : 1,  // 图层透明度 0 透明 1 完全不透明
	//	renderMode : 1  // 1 裁剪  2 缩放
	//}],
	//publish: {  //旁路直播推流
	//width: 800,
	//	height : 600,
	//	framerate : 15,
	//	bitrate : 1000
	//},
	//live : {  //连麦推流
	//width: 480,
	//	height : 640,
	//	framerate : 15,
	//	bitrate : 800
	//}
	//},

	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pAgoraManager->ChatRoomInfo.sJasonData.c_str(), root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
	{
		//  		std::string upload_id = root["uploadid"].asString();  // 访问节点，upload_id = "UP000000"  
		//  		int code = root["code"].asInt();    // 访问节点，code = 100 
		//  
		//  		int size_root = root["content"].size();
		//   		for (int i = 0; i < size_root; ++i)
		//   		{
		Json::Value User = root["layout"]["userlist"];
		int size_list = User.size();
		if (size_list > MAX_USER_IN_CHATROOM)
		{
			es.set(104);
			es.set(trycode);
			return true;
		}
		for (int j = 0; j < size_list; j++)
		{
			pAgoraManager->UserInfo[j].x = User[j]["x"].asDouble();
			pAgoraManager->UserInfo[j].y = User[j]["y"].asDouble();
			pAgoraManager->UserInfo[j].uid = User[j]["uid"].asInt();
			pAgoraManager->UserInfo[j].width = User[j]["width"].asDouble();
			pAgoraManager->UserInfo[j].height = User[j]["height"].asDouble();
			pAgoraManager->UserInfo[j].zorder = User[j]["zorder"].asUInt();
			pAgoraManager->UserInfo[j].alpha = User[j]["alpha"].asUInt();
			pAgoraManager->UserInfo[j].renderMode = User[j]["renderMode"].asUInt();
		}

		User = root["channel"];
		pAgoraManager->ChatRoomInfo.sChannelName = User.asString();

		User = root["channelKey"];
		pAgoraManager->ChatRoomInfo.sChannelKey = User.asString();

		User = root["flvtitle"];
		pAgoraManager->RtmpPushInfo.sStreamName = User.asString();

		User = root["uploadip"];
		pAgoraManager->RtmpPushInfo.sPublishAddr = User.asString();
		pAgoraManager->RtmpPushInfo.brtmppush = TRUE;
		if (pAgoraManager->RtmpPushInfo.sPublishAddr.empty() || pAgoraManager->RtmpPushInfo.sStreamName.empty())
		{
			pAgoraManager->RtmpPushInfo.brtmppush = FALSE;
			writelog("%s %s", pAgoraManager->RtmpPushInfo.sPublishAddr.c_str(), pAgoraManager->RtmpPushInfo.sStreamName.c_str());
		}

		User = root["layout"]["live"];
		pAgoraManager->ChatRoomInfo.nWidth = User["width"].asInt();
		pAgoraManager->ChatRoomInfo.nHeight = User["height"].asInt();
		pAgoraManager->ChatRoomInfo.nFps = User["framerate"].asInt();
		pAgoraManager->ChatRoomInfo.nBitRateVideo = User["bitrate"].asInt();

		User = root["layout"]["publish"];
		pAgoraManager->RtmpPushInfo.width = User["width"].asInt();
		pAgoraManager->RtmpPushInfo.height = User["height"].asInt();
		pAgoraManager->RtmpPushInfo.fps = User["framerate"].asInt();
		pAgoraManager->RtmpPushInfo.bitrate = User["bitrate"].asInt();
		writelog("c[%d][%d][%d] r[%d][%d][%d]",
			pAgoraManager->ChatRoomInfo.nWidth,
			pAgoraManager->ChatRoomInfo.nHeight,
			pAgoraManager->ChatRoomInfo.nBitRateVideo,
			
			pAgoraManager->RtmpPushInfo.width,
			pAgoraManager->RtmpPushInfo.height,
			pAgoraManager->RtmpPushInfo.bitrate);
		//		}
	}
	else
	{
		es.set(124);
		es.set(trycode);
		return true;
	}



	string act_decode = act;
	//urldecode(act, act_decode);
	 //////////////////////////////////设置用户ID////////////////////////////////////////
	string uid;
	if (!getparam(act, std::string("uid="), uid) || uid.length() == 0)
	{
		es.set(105);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.nUID = strtoul(uid.c_str(), NULL, 10);
	if (pAgoraManager->ChatRoomInfo.nUID == 0)
	{
		es.set(106);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.bLeft = 0;
	pAgoraManager->ChatRoomInfo.bWaterMark = TRUE;
	if (pAgoraManager->ChatRoomInfo.nUID == pAgoraManager->UserInfo[0].uid)
	{
		pAgoraManager->ChatRoomInfo.bLeft = 1;
		pAgoraManager->ChatRoomInfo.bWaterMark = FALSE;
	}

	//     //////////////////////////////////////////////////////////////////////////
	//     RECT RenderRect;
	//     SetRect(&RenderRect, 0, 0, pVideoCaptureManeger->nWidth, pVideoCaptureManeger->nHeigh);
	//     AdjustWindowRectEx(&RenderRect, WS_POPUP | WS_MINIMIZEBOX, FALSE, WS_EX_DLGMODALFRAME);
	//     nClientWidth = RenderRect.right - RenderRect.left;
	//     nClientHeight = RenderRect.bottom - RenderRect.top;

		//////////////////////////////////////music player//////////////////////////////////////
	std::string name, pname;
	if (!getparam(act_decode, std::string("player="), name) || name.length() == 0)
	{
		es.set(107);//reply = "[-ERROR] [102]\n";
		es.set(trycode);
		return true;
	}
	urldecode(name, pname);
	memset(tempname, 0, 1024);
	memset(wstr, 0, 1024);
	//char tempname[256] = { 0 };
	len = MultiByteToWideChar(CP_UTF8, 0, pname.c_str(), -1, NULL, 0);
	//wchar_t wstr[MAX_DEVICE_ID_LENGTH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, pname.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, tempname, len, NULL, NULL);
	std::string abp = tempname;
	pAgoraManager->ChatRoomInfo.sPlayerPath = abp;
	////////////////////////////////////encpass//////////////////////////////////////
	std::string encpass, se;
	if (!getparam(act, std::string("encpass="), encpass) || encpass.length() == 0)
	{
		es.set(108);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	urldecode(encpass, se);
	memset(tempname, 0, 1024);
	memset(wstr, 0, 1024);
	//char tempname[256] = { 0 };
	len = MultiByteToWideChar(CP_UTF8, 0, se.c_str(), -1, NULL, 0);
	//wchar_t wstr[MAX_DEVICE_ID_LENGTH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, se.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, tempname, len, NULL, NULL);
	std::string ab = tempname;
	if (ab.empty())
	{
		es.set(109);// Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.sEncPass = ab;

	////////////////////////////////////audio info//////////////////////////////////////
	std::string samplerate;
	if (!getparam(act, std::string("samplerate="), samplerate) || samplerate.length() == 0)
	{
		es.set(110);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.nSampleRate = strtol(samplerate.c_str(), NULL, 10);
	if (pAgoraManager->ChatRoomInfo.nSampleRate <= 0)
	{
		es.set(111);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	std::string channel;
	if (!getparam(act, std::string("audio_channel="), channel) || channel.length() == 0)
	{
		es.set(112);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.nMicChannel = strtol(channel.c_str(), NULL, 10);
	if (pAgoraManager->ChatRoomInfo.nMicChannel <= 0)
	{
		es.set(113);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	std::string bmix;
	if (!getparam(act, std::string("bmix="), bmix) || bmix.length() == 0)
	{
		es.set(114);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	int32_t	paraValue = strtol(bmix.c_str(), NULL, 10);
	if (paraValue == 1)
		pAgoraManager->ChatRoomInfo.bMix = TRUE;
	else if (paraValue == 0)
		pAgoraManager->ChatRoomInfo.bMix = FALSE;
	else
	{
		es.set(115);//reply = "[-ERROR] [52]\n";
		es.set(trycode);
		return true;
	}

	std::string ndwidth;
	if (!getparam(act, std::string("display_width="), ndwidth) || ndwidth.length() == 0)
	{
		es.set(116);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.display_width = strtol(ndwidth.c_str(), NULL, 10);
	if (pAgoraManager->ChatRoomInfo.display_width <= 0)
	{
		es.set(117);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}

	std::string ndheight;
	if (!getparam(act, std::string("display_height="), ndheight) || ndheight.length() == 0)
	{
		es.set(118);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.display_height = strtol(ndheight.c_str(), NULL, 10);
	if (pAgoraManager->ChatRoomInfo.display_height <= 0)
	{
		es.set(119);//reply = "[-ERROR] [10013]\n";
		es.set(trycode);
		return true;
	}
	writelog("width-height:[%d][%d]", pAgoraManager->ChatRoomInfo.display_width, pAgoraManager->ChatRoomInfo.display_height);

	////////////////////////////////adjust display rect//////////////////////////////////////////
// 	RECT RenderRect;
// 	//SetRect(&RenderRect, 0, 0, 1400, 1000);
// 	SetRect(&RenderRect, 0, 0, pAgoraManager->ChatRoomInfo.display_width, pAgoraManager->ChatRoomInfo.display_height + NTITLE_HEIGHT);
// 	AdjustWindowRectEx(&RenderRect, WS_POPUP | WS_MINIMIZEBOX, FALSE, WS_EX_DLGMODALFRAME);
// 
// 	//		 GetWindowRect(hRenderWnd, &RenderRect);
// 	//		 SetWindowPos(hRenderWnd, HWND_TOPMOST, RenderRect.left, RenderRect.top, pAgoraManager->ChatRoomInfo.nWidth, pAgoraManager->ChatRoomInfo.nHeight, SWP_NOSIZE);
// 	nClientWidth = RenderRect.right - RenderRect.left;
// 	nClientHeight = RenderRect.bottom - RenderRect.top;
// 
// 	nChildHeight = pAgoraManager->ChatRoomInfo.display_height;
// 	nLeftWidth = pAgoraManager->ChatRoomInfo.display_width / 2;
// 	nRightWidth = pAgoraManager->ChatRoomInfo.display_width;
	//////////////////////////////////////////////////////////////////////////

	pAgoraManager->RtmpPushInfo.brtmpset = TRUE;
	std::string adu, d;
	if (!getparam(act_decode, std::string("mic="), adu) || adu.length() == 0)
	{
		es.set(120);//Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	urldecode(adu, d);
	memset(tempname, 0, 1024);
	memset(wstr, 0, 1024);
	len = MultiByteToWideChar(CP_UTF8, 0, d.c_str(), -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, d.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, tempname, len, NULL, NULL);
	std::string a = tempname;
	if (a.empty())
	{
		es.set(121);// Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.sMicName = a;

	////////////////////////////////////video device//////////////////////////////////////
	std::string camera, cameran;
	if (!getparam(act_decode, std::string("camera="), camera) || camera.length() == 0)
	{
		es.set(122);//Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	urldecode(camera, cameran);
	memset(tempname, 0, 1024);
	memset(wstr, 0, 1024);
	len = MultiByteToWideChar(CP_UTF8, 0, cameran.c_str(), -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, cameran.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, tempname, len, NULL, NULL);
	std::string camerana = tempname;
	if (camerana.empty())
	{
		es.set(123);// Parameter audio device name is wrong";
		es.set(trycode);
		return true;
	}
	pAgoraManager->ChatRoomInfo.sCamerName = camerana;

	///////////////////////////////////water mark///////////////////////////////////////
//     std::string bwmark;//, p;
//     if (!getparam(act, std::string("wm="), bwmark) || bwmark.length() == 0)
//     {
//         pAgoraManager->ChatRoomInfo.bWaterMark = false;
//     }
//     int32_t nmark = strtol(bwmark.c_str(),NULL,10);
//     if (nmark == 1)
// 		pAgoraManager->ChatRoomInfo.bWaterMark = true;
//     else
// 		pAgoraManager->ChatRoomInfo.bWaterMark = false;
// 
//      std::string bRTMP;
//      if(!getparam(act, std::string("brtmp="), bRTMP)||bRTMP.length()==0)
//      {
//          es.set(155);//reply = "[-ERROR] [42]\n";
//          es.set(errCode1002);
//          return true;
//      }
// 	 pAgoraManager->RtmpPushInfo.brtmppush = strtol(bRTMP.c_str(), NULL, 10);
//      if (pAgoraManager->RtmpPushInfo.brtmppush == 1)
//      {
//          pAgoraManager->RtmpPushInfo.brtmpset = FALSE;
//          std::string au;//, a;
//          if (!getparam(act_decode, std::string("addr_rtmp="), au) || au.length() == 0)
//          {
//              es.set(14);//Parameter server address is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          if (au.empty())
//          {
//              es.set(15);//Parameter server address is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.ip = au;
// //          ULONG ipaddr = inet_addr(au.c_str());
// //          if (ipaddr == 0)
// //          {
// //              es.set(16);//Parameter server address is wrong";
// //              es.set(errCode1002);
// //              return true;
// //          }
// //          RtmpConfigure.ip = ipaddr;
//          std::string pu;//, p;
//          if (!getparam(act_decode, std::string("port_rtmp="), pu) || pu.length() == 0)
//          {
//              es.set(17);//Parameter server port is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          if (pu.empty())
//          {
//              es.set(18);//Parameter server port is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.port = pu;
// 		 USHORT port = (USHORT)strtoul(pu.c_str(), NULL, 10);
//          if (port == 0)
//          {
//              es.set(19);//Parameter server port is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          std::string appu;//, app;
//          if (!getparam(act_decode, std::string("appname="), appu) || appu.length() == 0)
//          {
//              es.set(24);//Parameter appname is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          if (appu.empty())
//          {
//              es.set(25);//Parameter appname is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.appname = appu;
//          std::string su;//, s;
//          if (!getparam(act_decode, std::string("streamname="), su) || su.length() == 0)
//          {
//              es.set(26);//Parameter streamname is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          if (su.empty())
//          {
//              es.set(27);//Parameter streamname is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.streamname = su;
//          std::string fpsu;//, fps;
//          if (!getparam(act_decode, std::string("fps_rtmp="), fpsu) || fpsu.length() == 0)
//          {
//              es.set(28);//Parameter fps is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.fps =strtoul(fpsu.c_str(),NULL,10);
//          if (pAgoraManager->RtmpPushInfo.fps == 0)
//          {
//              es.set(30);//Parameter fps is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          std::string bitrateu;//, bitrate;
//          if (!getparam(act_decode, std::string("bitrate_rtmp="), bitrateu) || bitrateu.length() == 0)
//          {
//              es.set(31);//Parameter bitrate is wrong";
//              es.set(errCode1002);
//              return true;
//          }
//          pAgoraManager->RtmpPushInfo.bitrate = strtoul(bitrateu.c_str(),NULL,10);
//          if (pAgoraManager->RtmpPushInfo.bitrate == 0)
//          {
//              es.set(33);//Parameter bitrate is wrong";
//              es.set(errCode1002);
//              return true;
//          }
// 		 //////////////////////////////////////////////////////////////////////////
// 		 std::string widthu;//, bitrate;
// 		 if (!getparam(act_decode, std::string("width_rtmp="), widthu) || widthu.length() == 0)
// 		 {
// 			 es.set(35);//Parameter bitrate is wrong";
// 			 es.set(errCode1002);
// 			 return true;
// 		 }
// 		 pAgoraManager->RtmpPushInfo.width = strtoul(widthu.c_str(), NULL, 10);
// 		 if (pAgoraManager->RtmpPushInfo.width == 0)
// 		 {
// 			 es.set(36);//Parameter bitrate is wrong";
// 			 es.set(errCode1002);
// 			 return true;
// 		 }
// 		 //////////////////////////////////////////////////////////////////////////
// 		 std::string heightu;//, bitrate;
// 		 if (!getparam(act_decode, std::string("height_rtmp="), heightu) || heightu.length() == 0)
// 		 {
// 			 es.set(37);//Parameter bitrate is wrong";
// 			 es.set(errCode1002);
// 			 return true;
// 		 }
// 		 pAgoraManager->RtmpPushInfo.height = strtoul(heightu.c_str(), NULL, 10);
// 		 if (pAgoraManager->RtmpPushInfo.height == 0)
// 		 {
// 			 es.set(38);//Parameter bitrate is wrong";
// 			 es.set(errCode1002);
// 			 return true;
// 		 }
//      }
	return true;
}

DWORD Getenginewidth()
{
	const TCHAR _szLogSubKey[] = TEXT("Software\\6RoomsVCamera");
	const TCHAR _szLogValueName[] = TEXT("EngineWidth");
	HKEY hKey;
	LONG rc;
	DWORD dwType;
	DWORD dwLogFile = sizeof(DWORD);
	DWORD nWidth;

	rc = RegOpenKeyEx(HKEY_CURRENT_USER, _szLogSubKey, 0, KEY_READ, &hKey);
	if (rc != ERROR_SUCCESS)
		return 800;
	rc = RegQueryValueEx(hKey, _szLogValueName, NULL, &dwType, (LPBYTE)&nWidth, &dwLogFile);
	if (rc == ERROR_SUCCESS && dwType == REG_DWORD)
	{
		RegCloseKey(hKey);
		return nWidth;
	}
	RegCloseKey(hKey);
	return 800;
}

bool handlevideogetenginewidth(std::string &reply)
{
	DWORD mem_size = Getenginewidth();

	char buf[1024] = { 0 };

	sprintf_s(buf, 1024, "width:%d\n", mem_size);
	reply = buf;
	return true;
}


bool parseact(const std::string& act_de, std::string& reply, std::string& mime)
{
//	if (!strstr(act_de.c_str(), "getversion")) writelog("%s", act_de.c_str());
	std::string act_dec;
	urldecode(act_de, act_dec);
	if (act_dec.length() < 15 || memcmp(act_dec.c_str(), "GET /", 5) != 0) return false;

	std::string command;

	if (!getcommand(act_dec.substr(5), command)) return false;

	bool success = false;

	g_lock.WaitToWrite();

	if (command == "crossdomain.xml")
	{
		success = true;
		mime = "text/xml";
		reply =
			"<?xml version=\"1.0\"?>"
			"<cross-domain-policy>"
			"<allow-access-from domain=\"*\" />"
			"</cross-domain-policy>\n";
	}
	else if (command == "getversion")
	{
		success = handlegetversion(reply);
	}
	else if (command == "getpublishstatus")
	{
		success = handlegetrtmpstatus(reply);
		//writelog("getpublishstatus :%s", reply.c_str());
	}
	//     else if (command == "getsysteminfo")
	//     {
	//         success = handlegetsysteminfo(reply);
	//     }

	//     else if(command == "videopreview")
	//     {
	//         success = handlevideopreview(act_dec, reply);
	//     }
	else if (command == "getenginewidth")
	{
		success = handlevideogetenginewidth(reply);
	}
	else if (command == "agoramuteaudio")
	{
		success = handlemuteaudio(act_dec, reply);
		writelog("agoramuteaudio :%s", reply.c_str());
	}
	else if (command == "agorastart")
	{
		success = handlevideostart(act_dec, reply);
		writelog("agorastart :%s", reply.c_str());
	}
	else if (command == "agorastop")
	{
		success = handlevideostop(reply);
		writelog("agorastop :%s", reply.c_str());
	}
	else if (command == "agoraquit")
	{
		success = handlevideoquit(reply);
	}
	else if (command == "agorasetinitinfo")
	{
		success = handlevideosetinitinfo(act_dec, reply);
		writelog("agorasetinitinfo :%s", reply.c_str());
	}
	else if (command == "agoraping")
	{
		success = handlevideoheartbeat(reply);
		writelog("agoraping :%s", reply.c_str());
	}
	g_lock.Done();
	return success;
}

bool parsereq(const std::string& req, std::string& reply, std::string& mime)
{
	std::string::size_type p = req.find("\r\n");
	if (p == std::string::npos) return false;
	std::string act(req.substr(0, p));

	return parseact(act, reply, mime);
}
