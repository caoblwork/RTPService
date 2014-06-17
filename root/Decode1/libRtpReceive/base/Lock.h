#ifndef _LOCK_H_
#define _LOCK_H_

#include "Base.h"
#include <windows.h>
namespace base{

class Lock
{
public:
    Lock();

    ~Lock();

    bool Try();

    void Acquire();

    void Release();

private:
    CRITICAL_SECTION cs_;

    DISALLOW_COPY_AND_ASSIGN(Lock);
};

//
// Lock������
// ���캯��-����
// ��������-�ͷ���
//
class AutoLock {
public:
    explicit AutoLock(Lock& lock) : lock_(lock) {
        lock_.Acquire();
    }

    ~AutoLock() {
        lock_.Release();
    }

private:
    Lock& lock_;
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
};

//
// Lock������
// ���캯��-�ͷ���
// ��������-����
//
class AutoUnlock {
public:
    explicit AutoUnlock(Lock& lock) : lock_(lock) {
        lock_.Release();
    }

    ~AutoUnlock() {
        lock_.Acquire();
    }

private:
    Lock& lock_;
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
};

} // namespace Base

#endif // _LOCK_H_
