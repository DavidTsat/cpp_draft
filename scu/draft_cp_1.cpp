#include "draft_cp_1.h"

#include <iostream>
#include <vector>

using namespace std;

int main()
{
        char c = 4;
        int* p = safe_reint_cast<int*>(&c);

        cout << uintptr_t(p) << endl;

        uint64_t a1 = 77;

 //       uint32_t a2 = safe_reint_cast<uint32_t>(a1);

	C cc;
	foo<false>(&cc);

	Widget* ptr = create<Widget>("abcd", Type2Type<Widget>{});
	string* pS = create<string>("bbbb", Type2Type<string>{});

	cout << ptr->n << endl;
	cout << *pS << endl;

	NiftyContainer<Widget, true> nc;
	
	nc.push(ptr);

	NiftyContainer<int, false> nc2;
        nc2.push(7);

	constexpr bool isConvU = sizeof(test(U())) == sizeof(Small);
	cout << "is conv U: " << isConvU << endl;
	constexpr bool isConvV = sizeof(test(makeV())) == sizeof(Small);
	cout << "is conv V: " << isConvV << endl;

	cout << Conversion<double, int>::exists << endl;
	cout << Conversion<int, double>::exists << endl;
	cout << Conversion<int, char*>::exists << endl;
	
	cout << "\nexists\n";

	cout << Conversion<int, int>::exists << endl;
	cout << Conversion<int, char*>::exists << endl;

	cout << "\ntype traits\n";
	cout << TypeTraits<int>::isPointer << endl;
	cout << TypeTraits<int*>::isPointer << endl;
	cout << TypeTraits<vector<int>::iterator>::isPointer << endl;
	
	int* cp = &ptr->c;
	int Widget::* cpp = &Widget::c;

	cout << TypeTraits<decltype(cp)>::isMemberPointer << endl;
	cout << TypeTraits<decltype(p)>::isMemberPointer << endl;
	cout << TypeTraits<decltype(cpp)>::isMemberPointer << endl;

	return 0;
}
