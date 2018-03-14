#include "stdafx.h"
#include "ExtendAudioFrameObserver.h"

#include "AgoraManager.h"
//#include <timeapi.h>
extern AgoraManager*	pAgoraManager;
//#include "../PlayerHooker/Utils.h"

CExtendAudioFrameObserver::CExtendAudioFrameObserver()
{
	pPlayerData = new BYTE[0x800000];
	DeleteFile(_T("./V6room/MusicDest.pcm"));
	DeleteFile(_T("./V6room/FrameMix.pcm"));
	DeleteFile(_T("./V6room/PlayOut.pcm"));
}

CExtendAudioFrameObserver::~CExtendAudioFrameObserver()
{
	delete[] pPlayerData;
}

static inline int16_t MixerAddS16(int16_t var1, int16_t var2) {
	static const int32_t kMaxInt16 = 32767;
	static const int32_t kMinInt16 = -32768;
	int32_t tmp = (int32_t)var1 + (int32_t)var2;
	int16_t out16;

	if (tmp > kMaxInt16) {
		out16 = kMaxInt16;
	}
	else if (tmp < kMinInt16) {
		out16 = kMinInt16;
	}
	else {
		out16 = (int16_t)tmp;
	}

	return out16;
}

void MixerAddS16(int16_t* src1, const int16_t* src2, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		src1[i] = MixerAddS16(src1[i], src2[i]);
	}
}

BOOL mixAudioData(char* psrc, char* pdst, int datalen)
{
	if (!psrc || !pdst || datalen <= 0)
	{
		return FALSE;
	}

	for (int i = 0; i < datalen; i++)
	{
		pdst[i] += psrc[i];
	}
	return TRUE;
}

//DWORD timeold1 = timeGetTime();
// int timeinc1 = 0;
// FILE * outfile = NULL;
bool CExtendAudioFrameObserver::onRecordAudioFrame(AudioFrame& audioFrame)
{
	SIZE_T nSize = audioFrame.channels*audioFrame.samples * 2;

	if (bIsDebugMode){

		static int nCountAudioCallBack = 0;
		nCountAudioCallBack++;
		static DWORD dwLastStamp = GetTickCount();
		DWORD dwCurrStamp = GetTickCount();
		if (5000 < dwCurrStamp - dwLastStamp){

			float fRecordAudioFrame = nCountAudioCallBack * 1000.0 / (dwCurrStamp - dwLastStamp);
			char logMsg[128] = { '\0' };
			sprintf_s(logMsg, "RecordAudioFrame :%d , %d,16,%d [ Rate : %.2f]\n", nSize,audioFrame.channels, audioFrame.samplesPerSec, fRecordAudioFrame);
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

	unsigned int datalen = 0;
	pAgoraManager->pPlayerCaptureManager->getCircleBufferObject()->readBuffer(this->pPlayerData, nSize, &datalen);

	if (bIsSaveDumpPcm)
	{
		FILE* outfile1 = fopen("./V6room/MusicDest.pcm", "ab+");
		if (outfile1)
		{
			fwrite(this->pPlayerData, 1, datalen, outfile1);
			fclose(outfile1);
			outfile1 = NULL;
		}
	}

//	memcpy(this->pPlayerData, pAgoraManager->pPlayerCaptureManager->pPlayerData, pAgoraManager->pPlayerCaptureManager->nPlayerDataLen);
	int nMixLen = nSize;
	if (nSize > 0 && datalen > 0)
	{
		int nMixLen = datalen > nSize ? nSize : datalen;

// 		char loginfo[128] = { 0 };
// 		snprintf(loginfo, 128, "len_need[%d] len[%d] \n", nSize, datalen);
// 		OutputDebugStringA(loginfo);

// 		FILE* outfile = fopen("e:/player.pcm", "ab+");
// 		if (outfile)
// 		{
// 			fwrite(this->pPlayerData, 1, datalen, outfile);
// 			fclose(outfile);
// 			outfile = NULL;
// 		}
		//mixAudioData((char*)this->pPlayerData, (char*)audioFrame.buffer, nMixLen);
		MixerAddS16((int16_t*)audioFrame.buffer, (int16_t*)pPlayerData, (audioFrame.channels * audioFrame.bytesPerSample) *  audioFrame.samples / sizeof(int16_t));
	}

	if (bIsSaveDumpPcm)
	{
		FILE* outfile = fopen("./V6room/FrameMix.pcm", "ab+");
		if (outfile)
		{
			fwrite(audioFrame.buffer, 1, nMixLen, outfile);
			fclose(outfile);
			outfile = NULL;
		}
	}
	//	CAudioCapturePackageQueue::GetInstance()->PopAudioPackage(audioFrame.buffer, &nSize);
	
	return true;
}

bool CExtendAudioFrameObserver::onPlaybackAudioFrame(AudioFrame& audioFrame)
{
	SIZE_T nSize = audioFrame.channels*audioFrame.samples * 2;
//	CAudioPlayPackageQueue::GetInstance()->PushAudioPackage(audioFrame.buffer, nSize);
// 	FILE * recordf = fopen("d:/playback.pcm", "ab+");
// 	fwrite(audioFrame.buffer, 1, nSize, recordf);
// 	fclose(recordf);
#if 0
	if (bIsDebugMode)
	{
		FILE* outfile1 = fopen("./V6room/PlayOut.pcm", "ab+");
		if (outfile1)
		{
			fwrite(this->pPlayerData, 1, nSize, outfile1);
			fclose(outfile1);
			outfile1 = NULL;
		}
	}
#endif
	
	return true;
}

bool CExtendAudioFrameObserver::onMixedAudioFrame(AudioFrame& audioFrame)
{
	return true;
}

bool CExtendAudioFrameObserver::onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame& audioFrame)
{
	return true;
}
