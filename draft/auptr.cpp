#include <memory>
#include <string>
#include <cassert>

#include <iostream>

using namespace std;

namespace Priv
{
	template <typename T>
	struct Deleter
	{
		void operator()(T* p) const noexcept
		{
			default_delete<T>()(p);
		}
	};
};

template <typename T>
class AuPtr : Priv::Deleter<T>
{
	typedef Priv::Deleter<T> Deleter;

	typedef T* PtrType;
	typedef T& RefType;

	struct Test
	{
		static void operator delete(void*) = delete;
	};
public:
	AuPtr() : ptr_{} {}
	explicit AuPtr(PtrType ptr) : ptr_(ptr) {}

	AuPtr(AuPtr& rPtr) noexcept : ptr_(rPtr.ptr_) {rPtr.ptr_ = {};};
	AuPtr(AuPtr&& rPtr) = delete;
	
	AuPtr& operator=(AuPtr& rPtr) noexcept
	{
		if (this != &rPtr)
		{
			Deleter(ptr_);
			ptr_ = rPtr.ptr_;
			rPtr.ptr_ = {};
		}

		return *this;
	}

	AuPtr& operator=(AuPtr&& rPtr) noexcept = delete;

	~AuPtr() noexcept
	{
		Deleter(ptr_);
	}

	PtrType operator->() noexcept
	{
		return ptr_;
	}

	PtrType operator->() const noexcept
	{
		return ptr_;
	}

	RefType operator*() noexcept
	{
		return *ptr_;
	}

	RefType operator*() const noexcept
	{
		return *ptr_;
	}

	PtrType get() noexcept
	{
		return ptr_;
	}
	
	PtrType get() const noexcept
	{
		return ptr_;
	}

	operator Test*() const noexcept
	{
		if (!ptr_)
			return nullptr;

		static Test t;
		return &t;
	}
private:
	PtrType ptr_;
};

void test1()
{
	AuPtr<string> p1(new string("David"));

	cout << sizeof(p1) << endl;
	static_assert(sizeof(p1) == sizeof(void*));
	cout << *p1 << endl;

	assert(p1);

	AuPtr<string> p2(p1);
	assert(!p1);
	assert(p2);

	cout << *p2 << endl;
}

int main()
{
	test1();

	return 0;
}
