#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtc/matrix_inverse.hpp> // glm::affineInverse
#include <glm/gtx/euler_angles.hpp> // glm::eulerAngleXYZ
#include <glm/gtx/matrix_decompose.hpp> // glm::decompose
#include <glm/gtx/transform.hpp> // glm::scale

namespace transform_tree_glm {

    class Pose 
    {
    protected:
        #pragma region data members
        glm::vec3 m_position;
        glm::quat m_rotation;
        glm::vec3 m_scale;

        glm::mat4 m_pose;
        // bool m_dirtyPose = true;
        #pragma endregion


    public:
        inline static Pose identity() { return Pose(glm::mat4(1)); };

    public:
        #pragma region constructors
        Pose()
            : m_position(glm::vec3(0,0,0))
            , m_rotation()
            , m_scale(glm::vec3(1,1,1))
        {}

        Pose(const Pose& other)
            : m_position(other.m_position)
            , m_rotation(other.m_rotation)
            , m_scale(other.m_scale)
        {}

        Pose(const glm::mat4& pose)
            : m_position(glm::vec3(0,0,0))
            , m_rotation()
            , m_scale(glm::vec3(1,1,1))
        {
            setLocalPose(pose);
        }

        Pose(const glm::mat4& pose, const glm::vec3& scale) 
            : Pose(pose * glm::scale(scale))
        {}

        Pose(const glm::vec3& position, const glm::mat3& rotation, const glm::vec3& scale=glm::vec3(1,1,1)) 
            : m_position(position)
            , m_rotation(rotation)
            , m_scale(scale)
        {}
        Pose(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale=glm::vec3(1,1,1)) 
            : m_position(position)
            , m_rotation(rotation)
            , m_scale(scale)
        {}

        Pose(const glm::vec3& position, const glm::vec3& eulerXYZ, const glm::vec3& scale=glm::vec3(1,1,1)) 
            : m_position(position)
            , m_rotation(RotationEulerXYZ(eulerXYZ))
            , m_scale(scale)
        {}
        Pose(const glm::vec3& position, float eulerX, float eulerY, float eulerZ, const glm::vec3& scale=glm::vec3(1,1,1)) 
            : m_position(position)
            , m_rotation(glm::eulerAngleXYZ(eulerX, eulerY, eulerZ))
            , m_scale(scale)
        {}

        static glm::mat4 RotationEulerXYZ(const glm::vec3& eulerXYZ) 
        {
            return glm::eulerAngleXYZ(eulerXYZ.x, eulerXYZ.y, eulerXYZ.z);
        }
        static glm::vec3 ExtractEulerXYZ(const glm::mat4& rotation) 
        { 
            glm::vec3 result; 
            glm::extractEulerAngleXYZ(rotation, result.x, result.y, result.z); 
            return result;
        }
        #pragma endregion

    public:
        #pragma region directly access transformation parameters position, rotation and scale

        inline const glm::vec3& accessConstLocalPosition() const { return m_position; }
        inline const glm::quat& accessConstLocalRotation() const { return m_rotation; }
        inline const glm::vec3& accessConstLocalScale() const { return m_scale; }
        inline glm::vec3& accessLocalPosition() { return m_position; }
        inline glm::quat& accessLocalRotation() { return m_rotation; }
        inline glm::vec3& accessLocalScale() { return m_scale; }
        
        #pragma endregion

    public:
        #pragma region transformation matrices and quaternion for transformation parameters
        inline glm::mat4 localTranslationMatrix() const { return glm::translate(m_position); }
        inline glm::mat4 localScaleMatrix() const { return glm::scale(m_scale); }
        inline glm::quat localRotationQuaternion() const { return m_rotation; }
        inline glm::mat4 localRotationMatrix() const { return glm::mat4(m_rotation); }
        #pragma endregion

    public:
        #pragma region get local pose, position, rotation & scale in various formats
        glm::vec3 localPosition() const { return m_position; }
        glm::mat3 localRotation() const { return /*throw away last row and col to make it a rotation matrix*/ glm::mat3(localRotationMatrix()); }
        glm::vec3 localScale() const { return m_scale; }

        const glm::mat4& localPose() { 
            // if (m_dirtyPose)
            {
                m_pose = glm::mat4(m_rotation);
                //m_pose = m_pose * glm::scale(m_scale);
                m_pose = glm::scale(m_pose, m_scale);
                m_pose[3].xyz = m_position;
                // m_dirtyPose = false;
            }
            return m_pose;
        }
        glm::vec3 localRotationEulerXYZ() const { return ExtractEulerXYZ(localRotation()); }

        inline operator glm::mat4() const { return localTranslationMatrix(); }
        #pragma endregion
        
    public:
        #pragma region set local pose, position, rotation & scale in various formats
        void setLocalPosition(const glm::vec3& position) 
        { 
            m_position = position;
            // m_dirtyPose = true;
        }

        void setLocalRotation(const glm::mat3& rotation) 
        {
            m_rotation = rotation;
            // m_dirtyPose = true;
        }
        void setLocalRotation(const glm::quat& rotation) 
        {
            m_rotation = rotation;
            // m_dirtyPose = true;
        }
        void setLocalRotation(const glm::vec3& eulerXYZ) 
        {
            setLocalRotation(glm::quat(RotationEulerXYZ(eulerXYZ)));
        }
        void setLocalRotationEulerXYZ(const glm::vec3& rotation) 
        {
            setLocalRotation(glm::quat(RotationEulerXYZ(rotation)));
        }
        void setLocalScale(const glm::vec3& scale)
        {
            m_scale = scale;
            // m_dirtyPose = true;
        }

        void setLocalPose(const glm::vec3& position, const glm::mat3& rotation, const glm::vec3& scale = glm::vec3(0,0,0))
        {
            setLocalPosition(position);
            setLocalRotation(rotation);
            setLocalScale(scale);
        }

        void setLocalPose(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale = glm::vec3(0,0,0))
        {
            setLocalPosition(position);
            setLocalRotation(rotation);
            setLocalScale(scale);
        }

        void setLocalPose(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale = glm::vec3(0,0,0))
        {
            setLocalPosition(position);
            setLocalRotation(rotation);
            setLocalScale(scale);
        }

        void setLocalPose(const glm::mat4& pose)
        {
            glm::vec3 scale;
            glm::quat orientation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            if (glm::decompose(pose, scale, orientation, translation, skew, perspective))
            {
                m_scale = scale;
                m_position = translation;
                m_rotation = orientation;
                // m_dirtyPose = true;
            }
        }
        #pragma endregion

    };

} // namespace transform_tree_glm
