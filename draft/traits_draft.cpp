#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <memory>

#include <iostream>

using namespace std;


#define DEFINE_HAS_MEMBER_FUNC(func_name) \
	template <typename T, typename = void> \
	struct has_member_##func_name : false_type {}; \
	template <typename T> \
	struct has_member_##func_name<T, void_t<decltype(declval<T>().func_name())> > : true_type {}; \
	template <typename T> \
	inline constexpr bool has_member_##func_name##_v = has_member_##func_name<T>::value;

template <typename T, typename = void>
struct has_opplus : false_type {};

template <typename T>
struct has_opplus<T, void_t<decltype(declval<T>() + declval<T>())> > : true_type {};

template <typename T>
inline constexpr bool has_opplus_v = has_opplus<T>::value;

template <typename T, typename = void>
struct is_smart_ptr : false_type {};

template <typename T>
struct is_smart_ptr<T, void_t<typename T::element_type, decltype(declval<T>().get()), decltype(declval<T>().operator->())> >: true_type {};

template <typename T>
inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

DEFINE_HAS_MEMBER_FUNC(begin)
DEFINE_HAS_MEMBER_FUNC(end)

template <typename T, typename = void>
struct is_range : false_type {};

template <typename T>
struct is_range<T, enable_if_t<has_member_begin_v<T> && has_member_end_v<T> > > : true_type {};

template <typename T>
inline constexpr bool is_range_v = is_range<T>::value;

template <typename T, typename = enable_if_t<is_integral_v<T> || has_opplus_v<T> > >
T sum(const vector<T>& v)
{
	T t{};

	for (auto& t_ : v)
		t += t_;

	return t;
}

int get_rand_int(int L, int U)
{
	static random_device rd;
	static mt19937 gen(rd());

	uniform_int_distribution dist(L, U);

	return dist(gen);
}

struct Mon
{
	unsigned d_;

	Mon() = default;	
	Mon(unsigned d) : d_(d) {}

	Mon& operator+=(const Mon& m2)
	{
		d_ += m2.d_;

		return *this;
	}
};

Mon operator+(Mon m1, const Mon& m2)
{
	return m1+=m2;
}

void test1()
{
	static constexpr int N = 1<<4;
	vector<int> vi(N);
	
	generate(vi.begin(), vi.end(), bind(get_rand_int, 0, 128));

	for_each(vi.cbegin(), vi.cend(), [](int i) {cout << i << ' ';});

	cout << endl;
	cout << sum(vi) << endl;
}


void test2()
{
	static constexpr int N = 1<<4;
	vector<Mon> vi(N);
	
	generate(vi.begin(), vi.end(), bind(get_rand_int, 0, 128));

	for_each(vi.cbegin(), vi.cend(), [](const Mon& m) {cout << m.d_ << ' ';});

	cout << endl;
	cout << sum(vi).d_ << endl;

}

void test3()
{
	cout << (1 == is_smart_ptr_v<std::unique_ptr<string>>) << endl;
	cout << (1 == is_smart_ptr_v<std::shared_ptr<int>>) << endl;
	cout << (0 == is_smart_ptr_v<Mon>) << endl;
	
	cout << (0 == is_range_v<Mon>) << endl;
	cout << (0 == is_range_v<std::shared_ptr<int>>) << endl;
	cout << (1 == is_range_v<vector<Mon>>) << endl;
}

int main()
{
	test1();
	test2();
	test3();

	return 0;
}
