#include "OrbitCamera.h"

OrbitCamera::OrbitCamera(volpe::App* pApp)
    : m_pApp(pApp)
{
    m_lastMousePos = m_pApp->getMousePos();
}

OrbitCamera::~OrbitCamera()
{

}

void OrbitCamera::update(float dt)
{
    glm::vec2 mousePos = m_pApp->getMousePos();

    if(m_pApp->isLMBDown())
    {
        glm::vec2 mouseMovement = mousePos - m_lastMousePos;
        _rotate(mouseMovement);
    }
    else if(m_pApp->isMMBDown())
    {
        glm::vec2 mouseMovement = mousePos - m_lastMousePos;
        _pan(mouseMovement);
    }

    glm::vec2 mouseScroll = m_pApp->getMouseScroll();

    if(mouseScroll.y > 0) {
        m_distance -= (m_distance / 5.0f);
    } else if(mouseScroll.y < 0) {
        m_distance += (m_distance / 5.0f);
    }

    m_distance = volpe::max(10.0f, m_distance);

    m_far = volpe::max(150.0f, m_distance * 2.0f);
    m_near = m_distance / 10.0f;

    if(m_pApp->isKeyDown('f'))
    {
        focusOn(m_focusMin,m_focusMax);
    }

    m_lastMousePos = mousePos;
}

glm::mat4 OrbitCamera::getViewMatrix()
{
    glm::mat4 m(1.0f);
    m = glm::rotate(m_rotY, glm::vec3(0.0f,1.0f,0.0f));
    m = m * glm::rotate(m_rotX, glm::vec3(1.0f,0.0f,0.0f));

    m_position = m * glm::vec4(0.0f,0.0f,m_distance,1.0f);

    glm::vec3 up = m * glm::vec4(0.0f,1.0f,0.0f,1.0f);
    glm::vec3 pos = m_position + m_offset;
    glm::vec3 target = m_target + m_offset;

    return glm::lookAt(pos, target, up);
}

glm::mat4 OrbitCamera::getProjMatrix(int width, int height)
{
	return glm::perspective(m_fov, (float)width / (float)height, m_near, m_far);
}

void OrbitCamera::focusOn(const glm::vec3& min, const glm::vec3& max)
{
    m_focusMin = min;
    m_focusMax = max;
    m_offset = glm::vec3(0.0f,0.0f,0.0f);
    m_rotX = -MATH_PI / 4.0f;
    m_rotY = MATH_PI / 4.0f;

    m_target = min + ((max - min) * 0.5f);

    m_distance = _calculateRequiredDistance();
}

void OrbitCamera::_rotate(const glm::vec2& mouseMovement)
{
    m_rotX -= mouseMovement.y * 0.003f;
    m_rotY -= mouseMovement.x * 0.003f;
}

glm::vec3 OrbitCamera::_getCameraSide()
{
    glm::vec3 dir = m_target - m_position;
    glm::vec3 side = glm::cross(dir, glm::vec3(0.0f,1.0f,0.0f));
    return glm::normalize(side);
}

glm::vec3 OrbitCamera::_getCameraUp()
{
    glm::vec3 dir = m_target - m_position;
    glm::vec3 v = _getCameraSide();
    v = glm::cross(dir, v);
    return glm::normalize(v);
}

void OrbitCamera::_pan(const glm::vec2& mouseMovement)
{
    glm::vec3 side = _getCameraSide();
    glm::vec3 up = _getCameraUp();

    side = side * -mouseMovement[0] * 0.007f * (m_distance / 5.0f);
    up = up * -mouseMovement[1] * 0.007f * (m_distance / 5.0f);

    m_offset += side;
    m_offset += up;
}    

float OrbitCamera::_calculateRequiredDistance() 
{
    glm::vec3 min = m_focusMin;
    glm::vec3 max = m_focusMax;
    glm::vec3 center = min + ((max - min) * 0.5f);
    float r = volpe::max(glm::distance(center,min), glm::distance(center,max));

    return (r * 2.0f) / tan(m_fov / 1.5f);
}

glm::vec3 OrbitCamera::getViewDirection() const
{
    return glm::normalize(m_target - m_position);
}

glm::vec3 OrbitCamera::getViewPosition() const
{
    return m_position;
}