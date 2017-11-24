/*
 * mpUtils
 * ParticleRenderer.cpp
 *
 * A class for flexible rendering of particles.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ParticleRenderer class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "ParticleRenderer.h"
//--------------------

// function definitions of the ParticleRenderer class
//-------------------------------------------------------------------
ParticleRenderer::ParticleRenderer()
        : m_renderShader({{PROJECT_SHADER_PATH"ParticleRenderer/particleRenderer.vert"},
                          {PROJECT_SHADER_PATH"ParticleRenderer/particleRenderer.frag"}})
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_vao.enableArray(RENDERER_POSITION_ARRAY);
    m_vao.enableArray(RENDERER_SIZE_ARRAY);
}

void ParticleRenderer::draw()
{
    assert_true(m_viewportSize.y!=0,"Renderer","Do not render without setting the viewport size!")
    m_vao.bind();
    m_renderShader.use();
    m_renderShader.uniformMat4("model_view_projection", getModelViewProjection());
    m_renderShader.uniformMat4("projection", getProj());
    m_renderShader.uniform2f("viewport_size", m_viewportSize);
    glDrawArrays(GL_POINTS, 0, m_numOfParticles);
}

void ParticleRenderer::configureArrays(GLuint positionOffset, GLuint rendersizeOffset)
{
    m_vao.setAttribFormat(RENDERER_POSITION_ARRAY, 4, positionOffset);
    m_vao.addBinding(RENDERER_POSITION_ARRAY, RENDERER_BUFFER_BINDING);

    m_vao.setAttribFormat(RENDERER_SIZE_ARRAY, 1, rendersizeOffset);
    m_vao.addBinding(RENDERER_SIZE_ARRAY, RENDERER_BUFFER_BINDING);

    logINFO("Renderer") << "Reconfigured vertex arrays.";
    logDEBUG("Renderer") << "PositionOffset: " << positionOffset << " renderSize Offset: " << rendersizeOffset;
}
