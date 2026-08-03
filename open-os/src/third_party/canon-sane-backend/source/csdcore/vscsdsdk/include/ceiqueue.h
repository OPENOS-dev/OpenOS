/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CEI_QUEUE_CLASS_USING_SEMAPHORE_STD_HEADER__
#define __CEI_QUEUE_CLASS_USING_SEMAPHORE_STD_HEADER__

#include <mutex>
#include <deque>
#ifdef _WIN32
#include <Windows.h>
#else
#include <semaphore.h>
#include <unistd.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <stdio.h>


//semaphore
#ifdef _WIN32
class cei_semaphore {
public:
	cei_semaphore():m_hd(NULL), m_count(0), m_count_max(0) 
	{
		m_name[0] = 0;
	}
	~cei_semaphore() {

	}
	void init(char *name, int init_count, int max_count)
	{
		strcpy(m_name, name);
		m_hd = CreateSemaphore(NULL, init_count, max_count, m_name);
		m_count = init_count;
		m_count_max = max_count;
	}
	void init(int init_count, int max_count)
	{
		if (!m_name[0]) sprintf(m_name, "/0x%p", this);
		m_hd = CreateSemaphore(NULL, init_count, max_count, m_name);
		m_count = init_count;
		m_count_max = max_count;
	}
	void lock() 
	{
		if (m_hd) WaitForSingleObject(m_hd, INFINITE);
		m_count++;
	}
	bool try_lock() 
	{ 
		if (m_hd==NULL) return false;
		if (m_count < m_count_max) {
			return true;
		}
		else {
			return false;
		}
	}
	void unlock() 
	{
		if (m_hd) ReleaseSemaphore(m_hd, 1, NULL);
		m_count--;
	}
private:
	HANDLE m_hd;
	int m_count;
	int m_count_max;
	char m_name[256];
};
#elif __ANDROID__
class cei_semaphore {
public:
    cei_semaphore():m_count(1), m_count_max(1) {}
    ~cei_semaphore() {}
    void init(char *, int init_count, int max_count)
    {
        init(init_count, max_count);
    }
    void init(int init_count, int max_count)
    {
        m_sem.sem_open(init_count);
        m_count = init_count;
        m_count_max = max_count;
    }
private:
    // P-operation / acquire
    void wait()
    {
        m_sem.sem_wait();
        m_count++;
    }
    bool try_wait()
    {
        if (m_count < m_count_max) {
            return true;
        }
        else {
            return false;
        }
    }
    // V-operation / release
    void signal()
    {
        m_sem.sem_post();
        m_count--;
    }
public:
    //Lockable requirements
    void lock() { wait(); }
    bool try_lock() { return try_wait(); }
    void unlock() { signal(); }
private:
    class CSem
    {
    public:
        CSem():m_count(0){}
        ~CSem(){}
        void sem_open(int init_count)
        {
            m_count=init_count;
        }
        void sem_wait()
        {
            while (check()) sleep(1);
            {
                std::lock_guard< std::mutex > lg(m_mt);
                m_count--;
            }
        }
        void sem_post()
        {
            std::lock_guard< std::mutex > lg(m_mt);
            m_count++;
        }
    private:
        bool check()
        {
            std::lock_guard< std::mutex > lg(m_mt);
            return m_count<=0;
        }
        int m_count;
        std::mutex m_mt;//critical section
    }m_sem;
    int m_count;
    int m_count_max;
};
#else
class cei_semaphore {
public:
	cei_semaphore() {
		m_count = 1;
		m_count_max = 1;
		m_init_done = false;
		m_sem = NULL;
		m_name[0] = 0;
	}
	~cei_semaphore() {
		if (m_sem) {
			sem_close(m_sem);
			sem_unlink(m_name);
		}
	}
	void init(char *name, int init_count, int max_count)
	{
		strcpy(m_name, name);
		init(init_count, max_count);
	}
	void init(int init_count, int max_count)
	{
		if (!m_name[0]) snprintf(m_name, sizeof(m_name), "/0x%p", this);
		m_sem = sem_open(m_name, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, init_count);
		if (m_sem == NULL) {
			printf("sem_open() error \r\n");
			return;
		}
		m_count = init_count;
		m_count_max = max_count;
		m_init_done = true;
	}
private:
	// P-operation / acquire
	void wait()
	{
		if (!m_init_done) return;
		sem_wait(m_sem);
		m_count++;

	}
	bool try_wait()
	{
		if (!m_init_done) return false;
		if (m_count < m_count_max) {
			return true;
		}
		else {
			return false;
		}
	}
	// V-operation / release
	void signal()
	{

		if (!m_init_done) return;
		sem_post(m_sem);
		m_count--;

	}
public:
	//Lockable requirements
	void lock() { wait(); }
	bool try_lock() { return try_wait(); }
	void unlock() { signal(); }

private:
	sem_t *m_sem;
	char m_name[256];
	bool m_init_done;
	int m_count;
	int m_count_max;
};
#endif

//キュークラス
template<class _Ty>
class CCeiQueue
{
public:
	CCeiQueue(){}
	virtual ~CCeiQueue(){}
	void init(long max_count)
	{
        char s[256];
        snprintf(s, sizeof(s), "/c%p", this);
        m_count.init(s, 0, (int)max_count);
        snprintf(s, sizeof(s), "/r%p", this);
		m_reset_count.init(s, (int)max_count, (int)max_count);
	}
	void peek(_Ty& value, long order/*from 1*/) {
		if ((long)m_queue.size()>=(long)order) {
			std::lock_guard< std::mutex > lg(m_mt);
			typename std::deque< _Ty >::iterator itr = m_queue.begin();
			for (long i=1; itr!=m_queue.end(); itr++, i++) {
				if (order==i) {
					value = *itr;
					return;
				}
			}
		}
	}
	void pop(_Ty& value) 
	{
		m_count.lock();
		{
			std::lock_guard< std::mutex > lg(m_mt);
			value = m_queue.front();
			m_queue.pop_front();
		}
 		m_reset_count.unlock();
	}
	void push(_Ty value) 
	{ 
		m_reset_count.lock();
		{
			std::lock_guard< std::mutex > lg(m_mt);
			m_queue.push_back(value); 
		}				
		m_count.unlock();
	}
	long count() {
		std::lock_guard< std::mutex > lg(m_mt);
		return (long)m_queue.size();
	}
protected:
	std::deque<_Ty> m_queue;
	cei_semaphore m_count;//semaphore
	cei_semaphore m_reset_count;//semaphore
	std::mutex m_mt;//critical section
};
#endif
