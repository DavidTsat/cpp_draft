#include "vectorclass.h"

#include <stdio.h>
#include <functional>
#include <chrono>
#include <cassert>
#include <iostream>

using namespace std;

string ress;

//g++ -O3 -mavx512bw -ftree-vectorize pack.cpp -o pack

bool isPackableX(const string& s)
{
   static string packableDigits = "0123456789ABCDEFabcdef";
   return s.find_first_not_of(packableDigits) == string::npos;
}

std::function<bool(const string&)> isPackableX()
{
   return [](const string& s) { return isPackableX(s); };
}

bool isPackable(const string& s)
{
   return s.length() % 2 == 0 && isPackableX(s);
}

bool isPackable_AVX512(const string& s)
{
      __m512i reg_0 = _mm512_set1_epi8('0');
      __m512i reg_9 = _mm512_set1_epi8(9);
      __m512i reg_32 = _mm512_set1_epi8(0x20);
      __m512i reg_a = _mm512_set1_epi8('a');
      __m512i reg_5 = _mm512_set1_epi8(5);

      auto validateReg = [&](__m512i reg)
      {
         // https://softwareengineering.stackexchange.com/questions/268087/bitwise-operation-on-uppercase-ascii-character-turns-to-lowercase-why: A = 100 0001, OR with 010 0000 gives a = 110 0001. Same is true for all other letters.
         // ((s[i] - '0') <= 9) | ((s[i] || 0x20) - 'a' <=5)

         __m512i sub_0 = _mm512_sub_epi8(reg, reg_0);
         __mmask64 is_digit = _mm512_cmp_epu8_mask(sub_0, reg_9, _MM_CMPINT_LE); // k0 - k7 registers

         __m512i lower_reg = _mm512_or_si512(reg, reg_32);
         __m512i sub_a = _mm512_sub_epi8(lower_reg, reg_a);
         __mmask64 is_xdigit = _mm512_cmp_epu8_mask(sub_a, reg_5, _MM_CMPINT_LE);

         return is_digit | is_xdigit;
      };

      size_t len = s.size();
      const char* dataPtr = s.data();
      for (; len >= 64;) // can be optimized as well
      {
         __m512i data = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(dataPtr));
         __mmask64 is_validX = validateReg(data);

         if (is_validX != ~0ULL)
            return false;

         len -= 64;
         dataPtr += 64;
      }

      // process the remainings (< 64)
      if (len > 0)
      {
         __mmask64 len_bits = (1ULL << len) - 1ULL;
         __m512i reg = _mm512_maskz_loadu_epi8(len_bits, dataPtr);
         __mmask64 res = validateReg(reg);

         if ((res & len_bits) != len_bits)
            return false;
      }

      return true;
}

char packNibble(char c)
{
   if (isdigit(c))
      return (c - '0');
   else if (isxdigit(c))
      return ((c & 0x07) + 9);

   throw "!!!!";
}

string pack(const string& s)
{
   assert(isPackable(s));

   string help(s.length() / 2, ' ');
   for (uint i = 0; i < help.length(); i++)
      help[i] = (packNibble(s[i * 2]) << 4) | packNibble(s[i * 2 + 1]);

   return help;
}

string pack_VCL(const string& s)
{
   assert(isPackable_AVX512(s));

   string help(s.length() / 2, ' ');

   alignas(64) static const int8_t maskBytes[64] = 
   {
    16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1,
    16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1,
    16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1,
    16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1
   };

   Vec64c mask;
   mask.load(maskBytes);

   size_t readIdx = 0;
   size_t writeIdx = 0;

   for (; writeIdx + 32 <= help.length(); readIdx += 64, writeIdx += 32)
   {
   	Vec64uc nibs;
	nibs.load(s.data() + readIdx);
	// numeric values
	nibs = select(nibs <= '9', nibs - '0', (nibs & 0x07) + 9);
	//c = c * mask;
	Vec32s ps = _mm512_maddubs_epi16(nibs, mask);
//	Vec32s ps = madd_us(nibs, mask);
	Vec32c cmprsBytes = _mm512_cvtepi16_epi8(ps);
	cmprsBytes.store(help.data() + writeIdx);
   }

   
   for (; writeIdx < help.length(); readIdx += 2, ++writeIdx)
      help[writeIdx] = (packNibble(s[readIdx]) << 4) | packNibble(s[readIdx + 1]);

   return help;
}

string pack_AVX512(const string& s)
{
      assert(isPackable_AVX512(s)); // can be combined with the main loop.

      string help(s.length() / 2, ' ');

      // even-indexed bytes shift 4, odd-indexed stay at the same position
      alignas(64) static const int8_t mul_mask[64] = {
         16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1,
         16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1,
         16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1, 16, 1};

      __m512i mask = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(mul_mask));

      size_t len = s.size();
      size_t rIdx = 0;
      size_t wIdx = 0;
      char* dstPtr = help.data();
      const char* srcPtr = s.data();

      const __m512i reg_9 = _mm512_set1_epi8(9);
      const __m512i reg_a9 = _mm512_set1_epi8('9');
      const __m512i reg_a0 = _mm512_set1_epi8('0');
      const __m512i reg_7 = _mm512_set1_epi8(0x07);

      for (; len >= 64;)
      {
         // s = (s <= '9' ? (s - '0') : (s & 0x07) + 9))
         // horizontal_add(vertical_mul(s, mul_mask))

         __m512i data = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(srcPtr + rIdx));
         __mmask64 dig_mask = _mm512_cmp_epu8_mask(data, reg_a9, _MM_CMPINT_LE);
         __m512i dig = _mm512_sub_epi8(data, reg_a0);
         __m512i alpf = _mm512_add_epi8(_mm512_and_si512(data, reg_7), reg_9);

         data = _mm512_mask_blend_epi8(dig_mask, alpf, dig);
         __m512i packed = _mm512_maddubs_epi16(data, mask);
         __m256i out = _mm512_cvtepi16_epi8(packed); // truncate unused tail

         _mm256_storeu_si256(reinterpret_cast<__m256i*>(dstPtr + wIdx), out);

         len -= 64;
         rIdx += 64;
         wIdx += 32;
      }

      // the remainder must be even
      assert((len & 1) == 0);
      // process the remainings in scalar mode
      for (; rIdx < s.size(); rIdx += 2, ++wIdx)
      {
         char c1 = s[rIdx] <= '9' ? s[rIdx] - '0' : (s[rIdx] & 0x07) + 9;
         char c2 = s[rIdx + 1] <= '9' ? s[rIdx + 1] - '0' : (s[rIdx + 1] & 0x07) + 9;
         help[wIdx] = ((c1 << 4) | c2);
      }

      return help;
}


string genTestInput(size_t repetitions)
{
    const string basePattern =
        "00000015416371756972657253757263686172676552756C6500010000000000000000"
        "544E000000000100000000000000010000001541637175697265725375726368617267"
        "6552756C6500010000000000000000544E00000000010000000000000001";

    string largeInput;
    largeInput.reserve(basePattern.length() * repetitions);

    for (size_t i = 0; i < repetitions; ++i)
        largeInput += basePattern;

    return largeInput;
}

void test(string (*f)(const string&), size_t sz)
{
	string str = genTestInput(sz);
	auto st = chrono::steady_clock::now();

//	for (int i = 0; i < 100; ++i)
//	{
//		string serialized = pack("00000015416371756972657253757263686172676552756C6500010000000000000000544E0000000001000000000000000100000015416371756972657253757263686172676552756C6500010000000000000000544E00000000010000000000000001");
//	}
//	string restored = unpack(serialized);
	
	string ser = f(str);

	if (!ress.empty())
		assert(ser == ress);
	else
		ress = ser;
	// not to optimize:
	asm volatile("" : : "g"(ser.data()) : "memory");

	auto en = chrono::steady_clock::now();

	auto mcs = chrono::duration_cast<chrono::microseconds>(en - st);

//	cout << serialized << endl;
//	cout << restored << endl;	
	cout << mcs.count() << " mcs" << endl;
}



int main()
{
	test(pack, 100);
	test(pack_VCL, 100);
	test(pack_AVX512, 100);
	ress = "";
	cout << "\n----------------------------------------\n";

	test(pack, 1024);
	test(pack_VCL, 1024);
	test(pack_AVX512, 1024);
	ress = "";

	cout << "\n----------------------------------------\n";
	test(pack, 100000);
	test(pack_VCL, 100000);
	test(pack_AVX512, 100000);


	return 0;
}
