#include <functional>
#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>
#include <cassert>

#include <iostream>

#include "uptr.h"

using namespace std;

template <typename T>
class ShPtr
{
        typedef T* PtrType;
        typedef T& RefType;

        template <typename U>
        friend class ShPtr;

        struct Test
        {
                static void operator delete(void*) = delete;
        };
public:
        explicit ShPtr(PtrType ptr = {}) : ptr_(ptr), ptrImpl_{}
        {
                if (ptr_)
                        ptrImpl_ = new Ptr<PtrType>(ptr_);
        }

        template <typename D>
        explicit ShPtr(PtrType ptr, D&& d = {}) : ptr_(ptr), ptrImpl_(new Ptr<PtrType, D>(ptr_, forward<D>(d))) {}

        ShPtr(const ShPtr& rShPtr) : ptr_(rShPtr.ptr_), ptrImpl_(rShPtr.ptrImpl_)
        {
                if (ptrImpl_)
                        ptrImpl_->incCntr();
        }

        template <typename U, typename = enable_if_t<is_convertible_v<U*, PtrType>>>
        ShPtr(const ShPtr<U>& rShPtr) : ptr_(rShPtr.ptr_), ptrImpl_(rShPtr.ptrImpl_)
        {
                if (ptrImpl_)
                        ptrImpl_->incCntr();
        }

        ShPtr(ShPtr&& rShPtr) noexcept : ShPtr({})
        {
                swap(rShPtr);
        }

        template <typename U, typename = enable_if_t<is_convertible_v<U*, PtrType>>>
        ShPtr(ShPtr<U>&& rShPtr) noexcept : ptr_(rShPtr.ptr_), ptrImpl_(rShPtr.ptrImpl_)
        {
                rShPtr.ptr_ = nullptr;
                rShPtr.ptrImpl_ = nullptr;
        }

	template <typename U, typename D, 
		 typename = enable_if_t<is_convertible_v<U*, PtrType>>
			 >
	ShPtr(Uptr<U, D>&& up) noexcept : ShPtr(up.get(), move(up.get_deleter()))
	{
		up.release();
	}

        ShPtr& operator=(const ShPtr& rShPtr)
        {
                ShPtr sp(rShPtr);
                swap(sp);

                return *this;
        }

        template <typename U, typename = enable_if_t<is_convertible_v<U*, PtrType>>>
        ShPtr& operator=(const ShPtr<U>& rShPtr)
        {
                ShPtr<U> sp(rShPtr);
                swap(sp);

                return *this;
        }

        ShPtr& operator=(ShPtr&& rShPtr) noexcept
        {
                ShPtr sp(move(rShPtr));
                swap(sp);

                return *this;
        }

        template <typename U, typename = enable_if_t<is_convertible_v<U*, PtrType>>>
        ShPtr& operator=(ShPtr<U>&& rShPtr) noexcept
        {
                ShPtr sp(move(rShPtr));
                swap(sp);

                return *this;
        }

        void swap(ShPtr& sp2) noexcept
        {
                std::swap(ptr_, sp2.ptr_);
                std::swap(ptrImpl_, sp2.ptrImpl_);
        }

        ~ShPtr() noexcept
        {
                if (!ptrImpl_ )
                        return;

                if (!ptrImpl_->decCntr())
                        destroy();
        }

        PtrType operator->() noexcept
        {
                return ptr_;
        }

        PtrType operator->() const noexcept
        {
                return ptr_;
        }

        RefType operator*() noexcept
        {
                return *ptr_;
        }

        RefType operator*() const noexcept
        {
                return *ptr_;
        }

        unsigned use_count() const noexcept
        {
                //todo: maybe with bitwise ?

                return ptrImpl_ ? ptrImpl_->hCtr_ : 0;
        }

        operator Test*() const
        {
                if (!ptrImpl_)
                        return {};

                static Test t;
                return &t;
        }
        // todo: get_deleter
        //

        template <typename O, typename... Args>
        friend inline ShPtr<O> makeShPtr(Args&&... args);
private:
	template <typename TT, typename D> 
	class Uptr;

        struct PtrBase
        {
                unsigned hCtr_;

                PtrBase() : hCtr_(0) {}

                unsigned incCntr() noexcept
                {
                        return ++hCtr_;
                }

                unsigned decCntr() noexcept
                {
                        return --hCtr_;
                }

                void clone() const
                {
                        doClone();
                }

                void dispose() noexcept
                {
                        doDispose();
                }

                virtual ~PtrBase() {}
        private:
                virtual PtrBase* doClone() = 0;
                virtual void doDispose() = 0;
        };

        template <typename O>
        struct CtrlBlock : PtrBase
        {
                O obj_;

                template <typename... Args>
                CtrlBlock(Args&&... args) : PtrBase(), obj_(forward<Args>(args)...)
                {
                        PtrBase::incCntr();
                }

                PtrBase* doClone() override
                {
                        return 0;
                }

                void doDispose() override
                {
                        // noop ?
                        obj_.~O();
                }
        };

        template <typename P, typename D=default_delete<T>>
        struct Ptr : PtrBase
        {
                P p_;
                D deleter_;

                Ptr(P p, D&& d = {}) : PtrBase(), p_(p), deleter_(d)
                {
                        PtrBase::incCntr();
                }

                PtrBase* doClone() override
                {
                        return 0;
                }

                void doDispose() override
                {
                        deleter_(p_);
                }
        };

        void destroy() noexcept
        {
                ptrImpl_->dispose();
                delete ptrImpl_;
        }

        template <typename U, typename... Args>
        struct is_shptr_convertible : false_type {};

        template <typename U, typename V>
        struct is_shptr_convertible<U, V> : is_same<decay_t<V>, ShPtr<U>> {};

        template <typename TT>
        struct is_shptr : false_type {};

        template <typename U>
        struct is_shptr<ShPtr<U>> : true_type {};

        template <typename TT>
        struct is_uptr : false_type {};

        template <typename U, typename D>
        struct is_uptr<Uptr<U, D>> : true_type {};

        template <typename... Args>
        struct is_smart_ptr_arg : false_type {};

        template <typename V>
        struct is_smart_ptr_arg<V> 
        : integral_constant<bool, is_shptr<decay_t<V>>::value || is_uptr<decay_t<V>>::value> {};

        template <typename... Args,
              typename = enable_if_t<!is_smart_ptr_arg<Args...>::value>>
        ShPtr(Args&&... args)
        {
                ptrImpl_ = new CtrlBlock<T>(forward<Args>(args)...);
                ptr_ = &(static_cast<CtrlBlock<T>*>(ptrImpl_))->obj_;
        }

        PtrType ptr_;
        PtrBase* ptrImpl_;
};


template <typename O, typename... Args>
ShPtr<O> makeShPtr(Args&&... args)
{
        return ShPtr<O>(forward<Args>(args)...);
}

