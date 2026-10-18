#include "shptr.h"

#include <iostream>

using namespace std;


void test1()
{
	ShPtr<int> p1(new int(7));
	ShPtr<int> p2(new int(8), [](int* a) { cout << "deleting p2\n"; delete a;});
	ShPtr<int> p3 = makeShPtr<int>(15);

	cout << *p1 << endl;
	cout << *p2 << endl;
	cout << *p3 << endl;
}

void test2()
{
	ShPtr<int> p1(new int(6));
	ShPtr<int> p2(new int(10));

	cout << *p1 << endl;
	cout << *p2 << endl;

	p1.swap(p2);

	cout << *p1 << endl;
	cout << *p2 << endl;

	ShPtr<int> p3(move(p2));
	cout << *p3 << endl;

	cout << "uc: " << p3.use_count() << endl;
	ShPtr<int> p4(p3);
	ShPtr<int> p5(p3);

	cout << *p3 << endl;
	cout << *p4 << endl;
	cout << *p5 << endl;

	cout << "uc: " << p5.use_count() << endl;
	cout << "uc: " << p4.use_count() << endl;
	cout << "uc: " << p3.use_count() << endl;

	p1 = p4;
	
	cout << *p1 << endl;
	cout << *p4 << endl;

	cout << "uc: " << p1.use_count() << endl;
	cout << "uc: " << p4.use_count() << endl;
	cout << "uc: " << p5.use_count() << endl;
	cout << "uc: " << p4.use_count() << endl;
	cout << "uc: " << p3.use_count() << endl;

	p1 = move(p4);
	cout << "--------------" << endl;
	
	cout << "uc: " << p1.use_count() << endl;
	cout << "uc: " << p4.use_count() << endl;
	cout << "uc: " << p5.use_count() << endl;
	cout << "uc: " << p4.use_count() << endl;
	cout << "uc: " << p3.use_count() << endl;

	ShPtr<int> p6 = makeShPtr<int>(55);

	p3 = p6;

	cout << "--------------" << endl;
	cout << *p3 << endl;
	cout << *p5 << endl;
	cout << p3.use_count() << endl;
	cout << p5.use_count() << endl;
	cout << p1.use_count() << endl;

}

void test3()
{
	Uptr<string, function<void(string*)> > p1(new string("David"), [](string* s){ cout << "deleting.\n"; delete s;});

	cout << *p1 << endl;

	ShPtr<string> p2(move(p1));

	cout << *p2 << endl;
}

//todo to BM
//

int main()
{
	test1();
	test2();
	test3();

	return 0;
}
