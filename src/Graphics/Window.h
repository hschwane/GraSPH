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
 * The context will automattically be the current one.
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

    operator GLFWwindow*() const; //!< return the inner pointer to the glfw window
    GLFWwindow* window() const; //!< return the inner pointer to the glfw window

    void makeContextCurrent() {glfwMakeContextCurrent(m_w.get());} //!< makes this window the current openGL context
    bool update(); //!< take care of regular window stuff (buffer swap and events). Returns false if the window wants to be closed.

    // window setting functions
    void setTitle(const std::string & s) {glfwSetWindowTitle(m_w.get(),s.c_str());} //!< change the window title
    glm::ivec2 getPosition() {glm::ivec2 p; glfwGetWindowPos(m_w.get(),&p.x,&p.y);return p;} //!< returns the window position
    void setPosition(glm::ivec2 pos) {glfwSetWindowPos(m_w.get(),pos.x,pos.y);} //!< sets a new window position
    void setPosition(int x, int y) {glfwSetWindowPos(m_w.get(),x,y);} //!< sets a new window position
    glm::ivec2 getSize(){glm::ivec2 p; glfwGetWindowSize(m_w.get(),&p.x,&p.y);return p;} //!< returns the current window size
    void setSize(glm::ivec2 size) {glfwSetWindowSize(m_w.get(),size.x,size.y);} //!< resize the window
    void setSize(int x, int y) {glfwSetWindowSize(m_w.get(),x,y);} //!< resize the window
    void minimize(){glfwIconifyWindow(m_w.get());} //!< minimize the window
    void restore(){glfwRestoreWindow(m_w.get());} //!< restore a minimized window
    void hide(){glfwHideWindow(m_w.get());} //!< make the window invisible
    void show(){glfwShowWindow(m_w.get());} //!< make the window visible if it was invisible before
    GLFWmonitor* getMonitor(){return glfwGetWindowMonitor(m_w.get());} //!< returns the monitor the window uses for fullscreen mode

    // window handling callbacks
    GLFWwindowposfun setPositionCallback(GLFWwindowposfun cb) {glfwSetWindowPosCallback(m_w.get(),cb);} //!< callback will be called whenever the position is changed
    GLFWwindowsizefun setSizeCallback(GLFWwindowsizefun cb) {glfwSetWindowSizeCallback(m_w.get(),cb);} //!< callback will be clled whenever the position is changed
    GLFWwindowclosefun setCloseCallback(GLFWwindowclosefun cb) {glfwSetWindowCloseCallback(m_w.get(),cb);} //!< callback will be called whenever the user tries to close the window
    GLFWwindowrefreshfun setRefreshCallback(GLFWwindowrefreshfun cb) {glfwSetWindowRefreshCallback(m_w.get(),cb);} //!< callback will be called whenever the wiindow contend needs to be redrawn
    GLFWwindowfocusfun setFocusCallback(GLFWwindowfocusfun cb) {glfwSetWindowFocusCallback(m_w.get(),cb);} //!< callback will be called when the window gains focus
    GLFWwindowiconifyfun setMinimizeCallback(GLFWwindowiconifyfun cb) {glfwSetWindowIconifyCallback(m_w.get(),cb);} //!< callback will be called when the window is minimized

    // input callbacks
    GLFWkeyfun setKeyCallback(GLFWkeyfun cb) {glfwSetKeyCallback(m_w.get(),cb);} //!< callback will be called when key input is availible
    GLFWcharfun setCharCallback(GLFWcharfun cb) {glfwSetCharCallback(m_w.get(),cb);} //!< callback provides character input
    GLFWcharmodsfun setCharmodsCallback(GLFWcharmodsfun cb) {glfwSetCharModsCallback(m_w.get(),cb);} //!< callback provides charater input with modifier keys
    GLFWmousebuttonfun setMousebuttonCallback(GLFWmousebuttonfun cb) {glfwSetMouseButtonCallback(m_w.get(),cb);} //!< called when a mouse button is pressed
    GLFWcursorposfun setCoursorposCallback(GLFWcursorposfun cb) {glfwSetCursorPosCallback(m_w.get(),cb);} //!< called when the cursor is moved
    GLFWcursorenterfun setCoursorenterCallback(GLFWcursorenterfun cb) {glfwSetCursorEnterCallback(m_w.get(),cb);} //!< called when the cursor enters or leaves the window
    GLFWscrollfun setScrollCallback(GLFWscrollfun cb) {glfwSetScrollCallback(m_w.get(),cb);} //!< called when the scroll wheel is moved
    GLFWdropfun setDropCallbac(GLFWdropfun cb) {glfwSetDropCallback(m_w.get(),cb);} //!< called when someting is drag'n droped onto the window

    // input functions
    void setInputMode(int mode, int value) {glfwSetInputMode(m_w.get(),mode,value);} //!< see glfwSetInputMode for reference
    int getInputMode(int mode){return glfwGetInputMode(m_w.get(),mode);} //!< set glfwSetInputMode for reference
    int getKey(int key) {return glfwGetKey(m_w.get(),key);} //!< state of key returns GLFW_PRESS or GLFW_RELEASE
    int getMouseButton(int button) {return glfwGetMouseButton(m_w.get(),button);} //!< state of mouse button returns GLFW_PRESS or GLFW_RELEASE
    glm::dvec2 getCursorPos() {glm::dvec2 p; glfwGetCursorPos(m_w.get(),&p.x,&p.y);return p;} //!< returns the cursor position within the window
    void setCursorPos(glm::dvec2 p) {glfwSetCursorPos(m_w.get(),p.x,p.y);} //!< sets a new cursor position
    void setCursorPos(double x, double y) {glfwSetCursorPos(m_w.get(),x,y);} //!< sets a new cursor position
    void setCursor(GLFWcursor* c) {glfwSetCursor(m_w.get(),c);} //!< sets a new cursor
    void setClipboard(const std::string & s) {glfwSetClipboardString(m_w.get(),s.c_str());} //!< copy a string to the clipboard
    std::string getClipboard() { return std::string(glfwGetClipboardString(m_w.get()));} //!< get the string from the clipboard

private:
    std::unique_ptr<GLFWwindow,void(*)(GLFWwindow*)> m_w; //!< pointer to the glfw window
    static int gl_major; //!< major openGL version to use when creating the next window
    static int gl_minor; //!< minor openGL version to use when creating the next window
};

}}

#endif //MPUTILS_WINDOW_H
