#ifndef OSGEN_H
#define OSGEN_H



#ifdef __linux
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>

typedef unsigned long ulong;
typedef unsigned long THANDLE;
typedef const char kchar;

#define CRLF  "\n"

class umutex
{
    mutable pthread_mutex_t _mut;
public:
    umutex()
    {
        pthread_mutexattr_t     attr;

        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_mut, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    virtual ~umutex()
    {
        pthread_mutex_unlock(&_mut);
        pthread_mutex_destroy(&_mut);
    }

    int mlock() const
    {
        int err = pthread_mutex_lock(&_mut);
        return err;
    }

    int try_lock() const
    {
        int err =pthread_mutex_trylock(&_mut);
        return err;
    }

    int munlock() const
    {
        int err =pthread_mutex_unlock(&_mut);
        return err;
    }
};

class OsThread
{
public:
    OsThread()
    {
        _bstop   = 1;
        _hthread = 0;
        _joined  = false;
        _init = -1;
    }

    virtual ~OsThread()
    {
        if(!_stopped)
        {
            stop_thread();
        }

        if(_joined && _hthread)
        {
            pthread_detach(_hthread);
        }
        if(_init==0)
            pthread_attr_destroy(&_attr);
        _hthread = 0;

    }

    virtual int  start_thread()
    {
        _bstop   = 0;

        _init=pthread_attr_init(&_attr);
        pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_JOINABLE);
        if (pthread_create(&_hthread, &_attr, SFoo, this) != 0)
        {
            pthread_attr_destroy(&_attr);
            return errno;
        }
        pthread_attr_destroy(&_attr);
        usleep(1000);
        return 0;
    }

    virtual void signal_to_stop()
    {
        _bstop = 1;
    }
    virtual void    stop_thread()
    {
        _bstop = 1;
        sleep(1);
        if(0==_stopped)
        {
            t_join();
        }
    }

    bool  is_stopped()
    {
        return _bstop;
    }

    int t_join()
    {
        if(!_joined)
        {
            _joined = true;
            return pthread_join(_hthread,0);
        }
        return 0;
    }

protected:
    virtual void thread_main()=0;
    int         _bstop;
private:
    int         _init;
    pthread_attr_t  _attr;
    THANDLE     _hthread;
    int         _stopped;
    bool        _joined;
    static void* SFoo(void* pData)
    {
        OsThread* pT = reinterpret_cast<OsThread*>(pData);
        pT->_stopped = 0;
        pT->thread_main();
        pT->_stopped = 1;
        return 0;
    }
};


/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED <<x<< RST
#define FGRN(x) KGRN <<x<< RST
#define FYEL(x) KYEL <<x<< RST
#define FBLU(x) KBLU <<x<< RST
#define FMAG(x) KMAG <<x<< RST
#define FCYN(x) KCYN <<x<< RST
#define FWHT(x) KWHT <<x<< RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST



#elif _WIN32

#include <windows.h>
#include <process.h>

#define CRLF  "\r\n"

#define RST  ""
#define KRED  ""

struct umutex
{
    mutable CRITICAL_SECTION _cs;
    umutex(){
        _inout = 0;
        InitializeCriticalSection(&_cs);
    }
    ~umutex(){
        if(_inout) LeaveCriticalSection(&_cs);
        DeleteCriticalSection(&_cs);
    }
    int mlock() {
        if(_inout)
            return 1;
        EnterCriticalSection(&_cs);
        _inout=1;
        return 0;
    }
    int try_lock()
    {
         if(_inout)
            return 1;
        #if(_WIN32_WINNT >= 0x0400)
            if(TryEnterCriticalSection(&_cs))
            {
                _inout=1;
                return 1;
            }
            return 0;
        #else
            EnterCriticalSection(&_cs);
            _inout=1;
            return 1;
        #endif
    }
    int munlock() {
        if(0==_inout)
            return 1;
        _inout=0;
        LeaveCriticalSection(&_cs);
        return 1;
    }
    long _inout;
};

#define SAFE_CLOSEHANDLE(_handle)  if(VALID_HANDLE(_handle))\
                                      {\
                                          BOOL b = ::CloseHandle(_handle);\
                                          assert(b);\
                                          if(b)\
                                              _handle = INVALID_HANDLE_VALUE;\
                                      }
#define BEGINTHREADEX(psa, cbStack, pfnStartAddr, pvParam, fdwCreate, pdwThreadID)  \
                      ((HANDLE) _beginthreadex((void*)psa, (UINT)cbStack,             \
                                      (PTHREAD_START)(pfnStartAddr),                  \
                                      (void*)pvParam, (UINT)(fdwCreate),              \
                                     (UINT*)(pdwThreadID)))

class OsThread
{
public:
    OsThread()
    {
        _bstop   = 1;
        _hthread = 0;
        _joined  = false;
        _init = -1;
    }

    virtual ~OsThread()
    {
        if(!_stopped)
        {
            stop_thread();
        }

        _hthread = 0;

    }

    virtual int  start_thread()
    {
        DWORD nDummy = 0;
        _hthread = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&OsThread::SFoo,
                                                static_cast<void*>(this),
                                                CREATE_SUSPENDED, &nDummy);
        if(_hthread){
            ::ResumeThread(_hthread);
            return 0;
        }
        return errno;
    }

    virtual void signal_to_stop()
    {
        _bstop = 1;
    }
    virtual void    stop_thread()
    {
        DWORD dwRetVal=WaitForSingleObject(_hthread, 1000);
        if(WAIT_OBJECT_0 != dwRetVal)
        {
            if(!is_stopped())
            {
                ::TerminateThread(_hthread, 0);
            }
            ::CloseHandle(_hthread);
            _hthread = 0;
        }
    }

    bool  is_stopped()
    {
        return _bstop;
    }

    int t_join()
    {
       WaitForSingleObject(_hthread, 30000);
        return 0;
    }

protected:
    virtual void thread_main()=0;
    int         _bstop;
private:
    int         _init;
    HANDLE      _hthread;
    int         _stopped;
    bool        _joined;
    static void* SFoo(void* pData)
    {
        OsThread* pT = reinterpret_cast<OsThread*>(pData);
        pT->_stopped = 0;
        pT->thread_main();
        pT->_stopped = 1;
        return 0;
    }
};


#endif

class AutoLock
{
public:
    AutoLock(umutex* m):_mutex(m)
    {
         _mutex->mlock();
    }
    AutoLock(const umutex* m):_mutex((umutex*)m)
    {
         _mutex->mlock();
    }
    ~AutoLock()
    {
        _mutex->munlock();
    }
private:
    umutex* _mutex;
};



#ifdef __linux

inline void msleep(int t)
{
    ::usleep(t*1000);
}

inline time_t tick_count()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
            return 0;
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#elif _WIN32

#include <windows.h>
inline size_t tick_count()
{
    return GetTickCount();
}

inline void msleep(int t)
{
    Sleep(t);
}

#define FRED(x) x
#define FGRN(x) x
#define FYEL(x) x
#define FBLU(x) "Send : [" << x << "]\n"
#define FMAG(x) "Rec  : [" << x << "]\n"
#define FCYN(x) x
#define FWHT(x) x

#define BOLD(x) x
#define UNDL(x) x

#endif

#endif // OSGEN_H
