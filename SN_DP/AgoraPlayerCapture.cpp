#include "stdafx.h"


#include "AgoraPlayerCapture.h"
#include "PlayerHooker.h"

#include "AgoraManager.h"
//#include <timeapi.h>
extern AgoraManager*	pAgoraManager;

CAudioCaptureCallback::CAudioCaptureCallback()
	: mpFile(NULL)
{
//	m_fileBkMusicSrc.openMedia("D:\\V6room\\BkMusicSrc.pcm");
	DeleteFile(_T("./V6room/MusicSrc.pcm"));
}

CAudioCaptureCallback::~CAudioCaptureCallback()
{
	if (mpFile != NULL)
	{
		fclose(mpFile);
	}
//	m_fileBkMusicSrc.close();
}

void CAudioCaptureCallback::onCaptureStart()
{

}

void CAudioCaptureCallback::onCaptureStop()
{
	PostQuitMessage(0);
	OutputDebugStringA("2....onCaptureStop\n");
}

//get audio data
//DWORD timelast = timeGetTime();
int ninclen = 0;
int nincdatalen = 0;
void CAudioCaptureCallback::onCapturedData(void* data, UINT dataLen, WAVEFORMATEX* format)
{
	if (pAgoraManager->bChooseKugou)
	{
		if (bIsDebugMode) {

			static int nCountAudioCallBack = 0;
			nCountAudioCallBack++;
			static DWORD dwLastStamp = GetTickCount();
			DWORD dwCurrStamp = GetTickCount();
			if (5000 < dwCurrStamp - dwLastStamp) {

				float fRecordAudioFrame = nCountAudioCallBack * 1000.0 / (dwCurrStamp - dwLastStamp);
				char logMsg[128] = { '\0' };
				sprintf_s(logMsg, "onCapturedData : %d,%d,16,%d [ MusicSrc Rate : %.2f]\n", dataLen, format->nChannels, format->nSamplesPerSec, fRecordAudioFrame);
				OutputDebugStringA(logMsg);

				FILE* log;
				log = fopen("./V6room/PlayerHookerV6_1.log", ("a+"));
				if (log != NULL)
				{
					SYSTEMTIME st;
					GetLocalTime(&st);
					fprintf(log, "%d%02d%02d-%02d%02d%02d%03d:  %s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, logMsg);
					fclose(log);
				}

				dwLastStamp = dwCurrStamp;
				nCountAudioCallBack = 0;
			}
		}

		pAgoraManager->pPlayerCaptureManager->getCircleBufferObject()->writeBuffer(data, dataLen);

		if (bIsSaveDumpPcm)
		{
			FILE* outfile = fopen("./V6room/MusicSrc.pcm", "ab+");
			if (outfile)
			{
				fwrite(data, 1, dataLen, outfile);
				fclose(outfile);
				outfile = NULL;
			}
		}
	}
// 	ninclen ++;
// 	nincdatalen += dataLen;
// 	DWORD timenow = timeGetTime();
// 	if (timenow - timelast >= 1000)
// 	{
// 		timelast = timenow;
// 		char loginfo1[128] = { 0 };
// 		snprintf(loginfo1, 128, "hook len:[%d]  [%d]  [%d]  [%d]\n", nincdatalen, ninclen, format->nSamplesPerSec, format->nAvgBytesPerSec);
// 		OutputDebugStringA(loginfo1);
// //		writelog("hook len:[%d] fps[%d]/s sample:%d\n", nincdatalen, ninclen, format->nSamplesPerSec);
// 		ninclen = 0;
// 		nincdatalen = 0;
// 	}
// 	FILE * outfile = fopen("e:/playerout.pcm", "ab+");
// 	if (outfile)
// 	{
// 		fwrite(data, 1, dataLen, outfile);
// 		fclose(outfile);
// 		outfile = NULL;
//	m_fileBkMusicSrc.write((char*)data, dataLen);
// 	}
// 	else
// 	{
// 		OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
// 	}
}


AgoraPlayerManager::AgoraPlayerManager()
{
	this->pCircleBuffer = new CicleBuffer(48000 * 2 * 2, 0);
	this->mpPlayerHooker = createPlayerHookerInstance();
	this->pPlayerData = new BYTE[0x800000];
	this->bHook = FALSE;
}

AgoraPlayerManager::~AgoraPlayerManager()
{
	delete this->pCircleBuffer;
	delete[] this->pPlayerData;
// 	if (mpAudioCaptureCallback)
// 	{
// 		delete mpAudioCaptureCallback;
// 		mpAudioCaptureCallback = NULL;
// 	}
	if (mpPlayerHooker)
	{
		delete mpPlayerHooker;
		mpPlayerHooker = NULL;
	}
}

BOOL AgoraPlayerManager::startHook(BOOL bstart, TCHAR* pPlayerPath)
{
	//clear circle buffer
	this->pCircleBuffer->flushBuffer();

	if (!bstart)
	{
		writelog("leave hook");
		mpPlayerHooker->stopAudioCapture();
		mpPlayerHooker->stopHook();
	}
	else
	{
		writelog("enter hook");
		mpPlayerHooker->startHook(pPlayerPath);
		mpPlayerHooker->startAudioCapture(pAgoraManager->mpAudioCaptureCallback);
	}
	this->bHook = bstart;
	return TRUE;
}

