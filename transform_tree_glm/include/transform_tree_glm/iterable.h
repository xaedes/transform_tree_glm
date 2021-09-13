#pragma once

#include <type_traits>

namespace transform_tree_glm {

    template< class T > struct remove_reference { typedef T type; };
    template< class T > struct remove_reference<T&> { typedef T type; };
    template< class T > struct remove_reference<T&&> { typedef T type; };

    template <
        typename Iterator, 
        typename T, 
        bool IsConst = std::is_const<typename remove_reference<typename Iterator::reference>::type>::value
    >
    struct TypeCastIterator : public Iterator
    {
        using iterator_category = typename Iterator::iterator_category; // that is input_iterator_tag + out_iterator_tag
        using difference_type = typename Iterator::difference_type;
        using value_type = T;
        using pointer = typename std::conditional_t< IsConst, const value_type*, value_type* >;
        using reference = typename std::conditional_t< IsConst, const value_type&, value_type& >;
        // using reference = typename std::conditional_t< IsConst, value_type const &, value_type & >;
        // using pointer = typename std::conditional_t< IsConst, value_type const *, value_type * >;

        using Iterator::Iterator;
        //template <typename... Args> 
        //TypeCastIterator(Args&& ... args) noexcept
        //    : Iterator(std::forward< Args >(args)...)
        //{}

        // Iterator must provide conversion operator to convert to Iterator::pointer
        // this way we can also support nullptr
        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        operator pointer() { return static_cast<pointer>(static_cast<Iterator::pointer>(*this)); }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        operator pointer() const { return static_cast<pointer>(static_cast<Iterator::pointer>(*this)); }

        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        reference operator*()
        {
            return *static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        reference operator*() const
        {
            return *static_cast<pointer>(*this);
        }

        value_type operator*() const
        {
            return *static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        pointer operator->()
        {
            return static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        pointer operator->() const
        {
            return static_cast<pointer>(*this);
        }
    };

    template <
        typename Iterator, 
        typename T, 
        bool IsConst = std::is_const<typename remove_reference<typename Iterator::reference>::type>::value
    >
    struct DataMemberIterator : public Iterator
    {
        using iterator_category = typename Iterator::iterator_category; // that is input_iterator_tag + out_iterator_tag
        using difference_type = typename Iterator::difference_type;
        using value_type = T;
        using pointer = typename std::conditional_t< IsConst, const value_type*, value_type* >;
        using reference = typename std::conditional_t< IsConst, const value_type&, value_type& >;
        // using reference = typename std::conditional_t< IsConst, value_type const &, value_type & >;
        // using pointer = typename std::conditional_t< IsConst, value_type const *, value_type * >;

        using Iterator::Iterator;
        //template <typename... Args> 
        //TypeCastIterator(Args&& ... args) noexcept
        //    : Iterator(std::forward< Args >(args)...)
        //{}

        // Iterator must provide conversion operator to convert to Iterator::pointer
        // this way we can also support nullptr.
        // access data member over downcasted Iterator operator->
        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        operator pointer() 
        { 
            Iterator& it = *static_cast<Iterator*>(this);
            // there is no reason downcasting shouldn't work, no check for nullptr
            return static_cast<pointer>(it->data);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        operator pointer() const 
        { 
            const Iterator& it = *static_cast<const Iterator*>(this);
            // there is no reason downcasting shouldn't work, no check for nullptr
            return static_cast<pointer>(it->data);
        }

        // following functions use the above defined conversion operators
        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        reference operator*()
        {
            return *static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        reference operator*() const
        {
            return *static_cast<pointer>(*this);
        }

        value_type operator*() const
        {
            return *static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
        pointer operator->()
        {
            return static_cast<pointer>(*this);
        }

        template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
        pointer operator->() const
        {
            return static_cast<pointer>(*this);
        }
    };


    // https://stackoverflow.com/a/23400695/798588
    template<class T, class U = T>
    struct Iterable
    {
        T _begin;
        U _end;

        Iterable(T begin, U end)
            : _begin(begin), _end(end)
        {}

        T begin()
        {
            return _begin;
        }

        U end()
        {
            return _end;
        }
    };
    template<class T, class U>
    Iterable<T, U> make_iterable(T t, U u)
    {
        return Iterable<T, U>(t, u);
    }

} // namespace transform_tree_glm
