#include "stdafx.h"
#include "AgoraManager.h"

AgoraManager::AgoraManager()
{
	this->bStopKugou = FALSE;
	this->bStartKugou = FALSE;
	this->fRendMode = NULL;
	this->nDisplayMode = DISPLAY_D3D;
	this->mevent_start = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->bAudioEnable = TRUE;
	this->bVideoEnable = TRUE;
	bHaveHook = FALSE;
}

AgoraManager::~AgoraManager()
{
	CloseHandle(this->mevent_start);
}

int32_t AgoraManager::writeConfigFile(int32_t renderMode)
{
	if (renderMode != DISPLAY_D2D && renderMode != DISPLAY_GDI && renderMode != DISPLAY_D3D)
		return 1;
	this->fRendMode = fopen(".//config.dat", "wb");
	if (this->fRendMode)
	{
		char temp[8] = { 0 };
		sprintf_s(temp, 8, "%d", renderMode);
		fwrite(temp, 1, 8, this->fRendMode);
		fclose(this->fRendMode);
	}
	else
		return 2;
	return 0;
}

int32_t AgoraManager::readConfigFile(int32_t* renderMode)
{
	this->fRendMode = fopen(".//config.dat", "rb");
	if (this->fRendMode)
	{
		char temp[8] = { 0 };
		int32_t ntemp = 0;
		fread(temp, 1, 8, this->fRendMode);
		fclose(this->fRendMode);
		ntemp = atoi(temp);
		if (ntemp != DISPLAY_D2D && ntemp != DISPLAY_GDI && ntemp != DISPLAY_D3D)
			return 1;
		*renderMode = ntemp;
	}
	else
		return 2;
	return 0;
}

BOOL AgoraManager::findPlayerPath(char* exename, int namelen, TCHAR* playerPath)
{
#define MY_BUFSIZE 256
	HKEY hKey;
	TCHAR szFindPath[MY_BUFSIZE] = { 0 };
	char szPlayer[MY_BUFSIZE] = { 0 };

	int i = 0;
	for (; i < namelen; i++)
	{
		if (exename[i] == '.')
			break;
		szPlayer[i] = exename[i];
	}
	if (i == namelen)
		return FALSE;

	TCHAR path_temp[MY_BUFSIZE] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, szPlayer, -1, path_temp, i);

	wsprintf(szFindPath, L"software\\%s", path_temp);
	TCHAR szProductType[MY_BUFSIZE];
	memset(szProductType, 0, sizeof(szProductType));
	DWORD dwBufLen = MY_BUFSIZE;
	LONG lRet;

	// 下面是打开注册表, 只有打开后才能做其他操作
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER,  // 要打开的根键 
		szFindPath, // 要打开的子子键 
		0,        // 这个一定要为0 
		KEY_QUERY_VALUE,  //  指定打开方式,此为读 
		&hKey);    // 用来返回句柄 

	if (lRet != ERROR_SUCCESS)   // 判断是否打开成功 
		return FALSE;
	//下面开始查询 
	lRet = RegQueryValueEx(hKey,  // 打开注册表时返回的句柄 
		TEXT("AppPath"),  //要查询的名称,qq安装目录记录在这个保存 
		NULL,   // 一定为NULL或者0 
		NULL,
		(LPBYTE)szProductType, // 我们要的东西放在这里 
		&dwBufLen);
	if (lRet != ERROR_SUCCESS)  // 判断是否查询成功 
		return FALSE;
	RegCloseKey(hKey);

	//memcpy(exePath, szProductType, dwBufLen);
	wsprintf(playerPath, L"%s\\%s", szProductType, L"KuGou.exe");

	return TRUE;
}

BOOL AgoraManager::setClientHwnd(HWND wnd, HWND wndL, HWND wndR)
{
	m_RenderWnd = wnd;
	m_RenderR = wndR;
	m_RenderL = wndL;
	return TRUE;
}

BOOL AgoraManager::setEventHandler(HWND hWnd)
{
	this->m_EngineEventHandler->SetMsgReceiver(hWnd);
	return TRUE;
}

BOOL AgoraManager::setRtcEngineVideoProfile(int nwidth, int nheight, int nfps, int nbitrate, BOOL bfineturn)
{
	IRtcEngine2 *lpRtcEngine2 = (IRtcEngine2 *)this->pRTCEngine;
	int nRet = lpRtcEngine2->setVideoProfileEx(nwidth, nheight, nfps, nbitrate);
	//pRTCEngine->setVideoProfile(VIDEO_PROFILE_TYPE::VIDEO_PROFILE_240P, true);

	// int ret = this->pRTCEngine->setVideoProfile2(nwidth, nheight)
	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setRtcEngineAudioProfile(int samplerate, int channel, BOOL bfineturn)
{
// 		AUDIO_PROFILE_MUSIC_STANDARD = 2, // 48Khz, 50kbps, mono, music
// 		AUDIO_PROFILE_MUSIC_STANDARD_STEREO = 3, // 48Khz, 50kbps, stereo, music
// 		AUDIO_PROFILE_MUSIC_HIGH_QUALITY = 4, // 48Khz, 128kbps, mono, music
// 		AUDIO_PROFILE_MUSIC_HIGH_QUALITY_STEREO = 5, // 48Khz, 128kbps, stereo, music
	
// 	int nSamplesPerCall = 44100 * 2 / 100;
// 	RtcEngineParameters rep(this->pRTCEngine);
// 	int ret = rep.setRecordingAudioFrameParameters(44100, 2, RAW_AUDIO_FRAME_OP_MODE_READ_WRITE, nSamplesPerCall);
// 	if (ret == 0)
	int ret = pRTCEngine->setAudioProfile(AUDIO_PROFILE_MUSIC_HIGH_QUALITY_STEREO, AUDIO_SCENARIO_SHOWROOM);

	return ret == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setRtcEngineAudioProfileEx(int nSampleRate, int nChannels, int nSamplesPerCall)
{
	RtcEngineParameters rep(pRTCEngine);

	int nRet = rep.setRecordingAudioFrameParameters(nSampleRate, nChannels, RAW_AUDIO_FRAME_OP_MODE_READ_WRITE, nSamplesPerCall);

	return nRet == 0 ? TRUE : FALSE;
}

enum VideoRenderType {
	kRenderWindowsD3D = 7, // only for windows
	kRenderWindowsD2D = 8, // only for windows
	kRenderWindowsGDI = 9, // only for windows
};

BOOL AgoraManager::SetVideoRenderType(int nType)
{
	int	nRet = 0;

	AParameter apm(this->pRTCEngine);
	switch (nType)
	{
	case 1:
		nRet = apm->setInt("che.video.renderer.type", VideoRenderType::kRenderWindowsGDI);
		break;
	case 2:
		nRet = apm->setInt("che.video.renderer.type", VideoRenderType::kRenderWindowsD3D);
		break;
	case 3:
		nRet = apm->setInt("che.video.renderer.type", VideoRenderType::kRenderWindowsD2D);
		break;
	default:
		nRet = apm->setInt("che.video.renderer.type", VideoRenderType::kRenderWindowsD3D);
		break;
	}

	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setLocalCanvas(uid_t uid, HWND hwnd)
{
	VideoCanvas vc;
	vc.uid = uid;
	vc.renderMode = RENDER_MODE_TYPE::RENDER_MODE_HIDDEN;
	vc.view = hwnd;
	int ret = pRTCEngine->setupLocalVideo(vc);
	if (ret == 0)
		return TRUE;
	return FALSE;
}

BOOL AgoraManager::setRemoteCanvas(uid_t id, HWND hwnd)
{
	VideoCanvas vc1;
	vc1.uid = id;
	vc1.renderMode = RENDER_MODE_TYPE::RENDER_MODE_HIDDEN;
	vc1.view = hwnd;
	int ret = pRTCEngine->setupRemoteVideo(vc1);
	if (ret == 0)
		return TRUE;
	return FALSE;
}

//can be called before leave channel
BOOL AgoraManager::enableVideo()
{
	int nRet = pRTCEngine->enableVideo();
	if (nRet != 0)
		return FALSE;
	bVideoEnable = TRUE;
	return TRUE;
}

BOOL AgoraManager::disableVideo()
{
	int nRet = pRTCEngine->disableVideo();
	if (nRet != 0)
		return FALSE;
	bVideoEnable = FALSE;
	return TRUE;
}

BOOL AgoraManager::setCompositingLayout(uid_t nLocal, uid_t nRemote)
{
	uid_t nleft = nLocal;
	uid_t nright = nRemote;
	if (!this->ChatRoomInfo.bLeft)
	{
		nleft = nRemote;
		nright = nLocal;
	}
	VideoCompositingLayout::Region aRegion[2];
	aRegion[0].x = this->UserInfo[0].x;
	aRegion[0].y = this->UserInfo[0].y;
	aRegion[0].width = this->UserInfo[0].width;
	aRegion[0].height = this->UserInfo[0].height;
	aRegion[0].alpha = this->UserInfo[0].alpha;
	aRegion[0].zOrder = this->UserInfo[0].zorder;
	aRegion[0].renderMode = (RENDER_MODE_TYPE)this->UserInfo[0].renderMode;
	aRegion[0].uid = this->UserInfo[0].uid;

	aRegion[1].x = this->UserInfo[1].x;
	aRegion[1].y = this->UserInfo[1].y;
	aRegion[1].width = this->UserInfo[1].width;
	aRegion[1].height = this->UserInfo[1].height;
	aRegion[1].alpha = this->UserInfo[1].alpha;
	aRegion[1].zOrder = this->UserInfo[1].zorder;
	aRegion[1].renderMode = (RENDER_MODE_TYPE)this->UserInfo[1].renderMode;
	aRegion[1].uid = this->UserInfo[1].uid;

	VideoCompositingLayout aLayout;
	aLayout.regionCount = 2;

	aLayout.regions = aRegion;
	this->pRTCEngine->setVideoCompositingLayout(aLayout);
	return TRUE;
}

BOOL AgoraManager::JoinChannel(char* szChannelName, UINT nUID, char* lpDynamicKey)
{
	int nRet = 0;
	char tempInfo[2048] = { 0 };
	if (this->RtmpPushInfo.brtmppush)
	{
		char tempPath[1024] = { 0 };
		sprintf_s(tempPath, 1024, "rtmp://%s/liverecord/%s?uid=%d&expass=%s", this->RtmpPushInfo.sPublishAddr.c_str(), this->RtmpPushInfo.sStreamName.c_str(), this->ChatRoomInfo.nUID, this->ChatRoomInfo.sEncPass.c_str());
		writelog("rtmpinfo:%s", tempPath);
		//sprintf_s(tempPath, 128, "rtmp://%s:%s/%s/%s", this->RtmpPushInfo.ip.c_str(), this->RtmpPushInfo.port.c_str(), this->RtmpPushInfo.appname.c_str(), this->RtmpPushInfo.streamname.c_str());
		sprintf_s(tempInfo, 2048, "{\"owner\":true,\"lifecycle\":2,\"defaultLayout\":0,\"width\":%d,\"height\":%d,\"framerate\":%d,\"audiosamplerate\":44100,\"audiobitrate\":96000,\"audiochannels\":2,\"bitrate\":%d,\"mosaicStream\":\"%s\",\"extraInfo\":\"{\\\"lowDelay\\\":true}\"}",
			this->RtmpPushInfo.width, this->RtmpPushInfo.height, this->RtmpPushInfo.fps, this->RtmpPushInfo.bitrate, tempPath/*"rtmp://aliliveup.6rooms.com/liverecord/v587"*/);
		writelog("channel info:%s", tempInfo);
	}

	nRet = pRTCEngine->joinChannel(lpDynamicKey, szChannelName, tempInfo, nUID);
	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::LeaveChannel()
{
//	this->pPlayerCaptureManager->startHook(TRUE);
	int ret = 0;
	ret = this->pRTCEngine->leaveChannel();
	return ret == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::initEngine(char* app_id)
{
	RtcEngineContext ctx;
	ctx.appId = app_id;
	ctx.eventHandler = this->m_EngineEventHandler;
	this->pRTCEngine->initialize(ctx);

	return TRUE;
}

BOOL AgoraManager::releaseEngine(BOOL bWaitRelease)
{
	this->pRTCEngine->release(bWaitRelease);
	return TRUE;
}

BOOL AgoraManager::enableAudioObserver(BOOL bEnable)
{
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	mediaEngine.queryInterface(this->pRTCEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);

	int nRet = 0;

	if (mediaEngine.get() == NULL)
		return FALSE;

	if (bEnable && this->m_CapAudioFrameObserver != NULL)
		nRet = mediaEngine->registerAudioFrameObserver(this->m_CapAudioFrameObserver);
	else
		nRet = mediaEngine->registerAudioFrameObserver(NULL);
	return TRUE;
}
///open ob server
BOOL AgoraManager::enableVideoObserver(BOOL bEnable)
{
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	mediaEngine.queryInterface(this->pRTCEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);

	int nRet = 0;
	AParameter apm(*this->pRTCEngine);

	if (mediaEngine.get() == NULL)
		return FALSE;

	if (bEnable && this->m_CapVideoFrameObserver != NULL) {
		apm->setParameters("{\"che.video.local.camera_index\":1024}");//1024 means we use observer instead of camera.
		nRet = mediaEngine->registerVideoFrameObserver(this->m_CapVideoFrameObserver);
	}
	else {
		nRet = mediaEngine->registerVideoFrameObserver(NULL);
		apm->setParameters("{\"che.video.local.camera_index\":0}");
	}
	return TRUE;
}

BOOL AgoraManager::muteLocalAudio(BOOL bMute)
{
	RtcEngineParameters rep(*this->pRTCEngine);
	int ret = rep.muteLocalAudioStream(bMute);
	if (ret == 0)
		return TRUE;
	return FALSE;
}

BOOL AgoraManager::muteLocalVideo(BOOL bMute)
{
	RtcEngineParameters rep(*this->pRTCEngine);
	int ret = rep.muteLocalVideoStream(bMute);
	if (ret == 0)
		return TRUE;
	return FALSE;
}

BOOL AgoraManager::enableOBServer(BOOL bAudioEnable, BOOL bVideoEnable)
{
	BOOL bDone = FALSE;
	///set observer
	bDone = this->enableVideoObserver(bVideoEnable);
	if (!bDone)
		return FALSE;
	bDone = this->enableAudioObserver(bAudioEnable);
	if (!bDone)
	{
		this->enableVideoObserver(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL AgoraManager::closeDevices()
{
	this->m_devAudioin->Close();
	this->m_devPlayout->Close();
	return TRUE;
}

BOOL AgoraManager::setDevices(std::string cMic, std::string cCamera)
{
	BOOL bdone = FALSE;
	int ret = -1;
	//enable function
	writelog("enableVideo");
	ret = this->pRTCEngine->enableVideo();
	if (ret != 0)
		return FALSE;
	writelog("enableAudio");
	ret = this->pRTCEngine->enableAudio();
	if (ret != 0)
		return FALSE;
	//create devices
	writelog("m_devAudioin->Create");
	if (!this->m_devAudioin->Create(this->pRTCEngine))
		return FALSE;
#if 0
	if (!this->m_devPlayout->Create(this->pRTCEngine))
		return FALSE;
#endif
	//set devices
	char device_id[MAX_DEVICE_ID_LENGTH] = { 0 };

	writelog("m_devAudioin->GetDevice");
	bdone = this->m_devAudioin->GetDevice(cMic, device_id);
	if (bdone)
	{
		writelog("m_devAudioin->GetDevice");
		bdone = this->m_devAudioin->SetCurDevice(device_id);
	}
	if (!bdone)
		return FALSE;

#if 0
	memset(device_id, 0, MAX_DEVICE_ID_LENGTH);
	bdone = this->m_devPlayout->GetDevice(cCamera, device_id);
	if (bdone)
		bdone = this->m_devPlayout->SetCurDevice(device_id);
	if (!bdone)
		return FALSE;
#endif

	return TRUE;
}

BOOL AgoraManager::setAudioAEC(BOOL bEnable /*= FALSE*/)
{
	int nRet = 0;

	AParameter apm(*this->pRTCEngine);

	if (bEnable)
		nRet = apm->setParameters("{\"che.audio.enable.aec\":true}");
	else
		nRet = apm->setParameters("{\"che.audio.enable.aec\":false}");

	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setAudioNS(BOOL bEnable /*= FALSE*/)
{
	int nRet = 0;

	AParameter apm(*this->pRTCEngine);

	if (bEnable)
		nRet = apm->setParameters("{\"che.audio.enable.ns\":true}");
	else
		nRet = apm->setParameters("{\"che.audio.enable.ns\":false}");

	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setAudioAgcOn(BOOL bEnable /*= FALSE*/)
{
	int nRet = 0;

	AParameter apm(*this->pRTCEngine);

	if (bEnable)
		nRet = apm->setParameters("{\"che.audio.agcOn\":true}");
	else
		nRet = apm->setParameters("{\"che.audio.agc\":false}");

	return nRet == 0 ? TRUE : FALSE;
}

BOOL AgoraManager::setDevicesParam()
{
	BOOL bDone = FALSE;

	bDone = this->setRtcEngineVideoProfile(this->ChatRoomInfo.nWidth, this->ChatRoomInfo.nHeight, this->ChatRoomInfo.nFps, this->ChatRoomInfo.nBitRateVideo, TRUE);
	if (bDone)
		bDone = this->setRtcEngineAudioProfile(this->ChatRoomInfo.nSampleRate, this->ChatRoomInfo.nMicChannel, TRUE);

	return bDone;
}

BOOL AgoraManager::setChannelAndRole(CHANNEL_PROFILE_TYPE ch_type, CLIENT_ROLE_TYPE cl_type, char* permissionkey)
{
	int res = -1;
	//set channel type
	res = this->pRTCEngine->setChannelProfile(CHANNEL_PROFILE_LIVE_BROADCASTING);
	if (res != 0)
		return FALSE;
	//set client type
	res = this->pRTCEngine->setClientRole(CLIENT_ROLE_BROADCASTER, permissionkey);
	if (res != 0)
		return FALSE;
	return TRUE;
}

BOOL AgoraManager::EnableLocalMirrorImage(BOOL bMirrorLocal)
{
	int nRet = 0;

	AParameter apm(*this->pRTCEngine);

	if (bMirrorLocal)
		nRet = apm->setParameters("{\"che.video.localViewMirrorSetting\":\"forceMirror\"}");
	else
		nRet = apm->setParameters("{\"che.video.localViewMirrorSetting\":\"disableMirror\"}");

	return nRet == 0 ? TRUE : FALSE;
}

void AgoraManager::restart()
{
	if (this->ChatRoomInfo.bPublishing)
	{
		int32_t ret = 0;
		this->stop();
		ret = this->start();
		writelog("restart done[%d]", ret);
	}
}

BOOL AgoraManager::initParam()
{
	this->ChatRoomInfo.bLeft = TRUE;
	this->ChatRoomInfo.bMix = TRUE;
	this->ChatRoomInfo.bPublishing = FALSE;
	this->ChatRoomInfo.bWaterMark = TRUE;
	this->ChatRoomInfo.display_height = 600;
	this->ChatRoomInfo.display_width = 800;
	this->ChatRoomInfo.nBitRateVideo = 400;
	this->ChatRoomInfo.nFps = 15;
	this->ChatRoomInfo.nHeight = 360;
	this->ChatRoomInfo.nMicChannel = 2;
	//this->ChatRoomInfo.nRID = 1111;
	this->ChatRoomInfo.nSampleRate = 44100;
	this->ChatRoomInfo.nUID = 1113;
	this->ChatRoomInfo.nWidth = 480;
	this->ChatRoomInfo.sCamerName = "Integrated Webcam";
	this->ChatRoomInfo.sChannelKey = "";
	this->ChatRoomInfo.sChannelName = "123test_baluoteliz";
	this->ChatRoomInfo.sMicName = "";
	this->ChatRoomInfo.sPlayerPath = "KuGou.exe";
	//this->ChatRoomInfo.sPublishUrl = "";

	this->RtmpPushInfo.ip = "aliliveup.6rooms.com";
	this->RtmpPushInfo.port = "1935";
	this->RtmpPushInfo.appname = "liverecord";
	//this->RtmpPushInfo.streamname = "v587";
	this->RtmpPushInfo.bitrate = 1000;
	this->RtmpPushInfo.brtmppush = TRUE;
	this->RtmpPushInfo.brtmpset = TRUE;
	this->RtmpPushInfo.fps = 15;
	this->RtmpPushInfo.height = 600;
	this->RtmpPushInfo.width = 800;
	return TRUE;
}

uint32_t AgoraManager::startHook()
{
	TCHAR path_temp[256] = { 0 };
	this->bFoundPlayer = false;//record upload param
	if (!this->bStartKugou)
	{
		//BOOL b = this->findPlayerPath("kugou.exe", sizeof("kugou.exe"), path_temp);
		ChatRoomInfo.sPlayerPath = "Kugou.exe";
		if (this->findPlayerPath((char*)this->ChatRoomInfo.sPlayerPath.c_str(), this->ChatRoomInfo.sPlayerPath.length(), path_temp))
		{
			this->bFoundPlayer = true;
			this->pPlayerCaptureManager->startHook(TRUE, path_temp);
			this->bStartKugou = TRUE;
			bHaveHook = TRUE;
		}
		writelog("findPlayerPath:%d", this->bFoundPlayer);
		//	this->pPlayerCaptureManager->startHook(TRUE, L"D:\\Program Files (x86)\\KuGou\\KGMusic\\KuGou.exe");
		OutputDebugStringA("player hook start");
	}

	return TRUE;
}

uint32_t AgoraManager::stopHook()
{
	if (this->pVideoCaptureManager)
	{
		this->pVideoCaptureManager->stopCapture();
		OutputDebugStringA("1....stopCapture\n");
	}
	else
		OutputDebugStringA("1....stopCapture failed\n");

	if (this->bStopKugou)
	{
		this->pPlayerCaptureManager->startHook(FALSE, NULL);
		bStopKugou = FALSE;
	}

	return TRUE;
}

int32_t AgoraManager::start()
{
	int res = 0;
	int ret = 0;
	BOOL bdone = FALSE;
// 
// 	this->initEngine(APP_ID);
// 	//set hwnd for message post by event_handler
// 	this->setEventHandler(m_RenderWnd);
// 	//open log
// 	RtcEngineParameters rep(pRTCEngine);
// 	res = rep.setLogFile("C:\\6RoomsLog\\agoraSDk.log");

	startHook();

// 	res = 1;
// 
//  	char tempPath[512] = { 0 };
//  	FILE* playerpath = fopen("./playerpath.txt", "rb");
//  	if (playerpath == NULL)
//  	{
//  		goto StartError;
//  	}
//  	int32_t lSize = 0;
//  	/* 获取文件大小 */
//  	fseek(playerpath, 0, SEEK_END);
//  	lSize = ftell(playerpath);
//  	rewind(playerpath);
//  	fread(tempPath, 1, lSize, playerpath);
//  	fclose(playerpath);
//  
//  	TCHAR path_temp[256] = { 0 };
//  	MultiByteToWideChar(CP_UTF8, 0, tempPath, -1, path_temp, lSize);
 	//this->pPlayerCaptureManager->startHook(TRUE, path_temp);

//	return 0;

	res = 1;
	writelog("initCapture");
	this->pVideoCaptureManager->initCapture(this->ChatRoomInfo.nWidth, this->ChatRoomInfo.nHeight, this->ChatRoomInfo.nFps);
	if (!this->pVideoCaptureManager->startCapture())
		goto StartError;

	writelog("setRtcEngineAudioProfileEx");
	setRtcEngineAudioProfileEx(44100, 2, 44100 * 2 / 100);
	res = 2;
	writelog("enableOBServer");
	if (!this->enableOBServer(TRUE, TRUE))
		goto StartError;
	
	res = 3;
	//ret = this->readConfigFile(&this->nDisplayMode);
	if (ret != 0)
		writelog("read config err:%d", ret);
	if (!SetVideoRenderType(DISPLAY_GDI))
		goto StartError;
	writelog("set render type:GDI");

	res = 4;
	char* permissionkey = NULL;
	writelog("setChannelAndRole");
	if (!this->setChannelAndRole(CHANNEL_PROFILE_LIVE_BROADCASTING, CLIENT_ROLE_BROADCASTER, permissionkey))
		goto StartError;
	//std::string mic = "内装麦克风 (Conexant SmartAudio HD)";
	//std::string sounder = "扬声器 (Conexant SmartAudio HD)";
	//this->setDevices(mic, sounder);

	res = 5;
	writelog("EnableLocalMirrorImage");
	if (!this->EnableLocalMirrorImage(FALSE))
		goto StartError;

	res = 6;
	writelog("setLocalCanvas");
	if (this->ChatRoomInfo.bLeft)
		this->setLocalCanvas(this->ChatRoomInfo.nUID, m_RenderL);
	else
		this->setLocalCanvas(this->ChatRoomInfo.nUID, m_RenderR);

	res = 7;
	writelog("setDevicesParam");
	if (!this->setDevicesParam())
		goto StartError;

	this->pRTCEngine->enableVideo();
	this->pRTCEngine->startPreview();
	res = 8;
	writelog("setDevices");
	if (!this->setDevices(this->ChatRoomInfo.sMicName, this->ChatRoomInfo.sCamerName))
		goto StartError;

	res = 9;
	writelog("setAudioAES false");
	if (!setAudioAEC(FALSE))
		goto StartError;

	res = 10;
	writelog("setAudioNS true");
	if (!setAudioNS(true))
		goto StartError;

	res = 11;
	writelog("setAudioAgcGain false");
	if (!setAudioAgcOn(FALSE))
		goto StartError;

	res = 10;
	writelog("JoinChannel");
	if (!this->JoinChannel((char*)this->ChatRoomInfo.sChannelName.c_str(), this->ChatRoomInfo.nUID, (char*)this->ChatRoomInfo.sChannelKey.c_str()))
		goto StartError;

	return 0;
StartError:
	writelog("start err:%d", res);
	return res;
}

BOOL AgoraManager::stop()
{
	stopHook();
	this->enableOBServer(FALSE, FALSE);
	this->LeaveChannel();//close all engine resources
	this->pRTCEngine->stopPreview();
	this->ChatRoomInfo.bPublishing = FALSE;
	return TRUE;
}