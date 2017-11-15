/*
 * mpUtils
 * Window.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Window class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_WINDOW_H
#define MPUTILS_WINDOW_H

// includes
//--------------------
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

//-------------------------------------------------------------------
/**
 * class Window
 *
 * usage:
 * This is an object oriented wrapper for glwfs Window. use the constructer to create the window.
 * After that call Window::makeContextCurrent() and you are good to go.
 * Most glfw Window functions are implemented in the wrapper, however if you need a glfw Window you can use Window::window()
 * or a cast to obtain a pointer to a glfw Window.
 * In the Windows main loop you shold call Window::update() it handles events and swaps the Framebuffer. It will return false
 * if the operating sistem is asking the window to be closed.
 * The window is created with the window hints for openGL Version and core profile. To request a specific version call
 * Window::setGlVersion() before creating a window. If you want to use other special options, please use glfwSetWindowHint()
 * to configure all options to your liking before creating a window.
 *
 */
class Window
{
public:
    static void setGlVersion(int major, int minor); //!< change the opengl version you want (bevor creating a window)

    /**
     * @brief Create a new window. The created window needs still to be made the current context
     * @param width width of the window
     * @param height height of the window
     * @param title title of the window
     * @param monitor set a GLFWmonitor to create a fullscreen window
     * @param share supply a pointer to nother window in order to share one gl kontext
     */
    Window(const int width, const int height, const std::string & title, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

    explicit operator GLFWwindow*() const; //!< return the inner pointer to the glfw window
    GLFWwindow* window() const; //!< return the inner pointer to the glfw window

    void makeContextCurrent() {glfwMakeContextCurrent(m_w.get());} //!< makes this window the current openGL context
    bool update(); //!< take care of regular window stuff (buffer swap and events). Returns false if the window wants to be closed.

    void setTitle(const std::string s) {glfwSetWindowTitle(m_w.get(),s.c_str());} //!< change the window title
    void setIcon(const std::string icon = ""); //!< sets the icon specified by its filename. dont specify a filename to use the default icon
    glm::ivec2 getPosition() {glm::ivec2 p; glfwGetWindowPos(m_w.get(),&p.x,&p.y);} //!< returns the window position
    void setPosition(glm::ivec2 pos) {glfwSetWindowPos(m_w.get(),pos.x,pos.y);} //!< sets a new window position
    glm::ivec2 getSize(){glm::ivec2 p; glfwGetWindowSize(m_w.get(),&p.x,&p.y);} //!< returns the current window size
    void setSize(glm::ivec2 size) {glfwSetWindowSize(m_w.get(),size.x,size.y);} //!< resize the window
    void minimize(){glfwIconifyWindow(m_w.get());} //!< minimize the window
    void restore(){glfwRestoreWindow(m_w.get());} //!< restore a minimized window
    void hide(){glfwHideWindow(m_w.get());} //!< make the window invisible
    void show(){glfwShowWindow(m_w.get());} //!< make the window visible if it was invisible before
    GLFWmonitor* getMonitor(){return glfwGetWindowMonitor(m_w.get());} //!< returns the monitor the window uses for fullscreen mode

private:
    std::unique_ptr<GLFWwindow,void(*)(GLFWwindow*)> m_w; //!< pointer to the glfw window
    static int gl_major; //!< major openGL version to use when creating the next window
    static int gl_minor; //!< minor openGL version to use when creating the next window
};

}}

#endif //MPUTILS_WINDOW_H
