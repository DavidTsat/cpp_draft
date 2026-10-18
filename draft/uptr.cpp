#include "uptr.h"

#include <iostream>

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

int main()
{
	test1();

	return 0;
}
