/*
 * mpUtils
 * Buffer.h
 *
 * Contains the Buffer class to manage an openGL Buffer
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

#include <GL/glew.h>
#include <vector>
#include <iterator>
#include "Handle.h"

namespace mpu {
namespace gph {

typedef Handle<uint32_t, decltype(&glCreateBuffers), &glCreateBuffers, decltype(&glDeleteBuffers), &glDeleteBuffers> BufferHandle;

/**
 * class BufferMap
 *
 * usage:
 * A buffer map will be returned from the buffers map() function and can be used to manipulate the buffer data
 *
 */
template <typename T>
class BufferMap
{
public:
    // constructors
    BufferMap()= default;
    BufferMap(const BufferMap &other)=default;
    BufferMap(BufferMap &&other)=default;

    BufferMap(const BufferHandle handle, T* datapointer, const std::size_t size) : m_handle(handle),
                                                                                         m_data(std::shared_ptr<T>(datapointer,[handle](T* t){glUnmapNamedBuffer(handle);})),
                                                                                         m_size(size)
    {}

    // array access operators
    T& operator[](std::size_t idx) { return *(m_data.get()+idx); }
    const T& operator[](std::size_t idx) const {return *(m_data.get()+idx);}

    // helper functions
    size_t size() const { return m_size;}
    bool empty() const { return (m_size > 0);}

    // iterators
    typedef T* iterator;
    typedef const T* const_iterator;

    iterator begin() { return m_data.get();}
    iterator end() { return m_data.get() + size();}
    const_iterator cbegin() const { return m_data.get();}
    const_iterator cend() const { return m_data.get() + size();}

    void flush(GLintptr offset, GLsizeiptr length) {glFlushMappedBufferRange(m_handle,offset,length);} //!< flush changes on a subregion of a buffer mapped with "GL_MAP_FLUSH_EXPLICIT_BIT"
    void flush() {glFlushMappedBufferRange(m_handle,0,m_size * sizeof(T));} //!< flush changes on the entire mapped region of a buffer mapped with "GL_MAP_FLUSH_EXPLICIT_BIT"

private:
    std::shared_ptr<T> m_data{nullptr};
    const BufferHandle m_handle{nullptr};
    const std::size_t m_size{0};
};

/**
 * class Buffer
 *
 * Class to manage an openGl buffer.
 * The class only holds a reference to the buffer, thus copying a Buffer object will result in two references to the same opengl Buffer.
 *
 * usage:
 * Use clone to deep-copy the opengl buffer.
 * The buffer can be allocated as an inmutable buffer right from the constructor or from one of the allocate functions.
 * Call stream() to use after construction to use the buffer as a mutable buffer.
 * Use bindBase() to bind the buffer to a binding point and index.
 * Functions write(), read() allow you to patially or fully read and write buffer date. Use copyTo() to copy data from one Buffer to another.
 *
 * The size() function will return the buffer size.
 *
 */
    class Buffer : public BufferHandle
    {
    public:
        constexpr static ptrdiff_t whole_size = -1;

        Buffer() = default; //!< creates an empty buffer without allocating it
        explicit Buffer(nullptr_t) : Handle(nullptr) {} //!< creates no buffer
        template<typename T>
        explicit Buffer( std::vector<T> data, GLbitfield flags = 0); //!< creates a buffer as a inmutable buffer and allocates it with data from ste std::vector
        template<typename T>
        explicit Buffer( T data, GLbitfield flags = 0); //!< creates a buffer as a inmutable buffer and allocates it with one field of type T CAUTION: do not use for arrays or vectors

        /**
         * @brief copys a part of this buffers data to another buffer
         * @tparam T data types to be cpied, can be omitted to copy byte by byte
         * @param target the Target Buffer object all data is copied to
         * @param count number of values of type T to be copyed
         * @param src_offset offset in the source buffer
         * @param dst_offset offset in the destination buffer
         */
        template<typename T = uint8_t>
        void copyTo(const Buffer& target, ptrdiff_t count, ptrdiff_t src_offset = 0, ptrdiff_t dst_offset = 0) const;
        void copyTo(const Buffer& target, ptrdiff_t dst_offset = 0) const; //!< same as above, but copy entire buffer byte by byte

        /**
         * @brief performes a deep copy on a part of the buffer creating a new buffer object
         * @tparam T type of elements to be copied
         * @param count number of elements to be copied
         * @param src_offset offset in the source buffer
         * @param dst_offset offset in the destination buffer
         * @return the newly created Buffer object
         */
        template<typename T = uint8_t>
        Buffer clone(ptrdiff_t count, ptrdiff_t src_offset = 0, ptrdiff_t dst_offset = 0) const;
        Buffer clone(ptrdiff_t dst_offset = 0) const; //!< same as above, but copy entire buffer byte by byte

        /**
         * see: glBindBufferBase
         * Binds the buffer to the given fixed binding index. And Target Useful when having global buffers which are always bound
         * to the same binding to all shaders can access them the same way.
         * Refers to GLSL via:
         * layout(binding = N) <buffertype>...
         */
        void bindBase( uint32_t binding, GLenum target) const;

        /**
         * @brief allocate in inmutable mode using glBufferStorage and upload the data from an std::vector to it
         * @tparam T type of values inside the std::array
         * @param data the std::vector of data to be uploaded
         * @param flags flags passed to glBufferStorage
         *                                      GL_DYNAMIC_STORAGE_BIT  allows the use of the write() function <br>
         *                                      GL_MAP_READ_BIT         allows mapping for read access <br>
         *                                      GL_MAP_WRITE_BIT        allows mapping for write access <br>
         *                                      GL_MAP_PERSISTENT_BIT   allows mapping in persistent mode which allows to maintain a mapping while buffer is used in openGL <br>
         *                                      GL_MAP_COHERENT_BIT     allows mapping in coherent mode where changes to a persistent buffer are always directly visible <br>
         *                                      GL_CLIENT_STORAGE_BIT   a hint that the buffer shoulb be created in client memory <br>
         */
        template<typename T>
        void allocate( std::vector<T> data, GLbitfield flags = 0) const;

        /**
         * @brief allocate in inmutable mode using glBufferStorage to count number of empty fields of type T
         * @tparam T type of values that will be put in the buffer
         * @param count the number of fields (size of the buffer)
         * @param flags flags passed to glBufferStorage <br>
         *                                      GL_DYNAMIC_STORAGE_BIT  allows the use of the write() function <br>
         *                                      GL_MAP_READ_BIT         allows mapping for read access <br>
         *                                      GL_MAP_WRITE_BIT        allows mapping for write access <br>
         *                                      GL_MAP_PERSISTENT_BIT   allows mapping in persistent mode which allows to maintain a mapping while buffer is used in openGL <br>
         *                                      GL_MAP_COHERENT_BIT     allows mapping in coherent mode where changes to a persistent buffer are always directly visible <br>
         *                                      GL_CLIENT_STORAGE_BIT   a hint that the buffer shoulb be created in client memory <br>
         */
        template<typename T>
        void allocate( intptr_t count, GLbitfield flags = 0) const;

        /**
         * @brief allocate in inmutable mode using glBufferStorage and upload a single element of type T
         *          CAUTION: do not use for arrays, vectors and so on
         * @tparam T type of values that will be put in the buffer
         * @param count the number of fields (size of the buffer)
         * @param flags flags passed to glBufferStorage <br>
         *                                      GL_DYNAMIC_STORAGE_BIT  allows the use of the write() function <br>
         *                                      GL_MAP_READ_BIT         allows mapping for read access <br>
         *                                      GL_MAP_WRITE_BIT        allows mapping for write access <br>
         *                                      GL_MAP_PERSISTENT_BIT   allows mapping in persistent mode which allows to maintain a mapping while buffer is used in openGL <br>
         *                                      GL_MAP_COHERENT_BIT     allows mapping in coherent mode where changes to a persistent buffer are always directly visible <br>
         *                                      GL_CLIENT_STORAGE_BIT   a hint that the buffer shoulb be created in client memory <br>
         */
        template<typename T>
        void allocate(T data, GLbitfield flags = 0) const;

        /**
         * @brief assign data to the buffer via glBufferSubData only availible when the flag GL_DYNAMIC_STORAGE_BIT was set during allocation
         * @tparam T type of data to be uploaded
         * @param data a vector containing all the data to upload to the buffer
         * @param offset offset of data in the buffer
         */
        template<typename T>
        void write( std::vector<T> data, intptr_t offset = 0) const;

        /**
         * @brief reads data from the buffer to the location pointed by data
         * @param size number of bytes to download
         * @param data where the bytes shoud be written
         * @param offset offset of data in the buffer
         */
        template <typename T>
        std::vector<T> read( GLsizei size,  intptr_t offset = 0) const;

        /**
         * @brief reads one element from the buffer
         * @param offset the position in the buffer of the element to be read
         */
        template <typename T>
        T read( intptr_t offset = 0) const;

        /**
         * @brief use the buffer in mutable mode and stram data to it using glBufferData. Do not use on a buffer which was alreade made unmutable with allocate
         * @tparam T the type of data to be copied into the buffer
         * @param data the date as an std vector
         * @param mode the Buffer mode a combination of GL_ and one of <br>
         *          STATIC_     optimized for writing once <br>
         *          DYNAMIC_    optimized for writing sometimes <br>
         *          STREAM_     optimized for writing every frame <br>
         *          and one of <br>
         *          DRAW        user will write data but never read it <br>
         *          READ        user wiil read data but never write it <br>
         *          COPY        user will not read or write any date (buffer for use in shaders only) <br>
         */
        template<typename T>
        void stream(std::vector<T> data, GLenum mode = GL_STREAM_DRAW) const;

        /**
         * @brief Map the buffer to local memory and retrieve a BufferMap object
         * @tparam T the data type you want to use to access the buffer data
         * @param count number of fields of type T to load from the buffer
         * @param offset offset in the the buffer where mapping should start
         * @param access access bit a combination of    GL_MAP_READ_BIT             map for read acces <br>
         *                                              GL_MAP_WRITE_BIT            map for write acces <br>
         *                                              GL_PERSISTENT_BIT           allow mapping while buffer is used in openGL <br>
         *                                              GL_COHERENT_BIT             on a persistent buffer, changes are always directly visible <br>
         *                                              GL_MAP_FLUSH_EXPLICIT_BIT   you need to flush everything by yourself instead of automatically on unmap <br>
         *                                              GL_MAP_INVALIDATE_RANGE_BIT   invalidate the mapped range, might be faster  if you write everything <br>
         *                                              GL_MAP_INVALIDATE_BUFFER_BIT  invalidate the complete buffer, might be faster if you write everything <br>
         *                                              GL_MAP_UNSYNCHRONIZED_BIT   openGL will not perform any syncronization bevor ahnding over the pointer to the mapped date <br>
         * @return a BufferMap object which can be used to manipulate the buffer data. when the BufferMap goes out of scope it will unmap the buffer
         */
        template<typename T = uint8_t>
        BufferMap<T> map(ptrdiff_t count, ptrdiff_t offset, GLbitfield access) const;

        // The buffer size in bytes.
        int64_t size() const;

        /**
         * @brief invalidates a subsection of the buffers date
         * @param offset? offset to the beginning of the subsection
         * @param length length of the subsection
         */
        void invalidate(GLintptr offset, GLsizeiptr length);
        void invalidate(); //!< invalidate the whle buffers data

        /**
         * @brief Generate a adress for the Buffer to access them in the shader without binding.
         * @tparam T
         * @param offset_bytes  TODO: make that doc better
         * @param access
         * @return
         */
        template<typename T = uint8_t>
        uint64_t address( ptrdiff_t offset_bytes = 0, GLenum access = GL_READ_ONLY) const;
    };

    template <typename T>
    Buffer::Buffer(const std::vector<T> data, const GLbitfield flags) : Buffer()
    {
        allocate<T>(data,flags);
    }

    template <typename T>
    Buffer::Buffer(const T data, const GLbitfield flags) : Buffer()
    {
        allocate<T>(data, flags);
    }

    inline void Buffer::bindBase(const uint32_t binding, const GLenum target) const
    {
        glBindBufferBase(target, binding, *this);
    }

    template <typename T>
    void Buffer::copyTo(const Buffer& target, const ptrdiff_t count, const ptrdiff_t src_offset, const ptrdiff_t dst_offset) const
    {
        glCopyNamedBufferSubData(*this, target, src_offset*sizeof(T), dst_offset*sizeof(T), sizeof(T) * count);
    }

    inline void Buffer::copyTo(const Buffer& target, const ptrdiff_t dst_offset) const
    {
        glCopyNamedBufferSubData(*this, target, 0, dst_offset, size());
    }

    template <typename T>
    Buffer Buffer::clone(const ptrdiff_t count, const ptrdiff_t src_offset, const ptrdiff_t dst_offset) const
    {
        Buffer newBuffer;
        copyTo(newBuffer, count * sizeof(T), src_offset*sizeof(T), dst_offset*sizeof(T));
        return newBuffer;
    }

    inline Buffer Buffer::clone(const ptrdiff_t dst_offset) const
    {
        return clone(size(), 0, dst_offset);
    }

    template<typename T>
    void Buffer::allocate(const std::vector<T> data, const GLbitfield flags) const
    {
        glNamedBufferStorage(*this, data.size()*sizeof(T), data.data(), flags);
    }

    template<typename T>
    void Buffer::allocate(const intptr_t count, const GLbitfield flags) const
    {
        glNamedBufferStorage(*this, count * sizeof(T), nullptr, flags);
    }

    template <typename T>
    void Buffer::allocate(const T data, const GLbitfield flags) const
    {
        glNamedBufferStorage(*this, sizeof(T), &data, flags);
    }

    template<typename T>
    void Buffer::write(const std::vector<T> data, const intptr_t offset) const
    {
        glNamedBufferSubData(*this, offset * sizeof(T), data.size()*sizeof(T), data.data());
    }

    template <typename T>
    std::vector<T> Buffer::read(const GLsizei size, const intptr_t offset) const
    {
        std::vector<T> v(size);
        glGetNamedBufferSubData(*this,offset*sizeof(T), size* sizeof(T),v.data());
        return v;
    }

    template <typename T>
    T Buffer::read(const intptr_t offset) const
    {
        T t;
        glGetNamedBufferSubData(*this,offset* sizeof(T),sizeof(T),&t);
        return t;
    }

    template<typename T>
    void Buffer::stream(const std::vector<T> data, const GLenum mode) const
    {
        glNamedBufferData(*this, data.size()*sizeof(T), data.data(), mode);
    }

    template <typename T>
    BufferMap<T> Buffer::map(const ptrdiff_t count, const ptrdiff_t offset, const GLbitfield access) const
    {
        static_assert(!std::is_same_v<T, void>, "Cannot use void* as buffer mapping return type.");
        return BufferMap<T>(*this, reinterpret_cast<T*>(glMapNamedBufferRange(*this, offset * sizeof(T), count * sizeof(T), access)), count);
    }

    template <typename T>
    uint64_t Buffer::address(const ptrdiff_t offset_bytes, const GLenum access) const
    {
        uint64_t addr;
        glMakeNamedBufferResidentNV(*this, access);
        glGetNamedBufferParameterui64vNV(*this, GL_BUFFER_GPU_ADDRESS_NV, &addr);
        glMakeNamedBufferNonResidentNV(*this);

//        assert_true(offset_bytes * sizeof(T) < static_cast<uint64_t>(size()), "Offset out of bounds.");

        return addr + offset_bytes * sizeof(T);
    }

    inline int64_t Buffer::size() const
    {
        int64_t num;
        glGetNamedBufferParameteri64v(*this, GL_BUFFER_SIZE, &num);
        return num;
    }

    inline void Buffer::invalidate(GLintptr offset, GLsizeiptr length)
    {
        glInvalidateBufferSubData(*this,offset,length);
    }

    inline void Buffer::invalidate()
    {
        glInvalidateBufferData(*this);
    }

}}