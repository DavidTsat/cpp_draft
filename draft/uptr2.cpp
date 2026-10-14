#include <functional>
#include <utility>

#include <iostream>

using namespace std;

namespace Priv
{
	template <typename T>
	struct DefaultDel
	{
		void operator()(T* p)
		{
			delete p;
		}
	};
};

//template <typename T, template<typename> class D=Priv::DefaultDel>
template <typename T, class D=Priv::DefaultDel<T>>
class Uptr : private D
{
	typedef T* PointerType;
	typedef T& ReferenceType;
	typedef D DeleterType;
public:
	Uptr() noexcept : DeleterType(), ptr_{} {}

	explicit Uptr(PointerType ptr) noexcept : ptr_(ptr) {}
	
	Uptr(PointerType ptr, DeleterType d) noexcept : DeleterType(d), ptr_(ptr) {}

	Uptr(Uptr&& rhs) noexcept : Uptr()
	{
		this->swap(rhs);
	}
	
	Uptr& operator=(Uptr&& rhs) noexcept
	{
		if (this == &rhs)
			return *this;
		
		reset(rhs.release());

		static_cast<DeleterType&>(*this) = std::move(static_cast<DeleterType&>(rhs));
		//todo
		//d_ = move(rhs.d_);

		return *this;
	}

	~Uptr() noexcept
	{
		destroy();
	}
	
	PointerType operator->() noexcept
	{
		return ptr_;
	}

	ReferenceType operator*() noexcept
	{
		return *ptr_;
	}

	PointerType operator->() const noexcept
	{
		return ptr_;
	}

	ReferenceType operator*() const noexcept
	{
		return *ptr_;
	}


	void reset(PointerType p = PointerType{}) noexcept
	{
		PointerType old = ptr_;
		ptr_ = p;

		if (old)
			(*this)(old);
	}
	
	PointerType release() noexcept
	{
		return exchange(ptr_, nullptr);
	}

	PointerType get() noexcept
	{
		return ptr_;
	}

	void swap(Uptr& rhs) noexcept
	{
		//DeleterType::swap(rhs);
		
		std::swap(static_cast<DeleterType&>(*this), static_cast<DeleterType&>(rhs));
		std::swap(ptr_, rhs.ptr_);
	}

	Uptr(const Uptr&) = delete;
	Uptr& operator=(const Uptr&) = delete;
private:
	PointerType ptr_;

	void destroy() noexcept
	{
		if (ptr_)
			(*this)(ptr_);
	}
};

template <typename T, typename... Args>
Uptr<T> make_uptr(Args&&... args)
{
	return Uptr<T>(new T(args...));
}

void test1()
{
	Uptr<string, function<void(string*)> > p1(nullptr, [](string* s){ cout << "deleting.\n"; delete s;});
	p1.reset(new string("abcdef"));

	cout << sizeof(p1) << endl;
	cout << *p1 << endl;

	Uptr<string, function<void(string*)> > p2(nullptr, [](string* s){ cout << "deleting2.\n"; delete s;});

//	p1.reset(p1.get());
	p1.reset(new string("ppppppppppp"));

	cout << *p1 << endl;
	p2 = move(p1);

	cout << *p2 << endl;
	p2.reset();

	p1 = Uptr<string, function<void(string*)> >(new string("4444"), [](string* s) {cout << "deleting3.\n"; delete s;});

	cout << *p1 << endl;
}

void test2()
{
	Uptr<string> p1;
	p1.reset(new string("abcdef"));

	cout << sizeof(p1) << endl;
	cout << *p1 << endl;

	Uptr<string> p2;

//	p1.reset(p1.get());
	p1.reset(new string("ppppppppppp"));

	cout << *p1 << endl;
	p2 = move(p1);

	cout << *p2 << endl;
	p2.reset();

	p1 = Uptr<string>(new string("4444"));

	cout << *p1 << endl;
}

void test3()
{
	Uptr<string> p1;
	p1 = make_uptr<string>("abcdef");

	cout << sizeof(p1) << endl;
	cout << *p1 << endl;

	Uptr<string> p2;

//	p1.reset(p1.get());
	p1 = make_uptr<string>("ppppppppppp");

	cout << *p1 << endl;
	p2 = move(p1);

	cout << *p2 << endl;
	p2.reset();

	p1 = make_uptr<string>("4444");

	cout << *p1 << endl;
}


int main()
{
	test1();
	test2();
	test3();

	return 0;
}
