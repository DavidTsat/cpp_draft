#include "draft_cp_1.h"

#include <iostream>
#include <typeinfo>

using namespace std;


template <typename T, typename U>
class TypeList
{
        typedef T Head;
        typedef U Tail;

};

namespace TL
{
//      typedef TypeList<signed char, TypeList<short, TypeList<int, TypeList<long, NullType> > > > SignedIntegrals;

#define TYPELIST_1(T1) TypeList<T1, NullType>
#define TYPELIST_2(T1, T2) TypeList<T1, TYPELIST_1(T2) >
#define TYPELIST_3(T1, T2, T3) TypeList<T1, TYPELIST_2(T2, T3) >
#define TYPELIST_4(T1, T2, T3, T4) TypeList<T1, TYPELIST_3(T2, T3, T4) >

        typedef TYPELIST_4(signed char, short, int, long) SignedIntegrals;

        template <typename TList>
        struct Length;

        template <>
        struct Length<NullType>
        {
                enum { value = 0 };
        };

        template <typename T, typename U>
        struct Length<TypeList<T, U> >
        {
                enum { value = 1 + Length<U>::value };
        };

        template <typename TList, unsigned i>
        struct TypeAt;

        template <typename Head, typename Tail>
        struct TypeAt<TypeList<Head, Tail>, 0>
        {
                typedef Head Result;
        };

        template <typename Head, typename Tail, unsigned i>
        struct TypeAt<TypeList<Head, Tail>, i>
        {
                typedef typename TypeAt<Tail, i-1>::Result Result;
        };

        template <typename TList, unsigned i, typename FType=NullType>
        struct TypeAtNonStrict
        {
                typedef FType Result;
        };

        template <typename Head, typename Tail, typename FType>
        struct TypeAtNonStrict<TypeList<Head, Tail>, 0, FType>
        {
                typedef Head Result;
        };

        template <typename Head, typename Tail, unsigned i, typename FType>
        struct TypeAtNonStrict<TypeList<Head, Tail>, i, FType>
        {
                typedef typename TypeAtNonStrict<Tail, i-1, FType>::Result Result;
        };

        template <typename TList, typename T>
        struct IndexOf;

        template <typename T>
        struct IndexOf<NullType, T>
        {
                enum { value = -1 };
        };

        template <typename T, typename Tail>
        struct IndexOf<TypeList<T, Tail>, T>
        {
                enum { value = 0 };
        };

        template <typename Head, typename Tail, typename T>
        struct IndexOf<TypeList<Head, Tail>, T>
        {
        private:
                enum { temp = IndexOf<Tail, T>::value };
        public:
                enum { value = temp == - 1 ? -1 : temp + 1 };
        };

        /*
         *  If TList is NullType and T is NullType, then Result is NullType.
   Else
     If TList is NullType and T is a single (nontypelist) type, then Result is a typelist hav-
       ing T as its only element.
     Else
       If
TList is NullType and T is a typelist, Result is T itself.
       Else if TList is non-null, then Result is a typelist having TList::Head as its
         head and the result of appending T to TList::Tail as its tail.
         */

        template <typename TList, typename T>
        struct Append;

        template <>
        struct Append<NullType, NullType>
        {
                typedef NullType Result;
        };

        template <typename T>
        struct Append<NullType, T>
        {
                typedef TYPELIST_1(T) Result;
        };

        template <typename U, typename V>
        struct Append<NullType, TypeList<U, V> >
        {
                typedef TypeList<U, V> Result;
        };

        template <typename Head, typename Tail, typename T>
        struct Append<TypeList<Head, Tail>, T>
        {
                typedef TypeList<Head, typename Append<Tail, T>::Result> Result;
        };

        typedef Append<SignedIntegrals, TYPELIST_3(float, double, long double)>::Result SignedTypes;

        template <typename TList, typename T>
        struct Erase;

        template <typename T>
        struct Erase<NullType, T>
        {
                typedef NullType Result;
        };

        template <typename Tail, typename T>
        struct Erase<TypeList<T, Tail>, T>
        {
                typedef Tail Result;
        };

        template <typename Head, typename Tail, typename T>
        struct Erase<TypeList<Head, Tail>, T>
        {
                typedef TypeList<Head, typename Erase<Tail, T>::Result> Result;
        };

        template <typename TList, typename T>
        struct EraseAll;

        template <typename T>
        struct EraseAll<NullType, T>
        {
                typedef NullType Result;
        };

        template <typename Tail, typename T>
        struct EraseAll<TypeList<T, Tail>, T>
        {
                typedef typename EraseAll<Tail, T>::Result Result;
        };

        template <typename Head, typename Tail, typename T>
        struct EraseAll<TypeList<Head, Tail>, T>
        {
                typedef TypeList<Head, typename Erase<Tail, T>::Result> Result;
        };

        template <typename TList, typename T, typename U>
        struct Replace;

        template <typename T, typename U>
        struct Replace<NullType, T, U>
        {
                typedef NullType Result;
        };

        template <typename Tail, typename T, typename U>
        struct Replace<TypeList<T, Tail>, T, U>
        {
                typedef TypeList<U, Tail> Result;
        };

        template <typename Head, typename Tail, typename T, typename U>
        struct Replace<TypeList<Head, Tail>, T, U>
        {
                typedef TypeList<Head, typename Replace<Tail, T, U>::Result> Result;
        };


        template <typename TList, typename T>
        struct MostDerived;

        template <typename T>
        struct MostDerived<NullType, T>
        {
                typedef NullType Result;
        };

        template <typename Head, typename Tail, typename T>
        struct MostDerived<TypeList<Head, Tail>, T>
        {
        private:
                typedef typename MostDerived<Tail, T>::Result Candidate;
        public:
                typedef typename Select<SUPER_SUBCLASS(Candidate, Head), Head, Candidate>::Result Result;
        };

        template <typename TList>
        struct DerivedToFront;

        template <>
        struct DerivedToFront<NullType>
        {
                typedef NullType Result;
        };

        template <typename Head, typename Tail>
        struct DerivedToFront<TypeList<Head, Tail> >
        {
        private:
                typedef typename MostDerived<Tail, Head>::Result TheMostDerived;
                typedef typename Replace<Tail, TheMostDerived, Head>::Result L;
        public:
                typedef TypeList<TheMostDerived, L> Result;
        };

        template <typename TList, template <typename> typename Unit>
        class GenScatterHierarchy;

        template <typename AtomicType, template <typename> typename Unit>
        class GenScatterHierarchy : public Unit<AtomicType>
        {
                typedef Unit<AtomicType> LeftBase;
        };

        template <template <typename> typename Unit>
        class GenScatterHierarchy<NullType, Unit>
        {
        };

        template <typename T1, typename T2, template <typename> typename Unit>
        class GenScatterHierarchy<TypeList<T1, T2>, Unit>
                : public GenScatterHierarchy<T1, Unit>
                , public GenScatterHierarchy<T2, Unit>
        {
        public:
                typedef TypeList<T1, T2> TList;
                typedef GenScatterHierarchy<T1, Unit> LeftBase;
                typedef GenScatterHierarchy<T2, Unit> RightBase;
        };

};
