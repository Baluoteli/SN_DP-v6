#pragma once
#include <stdint.h>
#include "IAgoraMediaEngine.h"
#include "IAgoraRtcEngine.h"
#include "IAgoraRtcEngine2.h"
//
#include "PlayerHooker.h"

#include "AgoraPlayerCapture.h"
#include "AgoraVideoCapture.h"
#include "AgoraVideoPlay.h"
#include "AGEngineEventHandler.h"
#include "ExtendVideoFrameObserver.h"
#include "ExtendAudioFrameObserver.h"

//devices
#include "AgoraAudInputManager.h"
#include "AgoraPlayoutManager.h"
#include "AgoraCameraManager.h"

#include <string>

#define WINDOW_HIDESHOW_SHOW WM_USER + 8200
#define WINDOW_HIDESHOW_HIDE WM_USER + 8201
#define AGORA_WINDOW_START	 WM_USER + 8202
#define AGORA_WINDOW_STOP	 WM_USER + 8203
#define TimerID_SDPlay		 WM_USER + 8204
#define TimerID_Min			 WM_USER + 8205   //check if mouse out of window

#define APPVERSION "v6AgoraVideo.0.0.1"
//#define NEED_PING
#define PING_TIME_FLASH 10000
#define MAX_USER_IN_CHATROOM	10

typedef struct ChatInfo
{
	int display_width;
	int display_height;
	//room info
	BOOL bPublishing;
	BOOL bWaterMark;
	int  bLeft;
	int  nUID;
	//audio info
	std::string sMicName;
	int  nSampleRate;
	int  nMicChannel;
	//video info
	std::string sCamerName;
	BOOL bMix;
	//live
	int nBitRateVideo;
	int nWidth;
	int nHeight;
	int nFps;
	//player
	std::string sPlayerPath;

	std::string sEncPass;
	std::string sChannelName;
	std::string sChannelKey;

	std::string sJasonData;
}CHATINFO;

typedef struct RTMPInfo
{
	std::string ip;
	std::string port;
	std::string appname;
	int fps;
	int width;
	int height;
	int bitrate;
	BOOL brtmppush;
	BOOL brtmpset;
	std::string sStreamName;
	std::string sPublishAddr;
}RTMPINFO;

typedef struct userinfo
{
	int32_t		uid;
	float		x;  //[0.1, 1.0] ��Ļ������ĺ�����
	float		y;	//[0.1, 1.0] ��Ļ�������������
	float		width;	//[0.1, 1.0] ��Ļ�������ʵ�ʿ��
	float		height;  //[0.1, 1.0] ��Ļ�������ʵ�ʸ߶�
	uint32_t	zorder;  //[0, 100] �Զ���ͼ�� 0 ���� 100 ����
	uint32_t	alpha;  // ͼ��͸���� 0 ͸�� 1 ��ȫ��͸��
	uint32_t	renderMode;  // 1 �ü�  2 ����
}USERINFO;

class AgoraManager
{
public:
	AgoraManager();
	~AgoraManager();
//	unique_ptr <VideoCaptureManager> pfffff;
	agora::rtc::IRtcEngine * pRTCEngine;
	CAGEngineEventHandler *m_EngineEventHandler;
	CExtendVideoFrameObserver * m_CapVideoFrameObserver;
	CExtendAudioFrameObserver * m_CapAudioFrameObserver;
	VideoCaptureManager * pVideoCaptureManager;
	VideoPlayManager * pVideoPlayManager;
	AgoraPlayerManager* pPlayerCaptureManager;

	IAudioCaptureCallback* mpAudioCaptureCallback;

	//devices
	CAgoraPlayoutManager*	m_devPlayout;
	CAgoraAudInputManager*	m_devAudioin;
	CAgoraCameraManager*	m_devCamera;

	HANDLE mevent_start;
	int32_t nStartState;
	int32_t nDisplayMode;

	bool bFoundPlayer;
	CHATINFO ChatRoomInfo;
	//rtmp info
	RTMPINFO RtmpPushInfo;

	USERINFO UserInfo[MAX_USER_IN_CHATROOM];

	BOOL bVideoEnable;
	BOOL bAudioEnable;

	const char* SDK_Version;

	void restart();
	BOOL AgoraManager::initParam();
	int32_t start();
	BOOL stop();
	int32_t writeConfigFile(int32_t renderMode);
	int32_t readConfigFile(int32_t* renderMode);
	BOOL findPlayerPath(char* exename, int namelen, TCHAR* playerPath);
	BOOL setClientHwnd(HWND wnd, HWND wndL, HWND wndR);
	BOOL EnableLocalMirrorImage(BOOL bMirrorLocal);
	BOOL setChannelAndRole(CHANNEL_PROFILE_TYPE ch_type, CLIENT_ROLE_TYPE cl_type, char* permissionkey);
	BOOL enableOBServer(BOOL bAudioEnable, BOOL bVideoEnable);
	BOOL setDevices(std::string cMic, std::string cCamera);
	BOOL setAudioAEC(BOOL bEnable = FALSE);
	BOOL setAudioNS(BOOL bEnable  = FALSE);
	BOOL AgoraManager::setAudioAgcOn(BOOL bEnable = FALSE);
	BOOL closeDevices();
	BOOL setDevicesParam();
	BOOL initEngine(char* app_id);
	BOOL releaseEngine(BOOL bWaitRelease);
	BOOL enableVideoObserver(BOOL bEnable);
	BOOL enableAudioObserver(BOOL bEnable);
	BOOL setEventHandler(HWND hWnd);
	BOOL setRtcEngineVideoProfile(int nwidth, int nheight, int nfps, int nbitrate, BOOL bfineturn);
	BOOL setRtcEngineAudioProfile(int samplerate, int channel, BOOL bfineturn);
	BOOL setRtcEngineAudioProfileEx(int nSampleRate, int nChannels, int nSamplesPerCall);
	BOOL SetVideoRenderType(int nType);
	BOOL setLocalCanvas(uid_t uid, HWND hwnd);
	BOOL setRemoteCanvas(uid_t id, HWND hwnd);
	BOOL setCompositingLayout(uid_t nLocal, uid_t nRemote);
	BOOL muteLocalAudio(BOOL bMute);
	BOOL muteLocalVideo(BOOL bMute);
	BOOL enableVideo();
	BOOL disableVideo();
	BOOL JoinChannel(char* lpChannelName, UINT nUID, char* lpDynamicKey);
	BOOL LeaveChannel();

	uint32_t startHook();
	uint32_t stopHook();

	BOOL bStopKugou;
	BOOL bHaveHook;
	BOOL bStartKugou;
	BOOL bChooseKugou;
private:


	FILE *fRendMode;
	HWND m_RenderWnd;
	HWND m_RenderR;
	HWND m_RenderL;
};
