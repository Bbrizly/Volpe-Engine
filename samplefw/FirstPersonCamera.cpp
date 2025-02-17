#include "FirstPersonCamera.h"
#include <glm/gtx/norm.hpp>

FirstPersonCamera::FirstPersonCamera(volpe::App* pApp)
    : m_pApp(pApp),
      m_position(0.0f, 0.0f, 0.0f),
      m_direction(0.0f, 0.0f, -1.0f),
      m_up(0.0f, 1.0f, 0.0f),
      m_yaw(-90.0f),
      m_pitch(0.0f),
      m_movementSpeed(2.0f),
      m_mouseSensitivity(0.1f),
      m_invertY(false),
      m_normalSpeed(2.0f),
      m_SprintSpeed(4.0f)
{
    m_lastMousePos = m_pApp->getMousePos();
    m_pApp->LockCursor();
}

FirstPersonCamera::~FirstPersonCamera() {}

void FirstPersonCamera::update(float dt) 
{
    float speedMultiplier = m_pApp->isKeyDown(GLFW_KEY_LEFT_SHIFT) ? m_SprintSpeed : m_normalSpeed;
    float adjustedSpeed = m_movementSpeed * speedMultiplier;

    glm::vec3 right = glm::normalize(glm::cross(m_direction, m_up));
    glm::vec3 forward = glm::normalize(m_direction);
    glm::vec3 movement(0.0f);

    if (m_pApp->isKeyDown('W')) {                   movement += forward;  }
    if (m_pApp->isKeyDown('S')) {                   movement -= forward;  }
    if (m_pApp->isKeyDown('A')) {                   movement -= right;    }
    if (m_pApp->isKeyDown('D')) {                   movement += right;    }
    if (m_pApp->isKeyDown(GLFW_KEY_SPACE)) {        movement += m_up;     }
    if (m_pApp->isKeyDown(GLFW_KEY_LEFT_CONTROL)) { movement -= m_up;     }
    movement *= adjustedSpeed * dt;
    m_position += movement;
    
    #pragma region Mouse Input
    if (m_pApp->isKeyJustDown('Y')) {
        m_invertY = !m_invertY;
    }
    
    if (m_pApp->isKeyJustDown(GLFW_KEY_TAB))
    {
        lockMouse = !lockMouse;
        if(lockMouse)
        {
            m_pApp->LockCursor();
            m_lastMousePos = m_pApp->getMousePos();
        }else
        {
            m_pApp->UnlockCursor();
            m_lastMousePos = m_pApp->getMousePos();
        }
    }

    glm::vec2 currentMousePos = m_pApp->getMousePos();
    glm::vec2 mouseMovement = currentMousePos - m_lastMousePos;
    _updateOrientation(mouseMovement);

    if (lockMouse)
    {
        int winWidth, winHeight;
        glfwGetWindowSize(m_pApp->getWindow(), &winWidth, &winHeight);
        glm::vec2 center(winWidth / 2.0f, winHeight / 2.0f);
        glfwSetCursorPos(m_pApp->getWindow(), center.x, center.y);
        m_lastMousePos = center;
    }
    else
    {
        m_lastMousePos = currentMousePos;
    }
    #pragma endregion
}

void FirstPersonCamera::_updateOrientation(const glm::vec2& mouseMovement)
{
    m_yaw += mouseMovement.x * m_mouseSensitivity;
    // Invert Y if m_invertY
    float yMovement = m_invertY ? mouseMovement.y : -mouseMovement.y;
    m_pitch += yMovement * m_mouseSensitivity; 

    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

    // Update direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_direction = glm::normalize(direction);
}

glm::mat4 FirstPersonCamera::getViewMatrix()
{
    return glm::lookAt(m_position, m_position + m_direction, m_up);
}

glm::mat4 FirstPersonCamera::getProjMatrix(int width, int height)
{
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_near, m_far);
}
/* accurate get frustum
Frustum FirstPersonCamera::getFrustum(int width, int height)
{
    // Assume m_fov is in radians; if it's in degrees, convert with glm::radians().
    float aspect = float(width) / float(height);
    
    // Retrieve near and far distances from the camera.
    float n = m_near; // e.g. 0.1f
    float f = m_far;  // e.g. 1000.0f

    // Compute near and far plane dimensions.
    float nearHeight = 2.0f * n * tan(m_fov / 2.0f);
    float nearWidth  = nearHeight * aspect;
    float farHeight  = 2.0f * f * tan(m_fov / 2.0f);
    float farWidth   = farHeight * aspect;
    
    // Get the camera’s position and view direction.
    // (For FirstPersonCamera, use getPosition() and getDirection();
    //  for OrbitCamera, use getViewPosition() and getViewDirection().)
    glm::vec3 camPos, camDir;
    // Here we assume your camera class provides these methods:
    camPos = getPosition();
    camDir = glm::normalize(getDirection());
    
    // Compute the camera's right and up vectors.
    // We assume a world-up of (0,1,0); if your camera uses a different up, adjust accordingly.
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 camRight = glm::normalize(glm::cross(camDir, worldUp));
    glm::vec3 camUp    = glm::normalize(glm::cross(camRight, camDir));
    
    // Compute the centers of the near and far planes.
    glm::vec3 nearCenter = camPos + camDir * n;
    glm::vec3 farCenter  = camPos + camDir * f;
    
    // Compute the 4 corners of the near plane.
    glm::vec3 ntl = nearCenter + camUp * (nearHeight / 2.0f) - camRight * (nearWidth / 2.0f);
    glm::vec3 ntr = nearCenter + camUp * (nearHeight / 2.0f) + camRight * (nearWidth / 2.0f);
    glm::vec3 nbl = nearCenter - camUp * (nearHeight / 2.0f) - camRight * (nearWidth / 2.0f);
    glm::vec3 nbr = nearCenter - camUp * (nearHeight / 2.0f) + camRight * (nearWidth / 2.0f);
    
    // Compute the 4 corners of the far plane.
    glm::vec3 ftl = farCenter + camUp * (farHeight / 2.0f) - camRight * (farWidth / 2.0f);
    glm::vec3 ftr = farCenter + camUp * (farHeight / 2.0f) + camRight * (farWidth / 2.0f);
    glm::vec3 fbl = farCenter - camUp * (farHeight / 2.0f) - camRight * (farWidth / 2.0f);
    glm::vec3 fbr = farCenter - camUp * (farHeight / 2.0f) + camRight * (farWidth / 2.0f);
    
    // Helper lambda: compute a plane (in form Ax+By+Cz+D=0) from 3 points.
    auto planeFromPoints = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) -> glm::vec4 {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        float d = -glm::dot(normal, a);
        return glm::vec4(normal, d);
    };
    
    // Build the 6 planes using three corners per plane.
    Frustum frustum;
    // Left plane: use near-top-left, near-bottom-left, far-bottom-left.
    frustum.planes[0] = planeFromPoints(ntl, nbl, fbl);
    // Right plane: use near-bottom-right, near-top-right, far-bottom-right.
    frustum.planes[1] = planeFromPoints(nbr, ntr, fbr);
    // Bottom plane: use near-bottom-left, near-bottom-right, far-bottom-right.
    frustum.planes[2] = planeFromPoints(nbl, nbr, fbr);
    // Top plane: use near-top-right, near-top-left, far-top-left.
    frustum.planes[3] = planeFromPoints(ntr, ntl, ftl);
    // Near plane: use near-top-left, near-top-right, near-bottom-right.
    frustum.planes[4] = planeFromPoints(ntl, ntr, nbr);
    // Far plane: use far-top-right, far-top-left, far-bottom-left.
    frustum.planes[5] = planeFromPoints(ftr, ftl, fbl);
    
    return frustum;
}
*/
/* getfrustum 
Frustum FirstPersonCamera::getFrustum(int width, int height)
{
    glm::mat4 proj = getProjMatrix(width, height);
    glm::mat4 view = getViewMatrix();
    glm::mat4 clip = proj * view;
    
    Frustum frustum;
    
    // Extract left plane
    frustum.planes[0] = glm::vec4(
        clip[0][3] + clip[0][0],
        clip[1][3] + clip[1][0],
        clip[2][3] + clip[2][0],
        clip[3][3] + clip[3][0]
    );
    // Extract right plane
    frustum.planes[1] = glm::vec4(
        clip[0][3] - clip[0][0],
        clip[1][3] - clip[1][0],
        clip[2][3] - clip[2][0],
        clip[3][3] - clip[3][0]
    );
    // Extract bottom plane
    frustum.planes[2] = glm::vec4(
        clip[0][3] + clip[0][1],
        clip[1][3] + clip[1][1],
        clip[2][3] + clip[2][1],
        clip[3][3] + clip[3][1]
    );
    // Extract top plane
    frustum.planes[3] = glm::vec4(
        clip[0][3] - clip[0][1],
        clip[1][3] - clip[1][1],
        clip[2][3] - clip[2][1],
        clip[3][3] - clip[3][1]
    );
    // Extract near plane
    frustum.planes[4] = glm::vec4(
        clip[0][3] + clip[0][2],
        clip[1][3] + clip[1][2],
        clip[2][3] + clip[2][2],
        clip[3][3] + clip[3][2]
    );
    // Extract far plane
    frustum.planes[5] = glm::vec4(
        clip[0][3] - clip[0][2],
        clip[1][3] - clip[1][2],
        clip[2][3] - clip[2][2],
        clip[3][3] - clip[3][2]
    );
    
    // Normalize all planes
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(frustum.planes[i]));
        frustum.planes[i] /= length;
    }
    
    return frustum;
}
*/