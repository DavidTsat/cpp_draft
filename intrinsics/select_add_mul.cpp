#include <chrono>
#include <iostream>
#include <functional>
#include <emmintrin.h>

//g++ -Ofast -fno-tree-vectorize -fno-tree-slp-vectorize -S -masm=intel select_add_mul.cpp -o select.s
//g++ -Ofast -fno-tree-vectorize -fno-tree-slp-vectorize select_add_mul.cpp -o select
//
using namespace std;

void SelectAddMul(short int aa[], short int bb[], short int cc[])
{
        for (int i = 0; i < 256; i++)
                aa[i] = (bb[i] > 0) ? (cc[i] + 2) : (bb[i] * cc[i]);
}

void SelectAddMulMask(short int aa[], short int bb[], short int cc[])
{
        for (int i = 0; i < 256; i++)
        {
                int mask = -(bb[i] > 0);

                aa[i] = (mask & (cc[i] + 2)) | (~mask & (bb[i] * cc[i]));
        }
}

static inline __m128i LoadVector(void const* p)
{
	return _mm_loadu_si128((__m128i const*)p);
}

static inline void StoreVector(void * d, const __m128i& x)
{
	_mm_storeu_si128((__m128i*)d, x);
}

void SelectAddMulMaskVec(short int aa[], short int bb[], short int cc[])
{
	// vec of (0,0,0,0,0,0,0,0)
	__m128i zero = _mm_setzero_si128();

	// vec of (2,2,2,2,2,2,2,2)
	__m128i two = _mm_set1_epi16(2);

	for (int i = 0; i < 256; i += 8)
	{
		// load 8 elems into vec b
		__m128i b = LoadVector(bb + i);
		
		// load 8 elems into vec c
		__m128i c = LoadVector(cc + i);

		// add 2 to each elem of vec c
		__m128i c2 = _mm_add_epi16(c, two);

		// mult b and c
		__m128i bc = _mm_mullo_epi16(b, c);
		
		// comp each elem of b with 0 and make a mask
		__m128i mask = _mm_cmpgt_epi16(b, zero);


		// AND c2 with the mask
		c2 = _mm_and_si128(c2, mask);

		// AND bc with the inverted mask
		bc = _mm_andnot_si128(mask, bc);

		// OR c2 and bc
		__m128i a = _mm_or_si128(c2, bc);

		// Store in 8 consec. elems in aa:
		StoreVector(aa + i, a);
	}
}

void test()
{
        short aa[256] = {0};
        short bb[256];
        short cc[256];

        for (int i = 0; i < 256; ++i)
        {
                bb[i] = static_cast<short>((i%2 == 0) ? (i - 128): i);
                cc[i] = static_cast<short>(i + 1);
        }

        auto measureF = [&](void(*f)(short int[], short int[], short int[]))
        {
                auto start = chrono::steady_clock::now();

                for (int i = 0; i < 1000; ++i)
                {
                        f(aa, bb, cc);
                }

                auto end = chrono::steady_clock::now();

                auto mcs = chrono::duration_cast<chrono::microseconds>(end - start);

                cout << "elapsed time: " << mcs.count() << " mcs\n";

        };

        measureF(SelectAddMul);
        measureF(SelectAddMulMask);
        measureF(SelectAddMulMaskVec);
}

int main()
{
        test();

        return 0;
}
