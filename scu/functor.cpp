#include "type_list.h"

#include <string>
#include <cstring>
#include <iostream>

using namespace TL;
using namespace std;


template <typename R, typename TList>
class FunctorImpl;

template <typename R>
class FunctorImpl<R, NullType>
{
public:
	virtual ~FunctorImpl() {}
	virtual R operator()() = 0;
	virtual FunctorImpl* clone() const = 0;
};

template <typename R, typename T>
class FunctorImpl<R, TYPELIST_1(T)>
{
public:
	virtual ~FunctorImpl() {}
	virtual R operator()(const T&) = 0;
	virtual FunctorImpl* clone() const = 0;
};


template <typename R, typename T1, typename T2>
class FunctorImpl<R, TYPELIST_2(T1, T2)>
{
public:
	virtual ~FunctorImpl() {}
	virtual R operator()(const T1&, const T2&) = 0;
	virtual FunctorImpl* clone() const = 0;
};

template <typename ParentFunctor, typename Fun>
class FunctorHandler : public FunctorImpl<typename ParentFunctor::ResultType, typename ParentFunctor::ParamList>
{
	typedef typename ParentFunctor::ResultType R;
	typedef typename ParentFunctor::Parm1 Parm1;
	typedef typename ParentFunctor::Parm2 Parm2;
public:
	FunctorHandler(const Fun& f) : f_(f) {}

	FunctorHandler* clone() const override
	{
		return new FunctorHandler(f_);
	}
	
	R operator()()
	{
		return f_();
	}

	R operator()(const Parm1& pm1)
	{
		return f_(pm1);
	}

	R operator()(const Parm1& pm1, const Parm2& pm2)
	{
		return f_(pm1, pm2);
	}

	~FunctorHandler() {}
private:
	Fun f_;
};

template <class ParentFunctor, typename PointerToObj, typename PointerToMemFn>
class MemFunHandler : public FunctorImpl<typename ParentFunctor::ResultType, typename ParentFunctor::ParamList>
{
	typedef typename ParentFunctor::ResultType R;
	typedef typename ParentFunctor::Parm1 Parm1;
	typedef typename ParentFunctor::Parm2 Parm2;

public:
	MemFunHandler(PointerToObj pO, PointerToMemFn pM) : pObj(pO), pMemFn(pM) {}
	
	MemFunHandler* clone() const override
	{
		return new MemFunHandler(pObj, pMemFn);
	}

	R operator()() 
	{
		return ((*pObj).*pMemFn)();
	}

	R operator()(Parm1 pm1)
	{
		return ((*pObj).*pMemFn)(pm1);
	}

	R operator()(Parm1 pm1, Parm2 pm2)
	{
		return ((*pObj).*pMemFn)(pm1, pm2);
	}

	~MemFunHandler() {}
private:
	PointerToObj pObj;
	PointerToMemFn pMemFn;
};

template <typename R, typename TList>
class Functor
{
public:
	typedef R ResultType;
	typedef TList ParamList;

	typedef typename TypeAtNonStrict<ParamList, 0>::Result Parm1;
	typedef typename TypeAtNonStrict<ParamList, 1>::Result Parm2;
	
	typedef FunctorImpl<R, TList> Impl;
public:
	Functor(const Functor& f) : pFImpl(f.pFImpl ? f.pFImpl->clone() : nullptr)
	{
	}

	explicit Functor(Impl* p) : pFImpl(p)
	{
	}

	template <typename Fun>
	explicit Functor(Fun f) : pFImpl(new FunctorHandler<Functor, Fun>(f))
	{
	}
	
	template <typename TObjP, typename TObjMem>
	explicit Functor(TObjP tp, TObjMem mF) : pFImpl(new MemFunHandler<Functor, TObjP, TObjMem>(tp, mF))
	{
	}

	Functor& operator=(const Functor& f)
	{
		Functor f2(f);
		swap(pFImpl, f2.pFImpl);
		return *this;
	}

	R operator()()
	{
		return (*pFImpl)();
	}

	R operator()(Parm1 a)
	{
		return (*pFImpl)(a);
	}

	R operator()(Parm1 a, Parm2 b)
	{
		return (*pFImpl)(a, b);
	}

	~Functor()
	{
		delete pFImpl;
	}

private:
	Impl* pFImpl;
};


int f1(string s, char c)
{
	cout << s + string(1, c) << endl;

	return strlen(s.c_str()) + c;
}

int f1(string s)
{
	cout << s << endl;
	return strlen(s.c_str());
}

class Parrot
{
public:
   void eat()
   {
      cout << "Tsk, knick, tsk...\n";
   }
   void speak()
   {
      cout << "Oh Captain, my Captain!\n";
   }
};

int main()
{
	typedef Functor<int, TYPELIST_2(string, char)> FunctorTest1;
	typedef Functor<int, TYPELIST_1(string)> FunctorTest2;
	typedef void (Parrot::* TpMemFun) ();

	typedef int (*F1)(string, char);
	typedef int (*F2)(string);

	FunctorTest1 f1t(static_cast<F1>(f1));
	FunctorTest2 f2t(static_cast<F2>(f1));

	cout << f1t("David", 'c') << endl << endl;
	cout << f2t("David") << endl;
	

	TpMemFun pActivity = &Parrot::eat;
	
	Parrot geronimo;
	Parrot* pGeronimo = &geronimo;

	(geronimo.*pActivity)();
	(pGeronimo->*pActivity)();

	pActivity = &Parrot::speak;
	
	(geronimo.*pActivity)();

	Functor<void, NullType> 
		cmd1(&geronimo, &Parrot::eat),
		cmd2(&geronimo, &Parrot::speak);
	
	cout << endl;

	cmd1();
	cmd2();

	return 0;
}
