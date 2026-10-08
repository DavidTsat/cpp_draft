#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cstddef>

using namespace std;

template <typename T>
class Serializer
{
public:
	static vector<byte> serialize(const T& obj)
	{
		vector<byte> v;
		serialize(obj, v);


            	return v;
    	}

    	static T deserialize(const vector<byte>& v)
   	{
        	T t;
            	deserialize(v, t);

            	return t;
    	}
};

template <>
class Serializer<int>
{
public:
	static vector<byte> serialize(const int& x)
	{
		vector<byte> v(sizeof(x));


            	memcpy(v.data(), &x, sizeof(x));

            	return v;
    	}

    	static int deserialize(const vector<byte>& v)
    	{
            	int x;

            	memcpy(&x, v.data(), sizeof(x));

            	return x;
    	}
};

template <>
class Serializer<string>
{
public:
	static vector<byte> serialize(const string& s)
	{
		size_t sz = s.size();

		vector<byte> v(sizeof(size_t) + sz);
		
		memcpy(v.data(), &sz, sizeof(size_t));
		memcpy(v.data() + sizeof(size_t), s.data(), sz);

		return v;
	}

	static string deserialize(const vector<byte>& v)
	{
		size_t sz;
		memcpy(&sz, v.data(), sizeof(size_t));
		string s;
		s.resize(sz);

		memcpy(s.data(), v.data() + sizeof(size_t), sz);

		return s;
	}
};

template <typename T>
class Serializer<vector<T>>
{
public:
	static vector<byte> serialize(const vector<T>& v)
	{
		if (v.empty())
			return {};

		vector<byte> vb(sizeof(size_t));

            	size_t sz = v.size();

            	memcpy(vb.data(), &sz, sizeof(size_t));
		
//		sz = sizeof(T);
//		memcpy(vb.data() + sizeof(size_t), &sz, vb.size());
		
		byte* p = vb.data() + sizeof(size_t);
		for (const T& t : v)
		{
			vector<byte> vbt = Serializer<T>::serialize(t);
			vb.resize(vb.size() + vbt.size());

			memcpy(p, vbt.data(), vbt.size());
		}

            	return vb;
    	}

    	static vector<T> deserialize(const vector<byte>& v)
    	{
		if (v.empty())
			return {};

		size_t sz;

            	memcpy(&sz, v.data(), sizeof(size_t));

            	vector<T> vt(sz);
		const byte* p = v.data() + sizeof(size_t);
		for (size_t i = 0; i < sz; ++i)
		{
			memcpy(&sz, p, sizeof(size_t));
			p += sizeof(size_t);
			vector<byte> vbi(sz);
			memcpy(vbi.data(), p, sz);
			p += sz;
			vt.push_back(Serializer<T>::deserialize(vbi));
		}

            	return vt;
    	}
};

void test1()
{
	int a = 77778;


	int b = Serializer<int>::deserialize(Serializer<int>::serialize(a));

    	cout << b << endl;

    	vector<string> vs({"abc", "defg", "hijklmnop", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"});

    	vector<string> vss = Serializer<vector<string>>::deserialize(Serializer<vector<string>>::serialize(vs));

    	for_each(vss.cbegin(), vss.cend(), [](const string& s){ puts(s.c_str());});
}

int main()
{
	test1();
	return 0;
}
