/*
 * mpUtils
 * camera.h
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
 * The camera classes are inspired by the camera implementation in the personal framework by Johannes Braun
 *
 */

#ifndef MPUTILS_CAMERA_H
#define MPUTILS_CAMERA_H

// includes
//--------------------
#include <glm/glm.hpp>
#include <memory>
#include "../Utils/Transform.h"
#include "../Utils/ModelViewProjection.h"
#include "../Window.h"
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

//-------------------------------------------------------------------
/**
 * class camera controller
 * Base class for camera controllers
 *
 * usage:
 * Use this as the base class for your camera controller. Overwrite the updateTransform() so that the cameras transform data is updated correctly.
 *
 */
class CameraController
{
public:
    virtual ~CameraController() = default;
    virtual void updateTransform(Transform& transform, double dt) = 0;
};

//-------------------------------------------------------------------
/**
 * class camera
 *
 * usage:
 * Use one of the constructors to create a camera with a controller of your choice.
 * You can then cahnge settings via the setters.
 * Use update to update the cameras position and rotation based on the camera controller.
 * After that you can get view and projection matrices via the viewMatrix() and projectionMatrix() function.
 * Alternatively specify a ModelViewProjection object that will be updated automatically.
 *
 */
class Camera
{
public:
    Camera(); //!< use no controller
    explicit Camera(Window* window); //!< use Default camera controller (window will be strored in the controller)
    explicit Camera(std::shared_ptr<CameraController> controller); //!< use the provided controller to update the camera

    void setController(std::shared_ptr<CameraController> controller){m_controller = controller;} //!< change the camera controller, the old one will be discarded
    void setFOV(float fovDegrees); //!< set the fov in degrees
    void setClip(float near,float far); //!< set the clipping distances
    void setCNear(float near); //!< set the near clipping distances
    void setCFar(float far); //!< set the clipping distances
    void setMVP(ModelViewProjection* mvp);//!< set a model view projection object to be automatically updated in the update call make sure to reset it if you invalidate the pointer
    void setAspect(float aspect);//!< set the render targets aspect ratio

    void update(double dt); //!< update the camera and if there is a ModelViewProjection, this will also be updated. dt is the delta time

    glm::mat4 viewMatrix() const;   //!< returns the viewMatrix
    glm::mat4 projectionMatrix() const; //!< returns the projection matrix

private:
    Transform m_transform; //!< the transform object describing the cameras position and rotation
    float m_fov = glm::radians(80.f); //!< the vield of view of the camera
    float m_clip_near = 0.1f; //!< near clip plane
    float m_clip_far = 1000.f; //!< far clip plane
    float m_aspect = 1.f; //!< the render targets aspect ratio

    ModelViewProjection* m_mvp = nullptr; //!< pointer to a modelViewProjection object to update during the update call

    std::shared_ptr<CameraController> m_controller; //!< the controller object that is updating the camera
};

/**
 * class SimpleWASDController
 *
 * A simple was flying camera controller.
 * Press right mouse button to enter rotation mode.
 * Move mouse to look around.
 * Use WASD ans QE to move camera around.
 * Use SPACE to enable double speed, LEFT SHIFT for half speed.
 * + and - to adjust speed
 * Do not destroy the window before destroying the controller.
 * There is no rotation along the Z axis. (+Y is up)
 *
 */
class SimpleWASDController : public CameraController
{
public:
    explicit SimpleWASDController( Window* window, float rotation_speed = 15.f, float movement_speed = 10.f);
    void updateTransform(Transform& transform, double dt) override;

private:
    bool m_mode_changed = false; //!< was the input mode changed last frame

    glm::dvec2 m_last_cursor_position{}; //!< cursor position on last update

    float m_rotation_speed;   //!< camera rotation speed
    float m_movement_speed;  //!< camera movement speed

    Window& m_window; //!< the window that is querried for inputs
};

typedef SimpleWASDController DefaultCameraController;


}}
#endif //MPUTILS_CAMERA_H
