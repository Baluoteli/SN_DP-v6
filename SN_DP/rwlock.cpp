#include "stdafx.h"
#include "rwlock.h"

rwlock::rwlock()
{
	m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
	m_hsemReaders = CreateSemaphoreA(NULL, 0, MAXLONG, NULL);
	m_hsemWriters = CreateSemaphoreA(NULL, 0, MAXLONG, NULL);
	InitializeCriticalSection(&m_cs);
}

rwlock::~rwlock()
{
// #ifdef _DEBUG
// 	if (m_nActive != 0)
// 		DebugBreak();
// #endif
	m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
	DeleteCriticalSection(&m_cs);
	CloseHandle(m_hsemReaders);
	CloseHandle(m_hsemWriters);
}

void rwlock::WaitToRead()
{
	EnterCriticalSection(&m_cs);

	BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

	if (fResourceWritePending)
	{
		m_nWaitingReaders++;
	}
	else
	{
		m_nActive++;
	}

// #ifdef _DEBUG
// 	printf("R: R=%d W=%d, A=%d\n", m_nWaitingReaders, m_nWaitingWriters, m_nActive);
// #endif
	LeaveCriticalSection(&m_cs);

	if (fResourceWritePending)
	{
		WaitForSingleObject(m_hsemReaders, INFINITE);
	}
}

void rwlock::WaitToWrite()
{
	EnterCriticalSection(&m_cs);

	BOOL fResourceOwned = (m_nActive != 0);
	if (fResourceOwned)
	{
		m_nWaitingWriters++;
	}
	else
	{
		m_nActive = -1;
	}

// #ifdef _DEBUG
// 	printf("W: R=%d W=%d, A=%d\n", m_nWaitingReaders, m_nWaitingWriters, m_nActive);
// #endif
	LeaveCriticalSection(&m_cs);

	if (fResourceOwned)
	{
		WaitForSingleObject(m_hsemWriters, INFINITE);
	}
}

void rwlock::Done()
{
	EnterCriticalSection(&m_cs);

	if (m_nActive > 0)
	{
		m_nActive--;
	}
	else
	{
		m_nActive++;
	}

	HANDLE hsem = NULL;
	LONG lCount = 1;

	if (m_nActive == 0)
	{
		if (m_nWaitingWriters > 0)
		{
			m_nActive = -1;
			m_nWaitingWriters--;
			hsem = m_hsemWriters;
		}
		else if (m_nWaitingReaders > 0)
		{
			m_nActive = m_nWaitingReaders;
			m_nWaitingReaders = 0;
			hsem = m_hsemReaders;
			lCount = m_nActive;
		}
		else
		{
			// There are no threads waiting at all; no semaphore gets released
		}
	}

// #ifdef _DEBUG
// 	printf("D: R=%d W=%d, A=%d C=%d\n", m_nWaitingReaders, m_nWaitingWriters, m_nActive, lCount);
// #endif
	LeaveCriticalSection(&m_cs);

	if (hsem != NULL)
	{
		ReleaseSemaphore(hsem, lCount, NULL);
	}
}
