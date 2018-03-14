#pragma once

class rwlock
{
public:
    rwlock();
    ~rwlock();

    void WaitToRead();        // Call this to gain shared read access
    void WaitToWrite();       // Call this to gain exclusive write access
    void Done();              // Call this when done accessing the resource

private:
    CRITICAL_SECTION m_cs;
    HANDLE m_hsemReaders;
    HANDLE m_hsemWriters;
    int m_nWaitingReaders;
    int m_nWaitingWriters;
    int m_nActive;
};
