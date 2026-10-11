#include <algorithm>
#include <utility>
#include <iostream>

using namespace std;

namespace Private
{
	class RefLinkingRingBase
	{
	protected:
		RefLinkingRingBase() : prev(this), next(this) {}
		
		RefLinkingRingBase(const RefLinkingRingBase& rhs) 
		{
			prev = const_cast<RefLinkingRingBase*>(&rhs);
			next = rhs.next;
			next->prev = this;
			prev->next = this;			
		}

		bool Release()
		{
			if (prev == this && next == this)
				return true;

			prev->next = next;
			next->prev = prev;
			return false;
		}

		void Swap(RefLinkingRingBase& rhs) 
		{
			if (this == &rhs) 
				return;

			swap(prev, rhs.prev);
			swap(next, rhs.next);

			prev->next = this;
    			next->prev = this;

    			rhs.prev->next = &rhs;
    			rhs.next->prev = &rhs;

/*		
			if (prev)
				prev->next = this;
			if (next)
				next->prev = this;

			if (rhs.prev)
				rhs.prev->next = &rhs;
			if (rhs.next)
				rhs.next->prev = &rhs;
*/
		}

	private:
		mutable const RefLinkingRingBase* prev;
		mutable const RefLinkingRingBase* next;
	};
};

template <class T>
class RefLinkingRing : public Private::RefLinkingRingBase
{
public:
	RefLinkingRing() {}

	RefLinkingRing(const RefLinkingRing& rhs) : Private::RefLinkingRingBase(rhs)
	{
	}

	static T Clone(const T& v)
	{
		return v;
	}

	bool Release(const T& _) 
	{
		return Private::RefLinkingRingBase::Release();
	}
	
	enum {SwapAllowed = false};
};

template <class T>
class DestructiveCopy
{
public:
	static T Clone(const T& v)
	{
		return exchange(const_cast<T&>(v), T{});
	}

	static bool Release(const T& v)
	{
		return true;
	}

	void Swap(const DestructiveCopy&) {}

	enum {SwapAllowed = true };
};

template <class T>
class DefaultSPStorage
{
protected:
	typedef T* StoredType;
	typedef T* PointerType;
	typedef T& ReferenceType;

public:
	DefaultSPStorage() : pointee(Default()) {}
	explicit DefaultSPStorage(const StoredType& p) : pointee(p) {}
	
	PointerType operator->()
	{
		return pointee;
	}

	ReferenceType operator*()
	{
		return *pointee;
	}

	friend inline PointerType GetImpl(const DefaultSPStorage<T>& o)
	{
		return o.pointee;
	}
	
	friend inline const StoredType& GetImplRef(const DefaultSPStorage& o)
	{
		return o.pointee;
	}

	friend inline StoredType& GetImplRef(DefaultSPStorage& o)
	{
		return o.pointee;
	}

	void Swap(DefaultSPStorage& rhs)
	{
		swap(pointee, rhs.pointee);
	}
protected:
	void Destroy()
	{
		delete pointee;
	}

	static StoredType Default() 
	{
		return 0;
	}

private:
	StoredType pointee;
};

template <class T>
class RefCountingOwnPolicy
{
protected:
	RefCountingOwnPolicy() : pCnt(new unsigned(1)) {}

	T Clone(const T& v)
	{
		++*pCnt;
		return v;
	}

	bool Release(const T& v)
	{
		if (!--*pCnt)
		{
			delete pCnt;
			return true;
		}
		return false;
	}
	
	void Swap(RefCountingOwnPolicy& rhs)
	{
		swap(pCnt, rhs.pCnt);
	}

	enum {SwapAllowed = true};
private:
	unsigned* pCnt;
};

// todo: ref counting circular
//

template <class T>
class RefLinkingOwnPolicyLinear
{
protected:
	RefLinkingOwnPolicyLinear() : prev(0), next(0) {}
	
	RefLinkingOwnPolicyLinear(RefLinkingOwnPolicyLinear& rhs) : prev(&rhs), next(rhs.next)
	{
		if (rhs.next)
			rhs.next->prev = this;
		rhs.next = this;
	}

	T Clone(const T& v)
	{
//		RefLinkingOwnPolicy* p = new RefLinkingOwnPolicy();
/*
		if (prev)
			prev->next = this;
		if (next)
		{
			next->prev = this;
		}
		//next = next;
		//p->prev = prev;
*/

		return v;
	}

	bool Release(const T& v)
	{
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = next;

		return !prev && !next;
	}

	void Swap(RefLinkingOwnPolicyLinear& rhs)
	{
		//todo: bug in case of adjacent nodes.
		//

		swap(prev, rhs.prev);
		swap(next, rhs.next);
		
		if (prev)
			prev->next = this;
		if (next)
			next->prev = this;

		if (rhs.prev)
			rhs.prev->next = &rhs;
		if (rhs.next)
			rhs.next->prev = &rhs;
	}

	enum {SwapAllowed = false};
private:
	RefLinkingOwnPolicyLinear* prev;
	RefLinkingOwnPolicyLinear* next;
};

template <
	typename T,
	template <class> class OwnershipPolicy = RefCountingOwnPolicy,
	template <class> class StoragePolicy = DefaultSPStorage	
	>
class SmartPtr : public StoragePolicy<T>, public OwnershipPolicy<typename StoragePolicy<T>::PointerType> 
{
	typedef StoragePolicy<T> SP;
	typedef OwnershipPolicy<typename SP::PointerType> OP;
public:
	typedef typename SP::PointerType PointerType;
	typedef const typename SP::PointerType ConstPointerType;
        typedef typename SP::StoredType StoredType;
        typedef typename SP::ReferenceType ReferenceType;
        typedef const typename SP::ReferenceType ConstReferenceType;

	SmartPtr(const StoredType& ptr) : SP(ptr), OP() {}
	
	SmartPtr(SmartPtr& rhs) : SP(rhs), OP(rhs)
	{
		GetImplRef(*this) = OwnershipPolicy<PointerType>::Clone(GetImplRef(rhs));
	}
	
	SmartPtr& operator=(SmartPtr& rhs)
	{
		SmartPtr sp(rhs);
		Swap(sp);
		return *this;
	}

	~SmartPtr()
	{
		if (OP::Release(GetImpl(*this)))
			SP::Destroy();
	}

	PointerType operator->()
	{
		return SP::operator->();
	}
	
	ConstPointerType operator->() const
	{
		return SP::operator->();
	}

	ReferenceType operator*()
	{
		return SP::operator*();
	}

	ConstReferenceType operator*() const
	{
		return SP::operator*();
	}

	PointerType get() const
	{
		return GetImpl(*this);
	}

	void Swap(SmartPtr& rhs)
	{
	//	if constexpr (OP::SwapAllowed)
			OP::Swap(rhs);
		SP::Swap(rhs);
	}
private:
};

void test1()
{
	SmartPtr<int> sp(new int(7));
	
	SmartPtr<int> sp2(new int(14));

	sp = sp2;
	cout << *sp << endl;
	cout << *sp2 << endl;

	sp.Swap(sp2);
	cout << *sp << endl;
	cout << *sp2 << endl;
}

void test2()
{
	SmartPtr<int, RefLinkingOwnPolicyLinear> sp(new int(7));
	
	SmartPtr<int, RefLinkingOwnPolicyLinear> sp2(new int(14));

	sp = sp2;
	cout << *sp << endl;
	cout << *sp2 << endl;

//	sp.Swap(sp2);  --- BUG leaks memory !
	cout << *sp << endl;
	cout << *sp2 << endl;
}

void test3()
{
	SmartPtr<int, RefLinkingRing> sp(new int(7));
	
	SmartPtr<int, RefLinkingRing> sp2(new int(14));

	sp = sp2;
	cout << *sp << endl;
	cout << *sp2 << endl;

	//sp.Swap(sp2);  ---- BUG leaks memory !
	cout << *sp << endl;
	cout << *sp2 << endl;
}

void test4()
{
	SmartPtr<int, DestructiveCopy> sp(new int(7));
	
	SmartPtr<int, DestructiveCopy> sp2(new int(14));

	sp = sp2;
	cout << *sp << endl;
	SmartPtr<int, DestructiveCopy> sp3(sp);
	cout << *sp3 << endl;


//	cout << *sp2 << endl;

	sp3.Swap(sp);
	cout << *sp << endl;
	cout << *GetImpl(sp) << endl;
	cout << *(sp.get()) << endl;

}


int main()
{
	test1();
//	test2();
//	test3();
	test4();

	return 0;
}
