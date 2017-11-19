/*
 * mpUtils
 * Buffer.h
 *
 * Contains the VertexArrray class to manage an openGL VertexArrayObject
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 * This file was originally written and generously provided for this framework from Johannes Braun.
 *
 */
#pragma once

#include <cinttypes>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Handle.h"
#include "Buffer.h"

namespace mpu {
namespace gph {

class VertexArray : public Handle<uint32_t, decltype(&glCreateVertexArrays), &glCreateVertexArrays, decltype(&glDeleteVertexArrays), &glDeleteVertexArrays>
{
public:
	VertexArray() = default; //!< default constructor
	explicit VertexArray(nullptr_t) : Handle(nullptr){} //!< constructor that does not call glGenBuffer

    void enableArray(GLuint attribIndex); //!< enable the attibute array "attribIndex"
    void disableArray(GLuint attribIndex); //!< enable the attibute array "attribIndex"

    /**
     * @brief Assign an openGL Buffer to a index on the vao
     * @param index the index where the buffer is added
     * @param buffer the buffer to add to the vao
     * @param offset the offset in bytes from the beginning of the buffer
     * @param stride byte offset from one vertex to the next (size of the vertex in bytes)
     */
    void setBuffer(GLuint bindingIndex, Buffer buffer, GLintptr offset, GLsizei stride);

    /**
     * @brief Set the format of an attribute that uses float or vec inside the shader.
     * @param attribIndex index of the attribute
     * @param vecSize size of the vector per vertex (1-4)
     * @param relativeOffset the relative offset of the vector inside the buffer. Use offset_of(&MyVertex::my_member)
     * @param type the type of the input data, GL_FLOAT mostly. Integer are converted into floating point
     * @param normalize enable normalization for integer to float conversion
     */
    void setAttribFormat(GLuint attribIndex, GLint vecSize, GLuint relativeOffset, GLenum type=GL_FLOAT, GLboolean normalize = GL_FALSE);
    void setAttribFormatInt(GLuint attribIndex, GLint vecSize, GLuint relativeOffset, GLenum type=GL_INT); //!< same as above, but for actual integers
    void setAttribFormatDouble(GLuint attribIndex, GLint vecSize, GLuint relativeOffset); //!< same as above, but for double

    void addBinding( GLuint attribIndex, GLuint bindingIndex); //!< configure a buffer to be the data source for an attribute array

    /**
     * @brief Enable a atrribute index, configure it and set the buffer at bindingIndex as the data source.
     *          It only works for floating point attributes. If you need double or int call the seperate functions on your own
     * @param attribIndex index of the attribute
     * @param bindingIndex binding index of the data source buffer
     * @param vecSize size of the vector per vertex (1-4)
     * @param relativeOffset the relative offset of the vector inside the buffer. Use offset_of(&MyVertex::my_member)
     * @param type the type of the input data, GL_FLOAT mostly. Integer are converted into floating point
     * @param normalize enable normalization for integer to float conversion
     */
    void addAttributeArray(GLuint attribIndex, GLuint bindingIndex, GLint vecSize, GLuint relativeOffset, GLenum type=GL_FLOAT, GLboolean normalize = GL_FALSE);

    /**
     * @brief Enable a atrribute index and configure it. Bind "buffer" at bindingIndex and set it as the data source.
     *          It only works for floating point attributes. If you need double or int call the seperate functions on your own
     * @param attribIndex index of the attribute
     * @param bindingIndex binding index of the data source buffer
     * @param buffer the buffer object to bind on the binding index
     * @param offset the offset in bytes from the beginning of the buffer
     * @param stride byte offset from one vertex to the next (size of the vertex in bytes)
     * @param vecSize size of the vector per vertex (1-4)
     * @param relativeOffset the relative offset of the vector inside the buffer. Use offset_of(&MyVertex::my_member)
     * @param type the type of the input data, GL_FLOAT mostly. Integer are converted into floating point
     * @param normalize enable normalization for integer to float conversion
     */
    void addAttributeBufferArray(GLuint attribIndex, GLuint bindingIndex, Buffer buffer, GLintptr offset, GLsizei stride, GLint vecSize, GLuint relativeOffset, GLenum type=GL_FLOAT, GLboolean normalize = GL_FALSE);
    void addAttributeBufferArray(GLuint attribIndex, Buffer buffer, GLintptr offset, GLsizei stride, GLint vecSize, GLuint relativeOffset, GLenum type=GL_FLOAT, GLboolean normalize = GL_FALSE); //!< like above, but use attribute index also as the binding index


    void setIndexBuffer( uint32_t buffer) const; //!< set a buffer as index buffer for the vao

	void bind() const; //!< bind the vao to use it as the rendering source
};

void VertexArray::enableArray(const GLuint attribIndex)
{
    glEnableVertexArrayAttrib(*this, attribIndex);
}

void VertexArray::disableArray(GLuint attribIndex)
{
    glDisableVertexArrayAttrib(*this, attribIndex);
}

void VertexArray::setBuffer(const GLuint bindingIndex, const Buffer buffer, const GLintptr offset, const GLsizei stride)
{
    glVertexArrayVertexBuffer(*this, bindingIndex, buffer, offset, stride);
}

void VertexArray::setAttribFormat(const GLuint attribIndex, const GLint vecSize, const GLuint relativeOffset, const GLenum type, const GLboolean normalize)
{
    glVertexArrayAttribFormat(*this, attribIndex, vecSize, type, normalize, relativeOffset);
}

void VertexArray::setAttribFormatInt(GLuint attribIndex, GLint vecSize, GLuint relativeOffset, GLenum type)
{
    glVertexArrayAttribIFormat(*this, attribIndex, vecSize, type, relativeOffset);
}

void VertexArray::setAttribFormatDouble(GLuint attribIndex, GLint vecSize, GLuint relativeOffset)
{
    glVertexArrayAttribLFormat(*this, attribIndex, vecSize, GL_DOUBLE, relativeOffset);
}

void VertexArray::addBinding(GLuint attribIndex, GLuint bindingIndex)
{
    glVertexArrayAttribBinding(*this, attribIndex, bindingIndex);
}

void VertexArray::addAttributeArray(GLuint attribIndex, GLuint bindingIndex, GLint vecSize, GLuint relativeOffset,
                                    GLenum type, GLboolean normalize)
{
    enableArray(attribIndex);
    setAttribFormat(attribIndex, vecSize, relativeOffset, type, normalize);
    addBinding(attribIndex, bindingIndex);
}

void VertexArray::addAttributeBufferArray(GLuint attribIndex, GLuint bindingIndex, Buffer buffer, GLintptr offset,
                                          GLsizei stride, GLint vecSize, GLuint relativeOffset, GLenum type,
                                          GLboolean normalize)
{
    enableArray(attribIndex);
    setBuffer(bindingIndex, buffer, offset, stride);
    setAttribFormat(attribIndex, vecSize, relativeOffset, type, normalize);
    addBinding(attribIndex,bindingIndex);
}

void VertexArray::addAttributeBufferArray(GLuint attribIndex, Buffer buffer, GLintptr offset, GLsizei stride,
                                          GLint vecSize, GLuint relativeOffset, GLenum type, GLboolean normalize)
{
    addAttributeBufferArray(attribIndex,attribIndex,buffer,offset,stride,vecSize,relativeOffset,type,normalize);
}

void VertexArray::setIndexBuffer(const uint32_t buffer) const
{
	glVertexArrayElementBuffer(*this, buffer);
}

void VertexArray::bind() const
{
	glBindVertexArray(*this);
}


}}