/*
 * gpulic
 * screenFillingTri.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2018 Hendrik Schwanekamp
 *
 */
#ifndef MPUTILS_SCREENFILL_H
#define MPUTILS_SCREENFILL_H

// includes
//--------------------
#include <GL/glew.h>
#include "../Opengl/Shader.h"
#include "../Opengl/VertexArray.h"
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

class ScreenFillingTri
{
public:
    ScreenFillingTri()
    {
        vao.bind(); // make sure some vertex array is bound
    }

    ScreenFillingTri(std::string file)
    {
        vao.bind();
        setScreenFillShader(file);
    }

    ScreenFillingTri(const ScreenFillingTri &other)=default;

    void setScreenFillShader(std::string file)
    {
        screenFillShader.rebuild({ {LIB_SHADER_PATH"screenFillingTri.vert"},
                                                {file} });
    }

    const ShaderProgram& shader()
    {
        return screenFillShader;
    }

    void draw()
    {
        screenFillShader.use();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

private:
    ShaderProgram screenFillShader;
    VertexArray vao;
};

}}
#endif //GPULIC_SCREENFILL_H
