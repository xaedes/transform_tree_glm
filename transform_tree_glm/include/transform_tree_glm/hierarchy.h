#pragma once

#include <type_traits>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t
#include <cassert>
#include <vector>
#include <functional>


#include "transform_tree_glm/iterable.h"

namespace transform_tree_glm {

    class Hierarchy
    {
    public:
        using value_type = Hierarchy*;
        using reference = Hierarchy*&;
        using const_reference = const Hierarchy*&;
        using difference_type = std::ptrdiff_t;
        using size_type = size_t;

        using pointer = Hierarchy*;
        using const_pointer = const Hierarchy*;

    protected:
        pointer m_parent = nullptr;
        // children
        pointer m_begin = nullptr;
        pointer m_last = nullptr;
        static constexpr pointer m_end = nullptr;
        size_type m_countChildren = 0;
        // in parent
        pointer m_prev = nullptr;
        pointer m_next = nullptr;


    public:
        Hierarchy() : data(nullptr) {}
        Hierarchy(void* data) : data(data) {}
        void* data = nullptr;

        ~Hierarchy()
        {
            erase_from_parent();
            clear();
        }

        #pragma region attributes
        inline bool empty() const { return m_countChildren == 0; }
        inline size_type size() const { return m_countChildren; }
        inline const_pointer parent() const { return m_parent; }
        inline const_pointer prev()   const { return m_prev; }
        inline const_pointer next()   const { return m_next; }
        inline const_pointer front()  const { return m_begin; }
        inline const_pointer back()   const { return m_last; }

        inline pointer parent() { return m_parent; }
        inline pointer prev()   { return m_prev; }
        inline pointer next()   { return m_next; }
        inline pointer front()  { return m_begin; }
        inline pointer back()   { return m_last; }
        #pragma endregion


        #pragma region iterator classes
        template <bool IsConst>
        class ChildrenIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // that is input_iterator_tag + out_iterator_tag
            using difference_type = std::ptrdiff_t;
            using value_type = Hierarchy;
            //using pointer = Hierarchy::pointer;
            //using reference = value_type&;

            // using pointer = value_type*;
            // using reference = value_type&;
            // https://stackoverflow.com/a/49425072/798588
            using pointer = typename std::conditional_t< IsConst, const value_type *, value_type * >;
            using reference = typename std::conditional_t< IsConst, const value_type &, value_type & >;
            // using pointer = typename std::conditional_t< IsConst, value_type const *, value_type * >;
            // using reference = typename std::conditional_t< IsConst, value_type const &, value_type & >;

            ChildrenIterator(const Hierarchy::pointer& ptr) noexcept
                : m_item(ptr) {}

            template<bool IsConst_ = IsConst, class = std::enable_if_t<IsConst_>>
            ChildrenIterator(const ChildrenIterator<false>& other)
                : m_item(other.m_item) {} 

            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            inline operator pointer() { return m_item; }

            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            inline operator pointer() const { return m_item; }

            // functionality for all iterator categories
            // copy-constructible, copy-assignable and destructible
            // Can be incremented
            ChildrenIterator(const ChildrenIterator& other) = default;
                //: m_item(other.m_item) {}
            //~ChildrenIterator() {}

            ChildrenIterator& operator=(const ChildrenIterator& other)
            {
                m_item = other.m_item;
                return *this;
            }

            ChildrenIterator& operator++() //prefix increment
            {
                if (m_item)
                {
                    m_item = m_item->m_next;
                }
                return *this;
            }

            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            reference operator*() 
            {
                return *m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            reference operator*() const
            {
                return *m_item;
            }

            friend void swap(ChildrenIterator& lhs, ChildrenIterator& rhs) //C++11 I think
            {
                using std::swap;
                swap(lhs.m_item, rhs.m_item);
            }

            // functionality for input_iterator_tag 
            // Supports equality/inequality comparisons
            // Can be dereferenced as an rvalue
            ChildrenIterator operator++(int) //postfix increment
            {
                ChildrenIterator beforeInc(*this);
                ++(*this);
                return beforeInc;
            }
            value_type operator*() const
            {
                return *m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            pointer operator->()
            {
                return m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            pointer operator->() const
            {
                return m_item;
            }
            friend bool operator==(const ChildrenIterator& lhs, const ChildrenIterator& rhs)
            {
                return lhs.m_item == rhs.m_item;
            }
            friend bool operator!=(const ChildrenIterator& lhs, const ChildrenIterator& rhs)
            {
                return lhs.m_item != rhs.m_item;
            }

            // functionality for out_iterator_tag 
            // Can be dereferenced as an lvalue
            // (only for mutable iterator types)
            //reference operator*(); // const
            // already defined

            // ChildrenIterator operator++(int) //postfix increment
            // already defined

            // functionality for forward_iterator_tag
            // default-constructible
            // Multi-pass: neither dereferencing nor incrementing affects dereferenceability
            // ChildrenIterator() : m_item(nullptr) {}
            ChildrenIterator() = default;
        protected:
            pointer m_item = nullptr;
        };

        template <bool IsConst>
        class RecurseIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // that is input_iterator_tag + out_iterator_tag
            using difference_type = std::ptrdiff_t;
            using value_type = Hierarchy;

            // using pointer = Hierarchy::pointer;
            // using reference = value_type&;
            // https://stackoverflow.com/a/49425072/798588
            using pointer = typename std::conditional_t< IsConst, const value_type*, value_type* >;
            using reference = typename std::conditional_t< IsConst, const value_type&, value_type& >;
            //using pointer = typename std::conditional_t< IsConst, value_type const *, value_type * >;
            //using reference = typename std::conditional_t< IsConst, value_type const &, value_type & >;

            RecurseIterator(pointer ptr) noexcept
                : m_item(ptr) 
                , m_recurseChildren(true) 
                , m_depth(0) 
            {}

            template<bool IsConst_ = IsConst, class = std::enable_if_t<IsConst_>>
            RecurseIterator(const RecurseIterator<false>& other) 
                : m_item(other.m_item) 
                , m_recurseChildren(other.m_recurseChildren) 
                , m_depth(other.m_depth)
            {} 

            inline int depth() const { return m_depth; }

            inline void skipChildren()
            {
                m_recurseChildren = false;
            }

            inline void includeChildren()
            {
                m_recurseChildren = true;
            }

            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            inline operator pointer() { return m_item; }
            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            inline operator pointer() const { return m_item; }

            // functionality for all iterator categories
            // copy-constructible, copy-assignable and destructible
            // Can be incremented
            RecurseIterator(const RecurseIterator& other) = default;
                //: m_item(other.m_item) 
                //, m_recurseChildren(other.m_recurseChildren) 
            //{}
            //~RecurseIterator() {}
            RecurseIterator& operator=(const RecurseIterator& other)
            {
                m_item = other.m_item;
                m_recurseChildren = other.m_recurseChildren;
                return *this;
            }
            RecurseIterator& operator++() //prefix increment
            {
                if (m_item)
                {
                    if (m_recurseChildren && (m_item->m_begin != m_item->m_end))
                    {
                        // advance to children 
                        m_item = m_item->m_begin;
                        ++m_depth;
                    }
                    // has no children
                    else if (m_item->m_next != nullptr)
                    {
                        // advance to next item 
                        m_item = m_item->m_next;
                    }
                    // has no next sibling
                    else 
                    {
                        // advance to parent, when parent has advanced to end, 
                        // proceed with next item of its parent, and so on
                        while (m_item->m_parent != nullptr)
                        {
                            if (m_item->m_parent->m_next != nullptr)
                            {
                                m_item = m_item->m_parent->m_next;
                                --m_depth;
                                return *this;
                            }
                            else 
                            {
                                m_item = m_item->m_parent;
                                --m_depth;
                            }
                        }
                        m_item = nullptr;
                    }

                }
                return *this;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            reference operator*() 
            {
                return *m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            reference operator*() const
            {
                return *m_item;
            }
            friend void swap(RecurseIterator& lhs, RecurseIterator& rhs) //C++11 I think
            {
                using std::swap;
                swap(lhs.m_item, rhs.m_item);
                swap(lhs.m_recurseChildren, rhs.m_recurseChildren);
                swap(lhs.m_depth, rhs.m_depth);
            }

            // functionality for input_iterator_tag 
            // Supports equality/inequality comparisons
            // Can be dereferenced as an rvalue
            RecurseIterator operator++(int) //postfix increment
            {
                RecurseIterator beforeInc(*this);
                ++(*this);
                return beforeInc;
            }
            value_type operator*() const
            {
                return *m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<!_IsConst>>
            pointer operator->()
            {
                return m_item;
            }
            template<bool _IsConst = IsConst, class = std::enable_if_t<_IsConst>>
            pointer operator->() const
            {
                return m_item;
            }
            friend bool operator==(const RecurseIterator& lhs, const RecurseIterator& rhs)
            {
                return (lhs.m_item == rhs.m_item); // this does not matter: && (lhs.m_recurseChildren == rhs.m_recurseChildren);
            }
            friend bool operator!=(const RecurseIterator& lhs, const RecurseIterator& rhs)
            {
                return !(lhs == rhs);
            }

            // functionality for out_iterator_tag 
            // Can be dereferenced as an lvalue
            // (only for mutable iterator types)
            //reference operator*(); // const
            // already defined

            // RecurseIterator operator++(int) //postfix increment
            // already defined

            // functionality for forward_iterator_tag
            // default-constructible
            // Multi-pass: neither dereferencing nor incrementing affects dereferenceability
            RecurseIterator() = default;
            // RecurseIterator() : m_item(nullptr), m_recurseChildren(true) {}
        protected:
            pointer m_item = nullptr;
            int m_depth = 0;
            bool m_recurseChildren = true;
        };
        #pragma endregion

        //using children_iterable = Iterable<ChildrenIterator, ChildrenIterator>;


        #pragma region iterators 
        using children_iterator = ChildrenIterator<false>;
        using recurse_iterator = RecurseIterator<false>;
        using iterator = RecurseIterator<false>;

        using const_children_iterator = ChildrenIterator<true>;
        using const_recurse_iterator = RecurseIterator<true>;
        using const_iterator = RecurseIterator<true>;

        static_assert(std::is_copy_constructible_v<children_iterator>,                  "std::is_copy_constructible_v<children_iterator>");
        static_assert(std::is_copy_constructible_v<recurse_iterator>,                   "std::is_copy_constructible_v<recurse_iterator>");
        static_assert(std::is_copy_constructible_v<iterator>,                           "std::is_copy_constructible_v<iterator>");

        static_assert(std::is_copy_constructible_v<const_children_iterator>,            "std::is_copy_constructible_v<const_children_iterator>");
        static_assert(std::is_copy_constructible_v<const_recurse_iterator>,             "std::is_copy_constructible_v<const_recurse_iterator>");
        static_assert(std::is_copy_constructible_v<const_iterator>,                     "std::is_copy_constructible_v<const_iterator>");

        static_assert(std::is_trivially_copy_constructible_v<const_children_iterator>,  "std::is_trivially_copy_constructible_v<const_children_iterator>");
        static_assert(std::is_trivially_copy_constructible_v<const_recurse_iterator>,   "std::is_trivially_copy_constructible_v<const_recurse_iterator>");
        static_assert(std::is_trivially_copy_constructible_v<const_iterator>,           "std::is_trivially_copy_constructible_v<const_iterator>");

        template <typename T> using children_as_iterator         = TypeCastIterator   < children_iterator       , T>;
        template <typename T> using recurse_as_iterator          = TypeCastIterator   < recurse_iterator        , T>;

        template <typename T> using children_data_iterator       = DataMemberIterator < children_iterator       , T>;
        template <typename T> using recurse_data_iterator        = DataMemberIterator < recurse_iterator        , T>;

        template <typename T> using const_children_as_iterator   = TypeCastIterator   < const_children_iterator , T>;
        template <typename T> using const_recurse_as_iterator    = TypeCastIterator   < const_recurse_iterator  , T>;

        template <typename T> using const_children_data_iterator = DataMemberIterator < const_children_iterator , T>;
        template <typename T> using const_recurse_data_iterator  = DataMemberIterator < const_recurse_iterator  , T>;

        // non-const & const, children & recurse, over Hierarchy, or typecasted as T, or over data member typecasted as T
                              inline iterator                        begin()                   const { return iterator(m_begin);                        }
                              inline iterator                        end()                     const { return iterator(m_end);                          }
     
                              inline children_iterator               begin_children()                { return children_iterator(m_begin);               }
                              inline children_iterator               end_children()                  { return children_iterator(m_end);                 }
     
                              inline recurse_iterator                begin_recurse()                 { return recurse_iterator(this);                   }
                              inline recurse_iterator                end_recurse()                   { return recurse_iterator(m_end);                  }
     
        template <typename T> inline children_as_iterator<T>         begin_children_as()             { return children_as_iterator<T>(m_begin);         }
        template <typename T> inline children_as_iterator<T>         end_children_as()               { return children_as_iterator<T>(m_end);           }
     
        template <typename T> inline recurse_as_iterator<T>          begin_recurse_as()              { return recurse_as_iterator<T>(this);             }
        template <typename T> inline recurse_as_iterator<T>          end_recurse_as()                { return recurse_as_iterator<T>(m_end);            }
     
        template <typename T> inline children_data_iterator<T>       begin_children_data()           { return children_data_iterator<T>(m_begin);       }
        template <typename T> inline children_data_iterator<T>       end_children_data()             { return children_data_iterator<T>(m_end);         }
     
        template <typename T> inline recurse_data_iterator<T>        begin_recurse_data()            { return recurse_data_iterator<T>(this);           }
        template <typename T> inline recurse_data_iterator<T>        end_recurse_data()              { return recurse_data_iterator<T>(m_end);          }
     
                              inline const_iterator                  cbegin()                  const { return const_iterator(m_begin);                  }
                              inline const_iterator                  cend()                    const { return const_iterator(m_end);                    }
      
                              inline const_children_iterator         cbegin_children()         const { return const_children_iterator(m_begin);         }
                              inline const_children_iterator         cend_children()           const { return const_children_iterator(m_end);           }
      
                              inline const_recurse_iterator          cbegin_recurse()          const { return const_recurse_iterator(this);             }
                              inline const_recurse_iterator          cend_recurse()            const { return const_recurse_iterator(m_end);            }
      
        template <typename T> inline const_children_as_iterator<T>   cbegin_children_as()      const { return const_children_as_iterator<T>(m_begin);   }
        template <typename T> inline const_children_as_iterator<T>   cend_children_as()        const { return const_children_as_iterator<T>(m_end);     }
     
        template <typename T> inline const_recurse_as_iterator<T>    cbegin_recurse_as()       const { return const_recurse_as_iterator<T>(this);       }
        template <typename T> inline const_recurse_as_iterator<T>    cend_recurse_as()         const { return const_recurse_as_iterator<T>(m_end);      }
     
        template <typename T> inline const_children_data_iterator<T> cbegin_children_data_as() const { return const_children_data_iterator<T>(m_begin); }
        template <typename T> inline const_children_data_iterator<T> cend_children_data_as()   const { return const_children_data_iterator<T>(m_end);   }
     
        template <typename T> inline const_recurse_data_iterator<T>  cbegin_recurse_data_as()  const { return const_recurse_data_iterator<T>(this);     }
        template <typename T> inline const_recurse_data_iterator<T>  cend_recurse_data_as()    const { return const_recurse_data_iterator<T>(m_end);    }
        #pragma endregion

        #pragma region iterables 
                              using children_iterable            = Iterable< children_iterator               >;
                              using recurse_iterable             = Iterable< recurse_iterator                >;
                              using const_children_iterable      = Iterable< const_children_iterator         >;
                              using const_recurse_iterable       = Iterable< const_recurse_iterator          >;

        template <typename T> using children_as_iterable         = Iterable< children_as_iterator        <T> >;
        template <typename T> using recurse_as_iterable          = Iterable< recurse_as_iterator         <T> >;
        template <typename T> using children_data_iterable       = Iterable< children_data_iterator      <T> >;
        template <typename T> using recurse_data_iterable        = Iterable< recurse_data_iterator       <T> >;

        template <typename T> using const_children_as_iterable   = Iterable< const_children_as_iterator  <T> >;
        template <typename T> using const_recurse_as_iterable    = Iterable< const_recurse_as_iterator   <T> >;
        template <typename T> using const_children_data_iterable = Iterable< const_children_data_iterator<T> >;
        template <typename T> using const_recurse_data_iterable  = Iterable< const_recurse_data_iterator <T> >;

        //non-const & const, children & recurse, over Hierarchy, or typecasted as T, or over data member typecasted as T
                              children_iterable               inline children()                  { return make_iterable(begin_children(), end_children());                          }
                              recurse_iterable                inline recurse()                   { return make_iterable(begin_recurse(), end_recurse());                            }

        template <typename T> children_as_iterable<T>         inline children_as()               { return make_iterable(begin_children_as<T>(), end_children_as<T>());              }
        template <typename T> children_as_iterable<T>         inline recurse_as()                { return make_iterable(begin_recurse_as<T>(), end_recurse_as<T>());                }
        
        template <typename T> children_data_iterable<T>       inline children_data()             { return make_iterable(begin_children_as<T>(), end_children_as<T>());              }
        template <typename T> recurse_data_iterable<T>        inline recurse_data()              { return make_iterable(begin_recurse_data_as<T>(), end_recurse_data_as<T>());      }

                              const_children_iterable         inline const_children()      const { return make_iterable(cbegin_children(), cend_children());                        }
                              const_recurse_iterable          inline const_recurse()       const { return make_iterable(cbegin_recurse(), cend_recurse());                          }

        template <typename T> const_children_as_iterable<T>   inline const_children_as()   const { return make_iterable(cbegin_children_as<T>(), cend_children_as<T>());            }
        template <typename T> const_recurse_as_iterable<T>    inline const_recurse_as()    const { return make_iterable(cbegin_recurse_as<T>(), cend_recurse_as<T>());              }

        template <typename T> const_children_data_iterable<T> inline const_children_data() const { return make_iterable(cbegin_children_data_as<T>(), cend_children_data_as<T>());  }
        template <typename T> const_recurse_data_iterable<T>  inline const_recurse_data()  const { return make_iterable(cbegin_recurse_data_as<T>(), cend_recurse_data_as<T>());    }
        #pragma endregion

        #pragma region visitor

        template 
        <
            typename iterator_t = Hierarchy::recurse_iterator,
            typename argument_t = typename iterator_t::pointer
        >
        class Visitor
        {
        public:
            using iterator = iterator_t;
            using argument_type = argument_t;
            // using pointer = typename std::conditional_t< IsConst, Hierarchy::const_pointer, Hierarchy::pointer >;
            // using iterator = typename std::conditional_t< IsConst, Hierarchy::const_recurse_iterator, Hierarchy::recurse_iterator >;

            struct Visit
            {
                Visit(Visitor& visitor, iterator item, int depth, int index)
                    : visitor(visitor), item(item), depth(depth), index(index)
                {}

                Visitor& visitor;
                iterator item;
                int depth;
                int index;


                inline void children()
                {
                    visitor.children();
                }
                inline void skipChildren()
                {
                    visitor.skipChildren();
                }
                inline void all()
                {
                    visitor.all();
                }
            };

            using CallbackType = std::function<void(Visit& visit, argument_type arg)>;
            Visitor(const CallbackType& cb, iterator begin)
                : cb(cb), begin(begin)
            {
                reset();
            }

            inline bool finished() const { return stack.empty(); }

            inline void reset()
            {
                stack.clear();
                stack.emplace_back(begin, 0, 0, false);
            }

            inline void all()
            // iterate until all items are visited
            {
                auto end = iterator(nullptr);
                while (stack.size())
                {
                    next();
                }
            }

            inline void children()
            // iterate until all children are visited
            {
                if (stack.empty()) return;
                auto parent = stack.back();
                while (stack.size()) 
                {

                    next();
                    if (stack.empty()) return;

                    auto currentDepth = stack.size()-1;
                    auto currentIndex = stack.back().index;
                    bool isChild = (currentDepth > parent.depth);
                    if (isChild) continue;
                    bool isSiblingOfParent = ((currentDepth == parent.depth) && (currentIndex > parent.index));
                    bool isSiblingOfTransitiveParent = (currentDepth < parent.depth);
                    if (isSiblingOfParent || isSiblingOfTransitiveParent)
                    {
                        return;
                    }
                } 
            }

            inline void skipChildren()
            // advance iterator to skip children
            {
                if (stack.empty()) return;
                auto& item = stack.back();
                item.it.skipChildren();
            }

            inline void skipSiblings()
            {
                if (stack.empty()) return;
                stack.pop_back();
                skipChildren();            
            }

            inline void next()
            {
                if(stack.empty()) return;
                //while(stack.size())
                //{
                    if(!stack.back().invoked)
                    {
                        invoke(stack.back());
                        return;
                    }
                    advance();

                //}
            }

        protected:


            const CallbackType& cb;
            iterator begin;
            const iterator end = iterator(nullptr);

            struct StackItem
            {
                iterator it;
                int depth;
                int index;
                bool invoked;
                StackItem() = default;
                StackItem(iterator it, int depth, int index, bool invoked)
                    : it(it), depth(depth), index(index), invoked(invoked)
                {}
            };
            
            std::vector<StackItem> stack;

            inline void invoke(StackItem& item)
            {
                item.invoked = true;
                Visit visit(*this, item.it, item.depth, item.index);
                cb(visit, item.it);
            }
            inline void advance()
            {
                int depth = static_cast<int>(stack.size()-1);
                auto it = stack.back().it;
                if (it == end) return;
                int old_depth = it.depth();
                assert(depth == old_depth);

                // actually advance iterator
                ++it;

                if(it.depth() > old_depth)
                {
                    stack.emplace_back(it, it.depth(), 0, false);
                }
                else if (it != end) // && (it.depth() <= old_depth)
                {
                    // either advanced the iterator at current level (it.depth() == old_depth)
                    // or advanced the iterator at above level (it.depth() < old_depth)
                    // when above level has already reached last element, the next level above it will
                    // be considered and so on. in the end the 
                    stack.resize(it.depth()+1);
                    assert(stack[it.depth()].depth == it.depth());
                    it.includeChildren(); // previous iterator may have skipped Children, reset to default
                    stack[it.depth()].it = it;
                    stack[it.depth()].invoked = false;
                    ++stack[it.depth()].index;
                } else // if (it == end)
                {
                    stack.clear();
                }
            }
        };

        template <    
            typename iterator = Hierarchy::recurse_iterator,
            typename argument_type = typename iterator::pointer
        >
        inline void visit(const typename Visitor<iterator, argument_type>::CallbackType& cb)
        {
            Visitor<iterator, argument_type> visitor(cb, this);
            visitor.all();
        }

        #pragma endregion

        #pragma region mutators
        inline iterator insert(iterator pos, pointer item)
        {
            return iterator(insert(static_cast<pointer>(pos), item));
        }
        inline children_iterator insert(children_iterator pos, pointer item)
        {
            return children_iterator(insert(static_cast<pointer>(pos), item));
        }
        // same as iterator insert(iterator pos, pointer item)
        //recurse_iterator insert(recurse_iterator pos, pointer item)
        //{
        //    return recurse_iterator(insert(*pos, item));
        //}

        inline iterator insert(iterator pos, std::initializer_list<pointer> items)
        {
            return iterator( insert(static_cast<pointer>(pos), items) );
        }
        inline children_iterator insert(children_iterator pos, std::initializer_list<pointer> items)
        {
            return children_iterator(insert(static_cast<pointer>(pos), items));
        }
        // same as iterator insert(iterator pos, std::initializer_list<pointer> items)
        //recurse_iterator insert(recurse_iterator pos, std::initializer_list<pointer> items)
        //{
        //    return recurse_iterator(insert(*pos, items));
        //}

        inline pointer insert(pointer pos, std::initializer_list<pointer> items)
        {
            // assert((pos == nullptr) || (pos->m_parent == this));
            if (pos && pos->m_parent != this)
                pos->m_parent->insert(pos, items);        
            for (auto& item : items)
            {
                insert(pos, item);
            }
            return *(items.begin());
        }

        inline pointer insert(pointer pos, pointer item)
        {
            assert((pos == nullptr) || (pos->m_parent == this));
            // if (pos && pos->m_parent != this)
            //     pos->m_parent->insert(pos, item);
            // pointer insert_pos = const_cast<pointer>(pos);
            if (item == pos) return item;
            if (item->m_parent && item->m_parent != this)
            {
                item->m_parent->erase(item);
            }
            if (m_begin == nullptr)
            {
                assert(m_countChildren == 0);
                // construct
                m_begin = item;
                m_last = item;
                item->m_parent = this;
                item->m_prev = nullptr;
                item->m_next = nullptr;
            }
            else if (pos == m_begin)
            {
                // push_front
                push_front(item);
                item->m_parent = this;
                item->m_prev = nullptr;
                item->m_next = m_begin;
                m_begin->m_prev = item;
                m_begin = item;
            }
            else if (pos == m_end)
            {
                // push_back
                item->m_parent = this;
                item->m_next = nullptr;
                item->m_prev = m_last;
                assert(m_last != nullptr);
                m_last->m_next = item;
                m_last = item;
            }
            else
            {
                assert(pos != nullptr); // this would push_back
                assert(pos->m_prev != nullptr); // this would be push_front
                // insert before pos
                pos->m_prev->m_next = item;
                item->m_prev = pos->m_prev;
                item->m_next = pos;
                pos->m_prev = item;
            }
            ++m_countChildren;
            return item;
        }

        // Replaces the contents of the container.
        inline void assign(std::initializer_list<pointer> items)
        {
            clear();
            insert(m_begin, items);
        }

        inline void assign(pointer item)
        {
            clear();
            insert(m_begin, item);
        }

        // static void push_front_into(pointer parent, pointer item)
        // {
        //     if (parent) parent->push_front(item);
        //     else item->erase(); 
        // }

        // static void push_back_into(pointer parent, pointer item)
        // {
        //     if (parent) parent->push_back(item);
        //     else item->erase(); 
        // }

        inline void push_front_into(pointer parent)
        {
            if (parent) parent->push_front(this);
            else erase(); 
        }

        inline void push_back_into(pointer parent)
        {
            if (parent) parent->push_back(this);
            else erase(); 
        }

        inline void push_front(pointer item)
        {
            insert(m_begin, item);
        }

        inline void push_back(pointer item)
        {
            insert(m_end, item);
        }

        inline void clear()
        {
            for (auto& item : children())
            {
                item.m_parent = nullptr;
            }
            m_begin = nullptr;
            m_last = nullptr;
        }

        inline pointer erase_from_parent()
        {
            return erase();
        }

        inline pointer erase()
        {
            return m_parent ? m_parent->erase(this) : nullptr;
        }

        inline pointer erase(pointer item)
        {
            if ((item == nullptr) || (item->m_parent == nullptr)) return m_begin;
            // assert(item->m_parent == this);
            if (item->m_parent != this)
                item->m_parent->erase(item);
            
            pointer next_item = static_cast<pointer>(++recurse_iterator(item));

            if (item->m_prev != nullptr)
            {
                item->m_prev->m_next = item->m_next;
            }
            if (item->m_next != nullptr)
            {
                item->m_next->m_prev = item->m_prev;
            }
            if (item == m_begin) // and not nullptr
            {
                m_begin = item->m_next;
            }
            if (item == m_last)
            {
                m_last = item->m_prev;
            }
            item->m_parent = nullptr;
            item->m_prev = nullptr;
            item->m_next = nullptr;
            --m_countChildren;
            return next_item;
        }

        inline void pop_front()
        {
            erase(m_begin);
        }

        inline void pop_back()
        {
            erase(m_last);
        }
        #pragma endregion
    };

// further information:
// https://stackoverflow.com/questions/8054273/how-to-implement-an-stl-style-iterator-and-avoid-common-pitfalls
// https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators
// https://stackoverflow.com/questions/37031805/preparation-for-stditerator-being-deprecated/38103394
// https://internalpointers.com/post/writing-custom-iterators-modern-cpp
// https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
// https://stackoverflow.com/a/56697826/798588
// https://stackoverflow.com/a/49425072/798588
// https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/

} // namespace transform_tree_glm
