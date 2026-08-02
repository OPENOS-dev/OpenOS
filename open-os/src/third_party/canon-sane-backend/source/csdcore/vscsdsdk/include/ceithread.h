/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CEI_THREAD_CLASS_USING_STD_HEADER__
#define __CEI_THREAD_CLASS_USING_STD_HEADER__

#include <thread>

#ifdef _WIN32
#include <memory>
typedef void *(LPCEITHREAD_FUNC)(void *param);
class ceithread
{
public:
	ceithread()
	{
	}
	~ceithread()
	{
	}
	int create(LPCEITHREAD_FUNC lpfn, void *param)
	{
		m_th.reset(new std::thread(lpfn, param));
		return 0;
	}
	void join()
	{
		if (m_th.get()) {
			m_th->join();
		}
	}
	bool joinable()
	{
		if (m_th.get()) return m_th->joinable();
		return false;
	}
private:
	std::unique_ptr< std::thread >m_th;
};
#else
typedef void *(LPCEITHREAD_FUNC)(void *param);
class ceithread
{
public:
	ceithread() :m_joinable(false) {}
	~ceithread() {
		if (joinable()) join();
	}

	int create(LPCEITHREAD_FUNC lpfn, void *param)
	{
		m_joinable = true;
		int ret = pthread_create(&m_tid, NULL, lpfn, param);
		if (ret) m_joinable = false;
		return ret;
	}
	void join()
	{
		if (m_joinable) pthread_join(m_tid, NULL);
		m_joinable = false;
	}
	bool joinable()
	{
		return m_joinable;
	}
private:
	pthread_t m_tid;
	bool m_joinable;
};
#endif

#endif