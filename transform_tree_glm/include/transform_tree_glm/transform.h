#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtc/matrix_inverse.hpp> // glm::affineInverse
#include <glm/gtx/euler_angles.hpp> // glm::eulerAngleXYZ
#include <glm/gtx/matrix_decompose.hpp> // glm::decompose
#include <glm/gtx/transform.hpp> // glm::scale

#include "transform_tree_glm/pose.h"
#include "transform_tree_glm/hierarchy.h"

namespace transform_tree_glm {

    template<typename name_value_t = std::string, typename idx_t = int>
    class Transform_ : protected Pose
    {
    public:
        using pointer = Transform_*;
        using const_pointer = const Transform_*;
        using pose_type = Pose;

        using idx_type = idx_t;
        using name_value_type = name_value_t;

        Transform_()
            : Pose()
            , m_hierarchy(this)
        {}
        Transform_(const Pose& pose)
            : Pose(pose)
            , m_hierarchy(this)
        {}

        Transform_(const std::string& name, const Pose& pose = Pose::identity())
            : Pose(pose)
            , m_hierarchy(this)
            , name(name)
        {
        }

        Transform_(pointer parent, const Pose& pose = Pose::identity())
            : Pose(pose)
            , m_hierarchy(this)
            , name("")
        {
            setParent(parent);
        }

        Transform_(void* data, pointer parent, const Pose& pose = Pose::identity())
            : Pose(pose)
            , m_hierarchy(this)
            , data(data)
            , name("")
        {
            setParent(parent);
        }

        Transform_(const std::string& name, pointer parent, const Pose& pose = Pose::identity())
            : Pose(pose)
            , m_hierarchy(this)
            , name(name)
        {
            setParent(parent);
        }

        Transform_(const std::string& name, void* data, pointer parent, const Pose& pose = Pose::identity())
            : Pose(pose)
            , m_hierarchy(this)
            , data(data)
            , name(name)
        {
            setParent(parent);
        }

    protected:
        Hierarchy m_hierarchy;
    public:
        void* data = nullptr;
        // using Hierarchy::data;
        name_value_type name;

        inline operator Hierarchy::pointer()
        {
            return &m_hierarchy;
        }

        #pragma region transformation hierarchy
        inline glm::mat4 transformParentToRoot() { return parent() ? parent()->transformLocalToRoot() : glm::mat4(1); }
        inline glm::mat4 transformLocalToRoot() { return transformParentToRoot() * localPose(); }
        inline const glm::mat4& transformLocalToParent() { return localPose(); }
        #pragma endregion

    public:
        #pragma region get local and world pose, position, rotation & scale in various formats
        inline glm::vec3 worldPosition() { return transformLocalToRoot()[3].xyz; }
        inline glm::mat3 worldRotation() { return /*throw away last row and col to make it a rotation matrix*/ glm::mat3(transformLocalToRoot()); }
        inline glm::mat4 worldPose() { return transformLocalToRoot(); }
        inline glm::vec3 worldRotationEulerXYZ() { return ExtractEulerXYZ(worldRotation()); }
        #pragma endregion

    public:
        #pragma region set local pose, position, rotation & scale in various formats

        inline void setWorldPosition(const glm::vec3& position) 
        {
            setWorldPosition(glm::vec4(position, 1));
        }
        inline void setWorldPosition(const glm::vec4& position) 
        { 
            glm::mat4 root_parent = transformParentToRoot();
            glm::mat4 parent_root = glm::affineInverse(root_parent);
            glm::vec4 pos_in_parent = parent_root * position;
            setLocalPosition(pos_in_parent);
        }

        inline void setWorldRotation(const glm::mat3& rotation) 
        { 
            glm::mat4 root_parent = transformParentToRoot();
            glm::mat4 parent_root = glm::affineInverse(root_parent);
            glm::mat3 rotation_in_parent = glm::mat3(parent_root) * rotation;
            setLocalRotation(rotation_in_parent);
        }

        inline void setWorldRotation(const glm::quat& rotation) 
        {
            setWorldRotation(glm::mat3(rotation));
        }

        inline void setWorldRotation(const glm::vec3& eulerXYZ) 
        {
            setWorldRotation(glm::quat(RotationEulerXYZ(eulerXYZ)));
        }

        inline void setWorldRotationEulerXYZ(const glm::vec3& rotation) 
        {
            setWorldRotation(glm::quat(RotationEulerXYZ(rotation)));
        }

        inline void setWorldPose(const glm::vec3& position, const glm::mat3& rotation, const glm::vec3& scale=glm::vec3(1,1,1)) 
        {
            setWorldPose(Pose(position, rotation, scale));
        }
        inline void setWorldPose(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale=glm::vec3(1,1,1)) 
        {
            setWorldPose(Pose(position, rotation, scale));
        }
        inline void setWorldPose(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale=glm::vec3(1,1,1)) 
        {
            setWorldPose(Pose(position, rotation, scale));
        }

        inline void setWorldPose(const glm::mat4& pose)
        {
            glm::mat4 root_parent = transformParentToRoot();
            glm::mat4 parent_root = glm::affineInverse(root_parent);
            glm::mat4 pose_in_parent = parent_root * pose;
            setLocalPose(pose_in_parent);
        }

        inline bool setParentKeepWorldPose(const pointer& newParent)
        {
            if (m_parent == newParent) return false;
            glm::mat4 oldWorldPose = worldPose();
            setParent(newParent);
            setWorldPose(oldWorldPose);
            return true;
        }        
        #pragma endregion
    public:
        #pragma region Pose access
        using Pose::RotationEulerXYZ;
        using Pose::ExtractEulerXYZ;
        using Pose::accessConstLocalPosition;
        using Pose::accessConstLocalRotation;
        using Pose::accessConstLocalScale;
        using Pose::accessLocalPosition;
        using Pose::accessLocalRotation;
        using Pose::accessLocalScale;
        using Pose::localTranslationMatrix;
        using Pose::localScaleMatrix;
        using Pose::localRotationQuaternion;
        using Pose::localRotationMatrix;
        using Pose::localPosition;
        using Pose::localRotation;
        using Pose::localScale;
        using Pose::localPose;
        using Pose::localRotationEulerXYZ;
        using Pose::setLocalPosition;
        using Pose::setLocalRotation;
        using Pose::setLocalRotationEulerXYZ;
        using Pose::setLocalScale;
        using Pose::setLocalPose;     
        #pragma endregion

    public:


        #pragma region hierarchy property access
        // inline operator pointer() { return hierarchy; }
        // inline operator pointer() const { return hierarchy; }
        // inline pointer pointer() const { return hierarchy.pointer(); }
        // inline pointer& pointer() { return hierarchy.pointer(); }
        // inline const pointer& cpointer() const { return hierarchy.cpointer(); }

        // inline idx_type indexInParent() const { return hierarchy.indexInParent(); }
        // inline void updateIndexInParent(idx_type idx) { hierarchy.updateIndexInParent(idx); }

        inline bool empty()                   const { return m_hierarchy.empty(); }
        inline Hierarchy::size_type size() const { return m_hierarchy.size(); }

        inline const_pointer parent() const { return m_hierarchy.parent() ? static_cast<const_pointer>(m_hierarchy.parent()->data) : nullptr;  }
        inline pointer       parent()       { return m_hierarchy.parent() ? static_cast<pointer>(m_hierarchy.parent()->data)       : nullptr; }

        inline const_pointer prev()   const { return m_hierarchy.prev() ? static_cast<const_pointer>(m_hierarchy.prev()->data) : nullptr;  }
        inline pointer       prev()         { return m_hierarchy.prev() ? static_cast<pointer>(m_hierarchy.prev()->data)       : nullptr; }

        inline const_pointer next()   const { return m_hierarchy.next() ? static_cast<const_pointer>(m_hierarchy.next()->data) : nullptr;  }
        inline pointer       next()         { return m_hierarchy.next() ? static_cast<pointer>(m_hierarchy.next()->data)       : nullptr; }

        inline const_pointer front()  const { return m_hierarchy.front() ? static_cast<const_pointer>(m_hierarchy.front()->data) : nullptr;  }
        inline pointer       front()        { return m_hierarchy.front() ? static_cast<pointer>(m_hierarchy.front()->data)       : nullptr; }

        inline const_pointer back()   const { return m_hierarchy.back() ? static_cast<const_pointer>(m_hierarchy.back()->data) : nullptr;  }
        inline pointer       back()         { return m_hierarchy.back() ? static_cast<pointer>(m_hierarchy.back()->data)       : nullptr; }
        #pragma endregion

        #pragma region hierarchy iterators and iterables
        // code line length is very wide for this section as it allows MUCH better grouping of similar code fragments.
        // horizontal code scrolling for this section is still preferable to the uglyness which would result from more line breaks.
                              using children_iterator            = DataMemberIterator< Hierarchy::children_iterator,       Transform_ >;
                              using recurse_iterator             = DataMemberIterator< Hierarchy::recurse_iterator,        Transform_ >;
                              using const_children_iterator      = DataMemberIterator< Hierarchy::const_children_iterator, Transform_ >;
                              using const_recurse_iterator       = DataMemberIterator< Hierarchy::const_recurse_iterator,  Transform_ >;

        template <typename T> using children_data_iterator       = DataMemberIterator< children_iterator,       T >;
        template <typename T> using recurse_data_iterator        = DataMemberIterator< recurse_iterator,        T >;
        template <typename T> using const_children_data_iterator = DataMemberIterator< const_children_iterator, T >;
        template <typename T> using const_recurse_data_iterator  = DataMemberIterator< const_recurse_iterator,  T >;

                              using children_iterable            = Iterable< children_iterator       >;           
                              using recurse_iterable             = Iterable< recurse_iterator        >;            
                              using const_children_iterable      = Iterable< const_children_iterator >;     
                              using const_recurse_iterable       = Iterable< const_recurse_iterator  >;      

        template <typename T> using children_data_iterable       = Iterable< children_data_iterator<T>       >;      
        template <typename T> using recurse_data_iterable        = Iterable< recurse_data_iterator<T>        >;       
        template <typename T> using const_children_data_iterable = Iterable< const_children_data_iterator<T> >;
        template <typename T> using const_recurse_data_iterable  = Iterable< const_recurse_data_iterator<T>  >; 

                              inline children_iterator               begin_children()              { return m_hierarchy.begin_children_data<Transform_>(); }
                              inline children_iterator               end_children()                { return m_hierarchy.end_children_data<Transform_>(); }

                              inline recurse_iterator                begin_recurse()               { return m_hierarchy.begin_recurse_data<Transform_>(); }
                              inline recurse_iterator                end_recurse()                 { return m_hierarchy.end_recurse_data<Transform_>(); }

        template <typename T> inline children_data_iterator<T>       begin_children_data()         { return children_data_iterator<T>(begin_children()); }
        template <typename T> inline children_data_iterator<T>       end_children_data()           { return children_data_iterator<T>(end_children()); }

        template <typename T> inline recurse_data_iterator<T>        begin_recurse_data()          { return recurse_data_iterator<T>(begin_recurse()); }
        template <typename T> inline recurse_data_iterator<T>        end_recurse_data()            { return recurse_data_iterator<T>(end_recurse()); }


                              inline const_children_iterator         cbegin_children()       const { return m_hierarchy.cbegin_children_data<Transform_>(); }
                              inline const_children_iterator         cend_children()         const { return m_hierarchy.cend_children_data<Transform_>(); }

                              inline const_recurse_iterator          cbegin_recurse()        const { return m_hierarchy.cbegin_recurse_data<Transform_>(); }
                              inline const_recurse_iterator          cend_recurse()          const { return m_hierarchy.cend_recurse_data<Transform_>(); }

        template <typename T> inline const_children_data_iterator<T> cbegin_children_data()  const { return const_children_data_iterator<T>(cbegin_children()); }
        template <typename T> inline const_children_data_iterator<T> cend_children_data()    const { return const_children_data_iterator<T>(cend_children()); }

        template <typename T> inline const_recurse_data_iterator<T>  cbegin_recurse_data()   const { return const_recurse_data_iterator<T>(cbegin_recurse()); }
        template <typename T> inline const_recurse_data_iterator<T>  cend_recurse_data()     const { return const_recurse_data_iterator<T>(cend_recurse()); }

                              inline children_iterable               children()                    { return children_iterable(begin_children(), end_children()); }
                              inline recurse_iterable                recurse()                     { return recurse_iterable(begin_recurse(), end_recurse()); }

        template <typename T> inline children_data_iterable<T>       children_data()               { return children_data_iterable<T>(begin_children_data(), end_children_data()); }
        template <typename T> inline recurse_data_iterable<T>        recurse_data()                { return recurse_data_iterable<T>(begin_recurse_data(), end_recurse_data()); }

                              inline const_children_iterable         const_children()        const { return const_children_iterable(cbegin_children(), cend_children()); }
                              inline const_recurse_iterable          const_recurse()         const { return const_recurse_iterable(cbegin_recurse(), cend_recurse()); }

        template <typename T> inline const_children_data_iterable<T> const_children_data()   const { return const_children_data_iterable<T>(cbegin_children_data(), cend_children_data()); }
        template <typename T> inline const_recurse_data_iterable<T>  const_recurse_data()    const { return const_recurse_data_iterable<T>(cbegin_recurse_data(), cend_recurse_data()); }
        #pragma endregion

        #pragma region hierarchy visitors

                              using visitor            = Hierarchy::Visitor< recurse_iterator               >;
                              using const_visitor      = Hierarchy::Visitor< const_recurse_iterator         >;

        template <typename T> using data_visitor       = Hierarchy::Visitor< recurse_data_iterator<T>       >;
        template <typename T> using const_data_visitor = Hierarchy::Visitor< const_recurse_data_iterator<T> >;
        
                              inline void visit( const typename visitor::CallbackType& cb )                     { m_hierarchy.visit<visitor::iterator, visitor::argument_type>(cb); }
                              inline void cvisit( const typename const_visitor::CallbackType& cb )              { m_hierarchy.visit<const_visitor::iterator, const_visitor::argument_type>(cb); }
        template <typename T> inline void visit_data( const typename data_visitor<T>::CallbackType& cb )        { m_hierarchy.visit<data_visitor<T>::iterator, data_visitor<T>::argument_type>(cb); }
        template <typename T> inline void cvisit_data( const typename const_data_visitor<T>::CallbackType& cb ) { m_hierarchy.visit<const_data_visitor<T>::iterator, const_data_visitor<T>::argument_type>(cb); }

        #pragma endregion

        #pragma region hierarchy mutators
        inline bool removeChild(const pointer& child) 
        // inline bool removeChild(const pointer& child, bool enableSetParent = true) 
        {
            if (child->parent() != this) return false;
            m_hierarchy.erase_from_parent(*child); 
            return true;
        }
        // { return hierarchy.removeChild(&child->hierarchy, enableSetParent); }

        inline void addChild(const pointer& child) 
        // inline bool addChild(const pointer& child, bool enableSetParent = true, bool avoidDuplicateChild = false) 
        {
            m_hierarchy.push_back(*child);
        }
        // { return hierarchy.addChild(&child->hierarchy, enableSetParent, avoidDuplicateChild); }

        inline void setParent(const pointer& newParent) 
        // inline bool setParent(const pointer& newParent, bool enableRemoveChild = true, bool enableAddChild = true, bool avoidDuplicateChild = false) 
        { 
            m_hierarchy.push_back_into(*newParent);
        }
        // { return hierarchy.setParent(&newParent->hierarchy, enableRemoveChild, enableAddChild, avoidDuplicateChild); }

        // Hierarchy<T> itself provides conversion to arbitrary Hierarchy<U>*

        // inline void setParent(const Hierarchy* newParent) 
        // // inline bool setParent(const pointer& newParent, bool enableRemoveChild = true, bool enableAddChild = true, bool avoidDuplicateChild = false) 
        // { 
        //     m_hierarchy.push_back_into(newParent);
        // }

        // inline bool setParentKeepWorldPose(const pointer& newParent) 
        // // inline bool setParentKeepWorldPose(const pointer& newParent, bool enableRemoveChild = true, bool enableAddChild = true, bool avoidDuplicateChild = false) 
        // { return hierarchy.setParentKeepWorldPose(&newParent->hierarchy, enableRemoveChild, enableAddChild, avoidDuplicateChild); }
        #pragma endregion
    };
    typedef Transform_<> Transform;
    
} // namespace transform_tree_glm
