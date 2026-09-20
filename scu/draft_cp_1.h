#include <cstdint>
#include <vector>

using namespace std;

template <bool>
struct CompTimeChecker;

template <>
struct CompTimeChecker<true> {};

template <bool>
struct CompTimeCheckerMsg;

template <bool>
struct CompTimeCheckerMsg
{
        CompTimeCheckerMsg(...);
};

template <>
struct CompTimeCheckerMsg<false> {};

#define STATIC_CHECK(expr) CompTimeChecker<(expr)>();

#define STATIC_CHECK_MSG(expr, msg) \
{ \
        class ERROR_##msg {}; \
        (void) sizeof(CompTimeCheckerMsg<(expr)>((ERROR_##msg{}))); \
}


template <typename To, typename From>
To safe_reint_cast(From f)
{
//      STATIC_CHECK(sizeof(From) <= sizeof(To));
        STATIC_CHECK_MSG(sizeof(From) <= sizeof(To), Wrong_Cast);

        return reinterpret_cast<To>(f);
}

class Interface
{
public:
        virtual void fun() = 0;
};

template <typename Obj, typename Arg>
Interface* makeAdapter(const Obj& obj, const Arg& arg)
{
        class Adapt : public Interface
        {
        public:
                Adapt(const Obj& obj_, const Arg& arg_) : obj(obj_), arg(arg_) {}
                void fun() override
                {
                        obj.call(arg);
                }
        private:
                Obj obj;
                Arg arg;
        };

        return new Adapt(obj, arg);
}

template <int V>
struct ValueToType
{
        enum value {v = V};
};

template <typename T>
void foo(const T* t, ValueToType<true> v)
{
        T* t2 = t->clone();
}

template <typename T>
void foo(const T* t, ValueToType<false> v)
{
        T* t2 = new T(*t);

        //...
        delete t2;
}


template <bool isPolym, typename T>
void foo(const T* t)
{
        foo(t, ValueToType<isPolym>());
        /*
        if (isPolym)
        {
                T* t2 = t->clone();
                //...
        }
        else
        {
                T* t2 = new T(*t);
                //...
                delete t2;
        }
        */
}

struct A
{
        virtual A* clone() = 0;
};

struct B : public A
{
        A* clone() override
        {
                return new B(*this);
        }
};

struct C {};


struct Widget
{
        Widget(const char* name, int c_) : n(name), c(c_) {}

        const char* n;
        int c;
};

template <typename T>
struct Type2Type
{
        typedef T OrigType;
};

template <typename T, typename U>
T* create(const U& arg, Type2Type<T>)
{
        return new T(arg);
}

template <typename T, typename U>
T* create(const U& arg, Type2Type<Widget>)
{
        return new Widget(arg, -1);
}

// if T is polymorphic, store T*, otherwise T
template <bool isPolym_, typename T_, typename U_>
struct Select
{
        typedef U_ Result;
};

template <typename T_, typename U_>
struct Select<true, T_, U_>
{
        typedef T_ Result;
};

template <typename T, bool isPolym>
class NiftyContainer
{


        typedef typename Select<isPolym, T*, T>::Result ValueType;
        vector<ValueType> v;

public:
        void push(const ValueType& v_)
        {
                v.push_back(v_);
        }
};

struct U {};

struct V : U {};

typedef char Small;
struct Big {char c[2];};

Small test(V v);
Big test(...);

V makeV();

template <typename T, typename U>
class Conversion
{
        typedef char Small;
        struct Big
        {
                char c[2];
        };

        static Small test(U);
        static Big   test(...);

        static T makeT();
public:
        enum { exists = sizeof(test(makeT())) == sizeof(Small) };

        enum { is_same_t = false };
};

template <typename T>
class Conversion<T, T>
{
public:
        enum { exists = 1 };
        enum { is_same_t = 1 };
};

#define SUPER_SUBCLASS(T, U) \
        (Conversion<const U*, const T*>::exists && \
        !Conversion<const T*, const void*>::is_same_t)

class NullType {};

template <typename T>
class TypeTraits
{
        template <typename U>
        struct PointerTraits
        {
                enum { Result = false };
                typedef NullType PointeeType;
        };

        template <typename U>
        struct PointerTraits<U*>
        {
                enum { Result = true };
                typedef U PointeeType;
        };

        template <typename U>
        struct ReferenceTraits
        {
                enum { Result = false };
                typedef NullType ReferencedType;
        };

        template <typename U>
        struct ReferenceTraits<U&>
        {
                enum { Result = true };
                typedef U ReferencedType;
        };

        template <typename U>
        struct PToMTraits
        {
                enum { Result = false };
        };

        template <typename U, typename V>
        struct PToMTraits<U V::*>
        {
                enum { Result = true };
        };

        template <typename U>
        struct UnConst
        {
                typedef U Result;
        };

        template <typename U>
        struct UnConst<const U>
        {
                typedef U Result;
        };
public:
        enum { isPointer = PointerTraits<T>::Result };
        typedef typename PointerTraits<T>::PointeeType PointeeType;

        enum { isReference = ReferenceTraits<T>::Result };
        typedef typename ReferenceTraits<T>::ReferencedType ReferencedType;

        enum { isMemberPointer = PToMTraits<T>::Result };

        /*
        typedef TYPELIST_4 (
                        unsigned char, unsigned short int,
                        unsigned int, unsigned long int)
                UnsignedInts;
        typedef TYPELIST_4(
                        signed char, short int, int, long int)
                SignedInts;
        typedef TYPELIST_3(
                        bool, char, wchar_t)
                OtherInts;
        typedef TYPELIST_3(
                        float, double, long double)
                Floats;

        enum { isStdUnsignedInt = TL::IndexOf<T, UnsignedInts>::value >= 0 };
        enum { isSignedInt = TL::IndexOf<T, SignedInts>::value >= 0 };
        enum { isStdIntegral = isStdUnsignedInt || isStdSignedInt || TL::IndexOf<T, OtherInts>::value >= 0 };
        enum { isStdFloat = TL::IndexOf<T, Floats>::value >= 0 };
        enum { isStdArith = isStdIntegral || isStdFloat };
        enum { isStdFundamental = isStdArith || isStdFloat || Conversion<T, void>::sameType };

        typedef Select<isStdArith || isPointer || isMemberPointer, T, ReferencedType&>::Result ParameterType;
        */
        typedef typename UnConst<T>::Result NonConstType;
};
