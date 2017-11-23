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
ParticleRenderer::ParticleRenderer() : m_renderShader()
{
    m_vao.enableArray(RENDERER_POSITION_BINDING);
    m_vao.enableArray(RENDERER_SIZE_BINDING);
}

void ParticleRenderer::draw()
{
    m_vao.bind();
    m_renderShader.use();
    m_renderShader.uniformMat4("model_view_projection", getModelViewProjection());
    glDrawArrays(GL_POINTS, 0, m_numOfParticles);
}

void ParticleRenderer::configureArrays(GLuint positionOffset, GLuint rendersizeOffset)
{
    m_vao.setAttribFormat(RENDERER_POSITION_BINDING, 4, positionOffset);
    m_vao.addBinding(RENDERER_POSITION_BINDING, RENDERER_BUFFER_BINDING);

    m_vao.setAttribFormat(RENDERER_SIZE_BINDING, 1, rendersizeOffset);
    m_vao.addBinding(RENDERER_SIZE_BINDING, RENDERER_BUFFER_BINDING);

    logINFO("Renderer") << "Reconfigured vertex arrays.";
    logDEBUG("Renderer") << "PositionOffset: " << positionOffset << " renderSize Offset: " << rendersizeOffset;
}
