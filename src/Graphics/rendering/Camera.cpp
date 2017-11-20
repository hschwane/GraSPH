/*
 * mpUtils
 * camera.cpp
 *
 * Contains a camera and camera controller class to control the rendering view
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the camera class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "Camera.h"
#include <GL/glew.h>
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

// function definitions of the camera class
//-------------------------------------------------------------------
Camera::Camera() : m_controller( nullptr)
{
}

Camera::Camera(Window* window)
        : m_controller(std::static_pointer_cast<CameraController>(std::make_shared<DefaultCameraController>(window)))
{
}

Camera::Camera(std::shared_ptr<CameraController> controller) : m_controller(controller)
{
}

void Camera::update(double dt)
{
    m_controller->updateTransform(m_transform, dt);

    if(m_mvp)
    {
        m_mvp->setView(viewMatrix());
    }
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::inverse(static_cast<glm::mat4>(m_transform));
}

glm::mat4 Camera::projectionMatrix() const
{
    return glm::perspective(m_fov, m_aspect, m_clip_near, m_clip_far);
}

void Camera::setFOV(const float fovDegrees)
{
    m_fov = glm::radians(fovDegrees);
    if(m_mvp)
        m_mvp->setProjection(projectionMatrix());
}

void Camera::setClip(const float near, const float far)
{
    m_clip_far = far;
    m_clip_near = near;
    if(m_mvp)
        m_mvp->setProjection(projectionMatrix());
}

void Camera::setCNear(const float near)
{
    m_clip_near = near;
    if(m_mvp)
        m_mvp->setProjection(projectionMatrix());
}

void Camera::setCFar(const float far)
{
    m_clip_far = far;
    if(m_mvp)
        m_mvp->setProjection(projectionMatrix());
}

void Camera::setMVP(ModelViewProjection *mvp)
{

    m_mvp = mvp;
    if(m_mvp)
    {
        m_mvp->setViewProjection(viewMatrix(), projectionMatrix());
    }
}

void Camera::setAspect(const float aspect)
{
    m_aspect = aspect;
    if(m_mvp)
        m_mvp->setProjection(projectionMatrix());
}

SimlpleWASDController::SimlpleWASDController( Window* window, float rotation_speed, float movement_speed)
        : m_rotation_speed(rotation_speed), m_movement_speed(movement_speed), m_window(*window)
{
}

void SimlpleWASDController::updateTransform(Transform &transform, double dt)
{

    auto mode = m_window.getInputMode(GLFW_CURSOR);

    // switch input mode on right click
    switch(m_window.getMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        case GLFW_PRESS:
            if (!m_mode_changed)
            {
                m_last_cursor_position = m_window.getCursorPos();
                m_window.setInputMode(GLFW_CURSOR,mode != GLFW_CURSOR_DISABLED ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                m_mode_changed = true;
            }
            break;

        default:
        case GLFW_RELEASE:
            m_mode_changed = false;
            break;
    }

    // update position
    const auto two_key_check = [this](int key_fwd, int key_bwd) {
        return static_cast<float>(static_cast<int>(m_window.getKey(key_fwd) == GLFW_PRESS) - static_cast<int>(m_window.getKey(key_bwd) == GLFW_PRESS));
    };
    transform.position += glm::rotate(transform.rotation, glm::vec3{
            two_key_check(GLFW_KEY_D, GLFW_KEY_A),
            two_key_check(GLFW_KEY_E, GLFW_KEY_Q),
            two_key_check(GLFW_KEY_S, GLFW_KEY_W)
    }) * m_movement_speed * static_cast<float>(dt);

    // update rotation
    if(mode == GLFW_CURSOR_DISABLED)
    {
        glm::dvec2 current_position = m_window.getCursorPos();
        const glm::dvec2 delta = m_last_cursor_position - current_position;
        m_last_cursor_position = current_position;

        transform.rotation = glm::quat(glm::vec3(glm::radians(delta.y * m_rotation_speed * dt), 0.f, 0.f))
                             * glm::quat(glm::vec3(0.f, glm::radians(delta.x * m_rotation_speed * dt), 0.f))
                             * transform.rotation;
    }


}

}}