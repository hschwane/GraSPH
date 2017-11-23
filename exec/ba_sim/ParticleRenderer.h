/*
 * mpUtils
 * ParticleRenderer.h
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

#ifndef MPUTILS_PARTICLERENDERER_H
#define MPUTILS_PARTICLERENDERER_H

// includes
//--------------------
#include <Graphics/Graphics.h>
#include "Common.h"
//--------------------

//-------------------------------------------------------------------
/**
 * class ParticleRenderer
 * A class for flexible rendering of particles.
 *
 * usage:
 * Use setParticleBuffer() to set the buffer the particles are in.
 * Then use configureArrays to set the Position of the vec4 Position and the float renderSize within the particle struct.
 * Set desired Model, View, and Projection matrices and then
 * call draw to draw.
 * Remember to use glMemoryBarrier(..) with GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT if the data was changed beforehand in an incoherent way.
 *
 */
class ParticleRenderer : mpu::gph::ModelViewProjection
{
public:
    ParticleRenderer();

    void draw();

    void configureArrays(GLuint positionOffset, GLuint rendersizeOffset);

    template <typename particle_type>
    void setParticleBuffer(mpu::gph::Buffer buffer, uint32_t numOfParticles);

private:
    mpu::gph::ShaderProgram m_renderShader;
    mpu::gph::VertexArray m_vao;
    uint32_t m_numOfParticles{0};
};

template<typename particle_type>
void ParticleRenderer::setParticleBuffer(mpu::gph::Buffer buffer, uint32_t numOfParticles)
{
    m_vao.setBuffer(RENDERER_BUFFER_BINDING,buffer,0,sizeof(particle_type));
    m_numOfParticles = numOfParticles;
    logINFO("Renderer") << "Set Buffer for renderer.";
    logDEBUG("Renderer") << "Buffer containing " << m_numOfParticles << " particles of size " << sizeof(particle_type);
}


#endif //MPUTILS_PARTICLERENDERER_H
