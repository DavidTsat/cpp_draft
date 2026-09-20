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
class FunctorHandler : public FunctorImpl< typename ParentFunctor::ResultType, typename ParentFunctor::ParamList>
{
public:
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

	Functor operator=(const Functor& f)
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

int main()
{
	typedef Functor<int, TYPELIST_2(string, char)> FunctorTest1;

	FunctorTest1 f1t(f1);

	cout << f1t("David", 'c') << endl;

	return 0;
}
