#include <functional>
#include <utility>

#include <iostream>

using namespace std;

namespace Priv
{
        template <typename T>
        struct DefaultDel
        {
                void operator()(T* p)
                {
                        delete p;
                }
        };
};

//template <typename T, template<typename> class D=Priv::DefaultDel>
template <typename T, class D=Priv::DefaultDel<T>>
class Uptr
{
        typedef T* PointerType;
        typedef T& ReferenceType;
        typedef D DeleterType;
public:
        Uptr() noexcept : ptr_{}, d_{} {}

        explicit Uptr(PointerType ptr) noexcept : ptr_(ptr), d_{} {}

        Uptr(PointerType ptr, DeleterType d) noexcept : ptr_(ptr), d_(move(d)) {}

        Uptr(Uptr&& rhs) noexcept : Uptr()
        {
                this->swap(rhs);
        }

        Uptr& operator=(Uptr&& rhs) noexcept
        {
                if (this == &rhs)
                        return *this;

                reset(rhs.release());
                d_ = move(rhs.d_);

                return *this;
        }

        ~Uptr() noexcept
        {
                destroy();
        }

        PointerType operator->() noexcept
        {
                return ptr_;
        }

        ReferenceType operator*() noexcept
        {
                return *ptr_;
        }

        PointerType operator->() const noexcept
        {
                return ptr_;
        }

        ReferenceType operator*() const noexcept
        {
                return *ptr_;
        }

        void reset(PointerType p = PointerType{}) noexcept
        {
                PointerType old = ptr_;
                ptr_ = p;

                if (old)
                        d_(old);
        }

        PointerType release() noexcept
        {
                return exchange(ptr_, nullptr);
        }

        PointerType get() noexcept
        {
                return ptr_;
        }

        void swap(Uptr& rhs) noexcept
        {
                std::swap(ptr_, rhs.ptr_);
                std::swap(d_, rhs.d_);
        }
	
	D get_deleter() const noexcept
	{
		return d_;
	}

        Uptr(const Uptr&) = delete;
        Uptr& operator=(const Uptr&) = delete;
private:
        PointerType ptr_;
        [[no_unique_address]] DeleterType d_;

        void destroy() noexcept
        {
                if (ptr_)
                        d_(ptr_);
        }
};

