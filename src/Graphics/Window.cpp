/*
 * mpUtils
 * Window.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Window class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include <Log/Log.h>
#include "Window.h"
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

// function definition for the glDebug callback
//-------------------------------------------------------------------
/**
 * @brief callback function used internally to write openGL errors to the log
 */
static void glDebugCallback(GLenum source, GLenum type, GLuint id, const GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
    const auto format_message = [&] {
        return "source=\"" + [&]() -> std::string {
            switch (source)
            {
                case GL_DEBUG_SOURCE_API: return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
                case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
                case GL_DEBUG_SOURCE_APPLICATION: return "Application";
                default: return "Other";
            }
        }() + "\", type=\"" + [&]() -> std::string {
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR: return "Error";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behavior";
                case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
                case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
                case GL_DEBUG_TYPE_MARKER: return "Marker";
                case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
                case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
                default: return "Other";
            }
        }() + "\" -- " + message;
    };

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            logERROR("OpenGL") << format_message();
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            logWARNING("OpenGL") << format_message();
            break;
        case GL_DEBUG_SEVERITY_LOW:
            logINFO("OpenGL") << format_message();
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            logDEBUG("OpenGL") << format_message();
            break;
        default:
            break;
    }
}

// function definitions of the Window class
//-------------------------------------------------------------------
Window::Window(const int width, const int height, const std::string &title, GLFWmonitor *monitor, GLFWwindow *share) : m_w(nullptr,[](GLFWwindow* wnd){})
{
    // init glfw once
    static struct GLFWinit
    {
        GLFWinit() {
            int e =glfwInit();
            if(e  != GL_TRUE)
            {
                logFATAL_ERROR("Graphics") << "Error initalising glfw. Returned: " << e ;
                throw std::runtime_error("Could not initalize glfw!");
            }

            glfwSetErrorCallback([](int code, const char * message){
                logERROR("GLFW") << "Error code: " << code << "Message: " << message;
            });
            logINFO("Graphics") << "Initialised GLFW. Version: " << glfwGetVersionString();
        }
        ~GLFWinit() { glfwTerminate(); }
    } glfwinit;

    // setting some important default settings
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* wnd = glfwCreateWindow(width, height, title.data(), monitor, share);
    if(!wnd)
    {
        logERROR("Window") << "Cannot create window.";
        throw std::runtime_error("Cannot create window");
    }
    m_w = std::unique_ptr<GLFWwindow,void(*)(GLFWwindow*)>(wnd,[](GLFWwindow* wnd){glfwDestroyWindow(wnd);});
    glfwSetWindowUserPointer(m_w.get(),this);
    makeContextCurrent();

    // init glew
    static struct GLEWinit
    {
        GLEWinit() {
            glewExperimental = GL_TRUE;
            GLenum e = glewInit();
            if(e != GLEW_OK)
            {
                logFATAL_ERROR("Graphics") << "Error initalising glew. Returned: " << e ;
                throw std::runtime_error("Could not initalize glew!");
            }
            logINFO("Graphics") << "Initialised GLEW."
                                << "\n\t\t\t\tOpenGL version: " << glGetString(GL_VERSION)
                                << "\n\t\t\t\tGLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
                                << "\n\t\t\t\tVendor: " << glGetString(GL_VENDOR)
                                << "\n\t\t\t\tRenderer: " << glGetString(GL_RENDERER)
                                << "\n\t\t\t";
        }
    } glewinit;

    glDebugMessageCallback(&glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, false);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, false);
}

Window::operator GLFWwindow*() const
{
    return m_w.get();
}

GLFWwindow* Window::window() const
{
    return m_w.get();
}

int Window::gl_major = 4;
int Window::gl_minor = 5;
void Window::setGlVersion(int major, int minor)
{
    gl_major = major;
    gl_minor = minor;
}

bool Window::update()
{
    glfwSwapBuffers(m_w.get());
    glfwPollEvents();
    return !glfwWindowShouldClose(m_w.get());
}

}}
