#include <memory>
#include <iostream>

struct B;

struct A 
{
    A(); 
    
    std::shared_ptr<B> p;
};

struct B 
{
    B();
    
    std::shared_ptr<A> p;
};

A::A() : p (new B{}) {}
B::B() : p (new A{}) {}

struct D;

struct C
{
    std::shared_ptr<D> p;
};

struct D
{
    std::shared_ptr<C> p;
};

void test1()
{    
    A a;
    B b;
}

void test2()
{
	auto pD = std::make_shared<D>();
	auto pC = std::make_shared<C>();

	pD->p = pC;
	pC->p = pD;
}

int main()
{
    test2();

    return 0;
}
