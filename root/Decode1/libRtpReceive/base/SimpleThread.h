#ifndef _SIMPLE_THREAD_H_
#define _SIMPLE_THREAD_H_
#include <windows.h>
namespace base
{

class SimpleThread
{
public:
    SimpleThread();

    virtual ~SimpleThread();

    // �����߳�, ��֤Start()��������ʱ���߳��Ѿ���ʼִ��
    virtual void Start();

    virtual void Stop();

    virtual bool IsStop();

    // �ȴ��̹߳ر�
    virtual void Join();

    // ������Ҫ���ش˺���
    // Run()�����������½����߳���
    virtual void Run() = 0;

    HANDLE ThreadId() { return thread_id_; }

    void ThreadMain();

private:
    HANDLE thread_id_;
    bool is_stop_;
};

} // namespace Thread

#endif // _SIMPLE_THREAD_H_
