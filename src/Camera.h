// #include <glm/glm.hpp>

// class Camera
// {
// public:
//     //---------------------------------------------------------------------
//     // Public methods
//     //---------------------------------------------------------------------
//     SceneCamera();
//     SceneCamera(float p_fFOV, float p_fAspectRatio, float p_fNearClip, float p_fFarClip, const glm::vec3 &p_vPos, const glm::vec3& p_vTarget, const glm::vec3& p_vUp);
//     virtual ~SceneCamera();

//     void SetFOV(float p_fFOV);
//     float GetFOV() const;

//     void SetAspectRatio(float p_fAspectRatio);
//     float GetAspectRatio() const;

//     void SetNearClip(float p_fNearClip);
//     float GetNearClip() const;

//     void SetFarClip(float p_fFarClip);
//     float GetFarClip() const;

//     void SetPos(const glm::vec3 &p_vPos);
//     const glm::vec3& GetPos() const;

//     void SetTarget(const glm::vec3& p_vTarget);
//     const glm::vec3& GetTarget() const;
//     void UpdateTarget(const glm::vec3& t);

//     void SetUp(const glm::vec3& p_vUp);
//     const glm::vec3& GetUp() const;

//     const glm::mat4& GetProjectionMatrix() const;
//     virtual const glm::mat4& GetViewMatrix() const;

//     glm::vec3 m_vPos;

// private:
//     //---------------------------------------------------------------------
//     // Private members
//     //---------------------------------------------------------------------

//     // Camera projection properties
//     float m_fFOV;
//     float m_fAspectRatio;
//     float m_fNearClip;
//     float m_fFarClip;

//     // Position, target and up vectors
//     glm::vec3 m_vTarget;
//     glm::vec3 m_vUp;

//     // Optimization; prevent calculating view/projection unless properties change
//     mutable bool m_bProjectionDirty;
//     mutable bool m_bViewDirty;

//     // Calculated projection matrix
//     mutable glm::mat4 m_mProjectionMatrix;

//     // Calculated view matrix
//     mutable glm::mat4 m_mViewMatrix;
// };