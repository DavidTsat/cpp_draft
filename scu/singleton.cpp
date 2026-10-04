#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <mutex>
#include <iostream>

using namespace std;

namespace Private
{
	template <typename T>
	struct Deleter
	{
		void operator()(T* p)
		{
			delete p;
		}
	};

	class LifetimeTracker
	{
	public:
		explicit LifetimeTracker(unsigned x) : l_(x) {}
		virtual ~LifetimeTracker() = 0;
		friend inline bool Compare(unsigned x, LifetimeTracker* p);
	private:
		unsigned l_;
	};

	inline LifetimeTracker::~LifetimeTracker() {}
	
	inline bool Compare(unsigned x, LifetimeTracker* p)
	{
		return p->l_ > x;
	}


	template <typename T, typename D>
	class ConcreteLifetimeTracker : public LifetimeTracker
	{
	public:
		ConcreteLifetimeTracker(T* pObj, unsigned l, D d) : LifetimeTracker(l), pTracked_(pObj), deleter_(d)
		{
		}
		~ConcreteLifetimeTracker() override
		{
			deleter_(pTracked_);
		}
	private:
		T* pTracked_;
		D deleter_;
	};

	typedef LifetimeTracker** TrackerArray;
	TrackerArray pTrackerArray;
	unsigned elements;
	
	void AtExitFn()
	{
		assert(elements != 0 && pTrackerArray != 0);

		LifetimeTracker* pLast = pTrackerArray[elements - 1];
		pTrackerArray = static_cast<TrackerArray>(realloc(pTrackerArray, sizeof(*pTrackerArray) * --elements));
		delete pLast;
	}

	template <class T>
	class CreateUsingNew;
	
	template <class T>
	class DefaultLifetime;

	template <class T>
	class SingleThreaded;
};

template <typename T, typename D=Private::Deleter<T>>
void setLongevity(T* pObj, unsigned l, D d = Private::Deleter<T>())
{
	using namespace Private;

	TrackerArray pNew = static_cast<TrackerArray>(realloc(pTrackerArray, sizeof(*pTrackerArray)*(elements + 1)));
	if (!pNew)
		throw bad_alloc();

	pTrackerArray = pNew;

	LifetimeTracker* p = new ConcreteLifetimeTracker(pObj, l, d);

	TrackerArray pos = upper_bound(pTrackerArray, pTrackerArray + elements, l, Compare);
	
	copy_backward(pos, pTrackerArray + elements, pTrackerArray + elements + 1);
	*pos = p;
	++elements;
	atexit(AtExitFn);
}

// phoenix singleton
// 
class Singleton
{
public:
	static Singleton& Instance()
	{
		if (!pInstance)
		{
			if (isDestroyed)
				OnDeadReference();
			else
				Create();
		}
		return *pInstance;
	}

private:
	static void OnDeadReference()
	{
		//runtime_error("Dead Reference!");
		
		Create();

		new (pInstance) Singleton;
		atexit(KillPhoenixSingleton);
		isDestroyed = false;
	}

	static void Create()
	{
		alignas(Singleton) static unsigned char s[sizeof(Singleton)];
		pInstance = new (s) Singleton;
	}

	virtual ~Singleton()
	{
		isDestroyed = true;
		pInstance = 0;
	}
	
	static void KillPhoenixSingleton()
	{
		pInstance->~Singleton();
	}

	static bool isDestroyed;
	static Singleton* pInstance;
};

bool Singleton::isDestroyed = false;
Singleton* Singleton::pInstance = 0;

template
<
	class T,
	template <class> class CreationPolicy = Private::CreateUsingNew,
	template <class> class LifetimePolicy = Private::DefaultLifetime,
	template <class> class ThreadingModel = Private::SingleThreaded
>
class SingletonHolder
{
public:
	static T& instance()
	{
		if (!pInst)
		{
			typename ThreadingModel<T>::LockGuard lg;
			if (!pInst)
			{
				if (destroyed_)
				{
					LifetimePolicy<T>::OnDeadReference();
					destroyed_ = false;
				}

				pInst = CreationPolicy<T>::Create();
				LifetimePolicy<T>::ScheduleDestroyer(&DestroySingleton);
			}
		}

		return *pInst;
	}
private:
	static void DestroySingleton()
	{
		assert(!destroyed_);
		CreationPolicy<T>::Destroy(pInst);
		pInst = 0;
		destroyed_ = true;
	}

	SingletonHolder();
	//...
	typedef typename ThreadingModel<T>::VolatileType InstanceType;
	static InstanceType* pInst;
	static bool destroyed_;
};


class Log
{
public:
	volatile static Log& instance()
	{
		Create();
		return *pInst;
	}

	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;

private:
	Log() = default;

	static void Create()
	{
		if (!pInst)
		{
			lock_guard lg(m);
			if (!pInst)
			{
				pInst = new Log();
				setLongevity(pInst, l);
			}
		}
	}
private:
	static mutex m;
	static unsigned l;
	volatile static Log* pInst;
};

mutex Log::m;
unsigned Log::l = 0;
volatile Log* Log::pInst = nullptr;


int main()
{
	(void)Log::instance();
	
	return 0;
}
