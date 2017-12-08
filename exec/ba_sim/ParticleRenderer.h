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

enum class Falloff
{
    NONE,
    LINEAR,
    SQUARED,
    CUBED,
    ROOT
};

//-------------------------------------------------------------------
/**
 * class ParticleRenderer
 * A class for flexible rendering of particles.
 *
 * usage:
 * Use setParticleBuffer() to set the buffer the particles are in.
 * Then use configureArrays to set the Position of the vec4 Position and the float renderSize within the particle struct.
 * Set desired Model, View, and Projection matrices and the current viewport size then
 * call draw to draw.
 * Remember to use glMemoryBarrier(..) with GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT if the data was changed beforehand in an incoherent way.
 *
 * You can use setColor and setBrightness to change the visuals.
 * setShaderSettings allows you to make particles round or squared, and decide if the will scale with perspective or not.
 * You can also set a style for the falloff, meaning different functions to calculate how moch darker the particles borders will be.
 * If you enable additive blending particle colors are added onto each other.
 *
 * Do NOT disable GL_PROGRAM_POINT_SIZE !!
 * If additive blending is on, do not disable GL_BLEND or change the blending function.
 * Remember to clear the depth buffer if depth testing is enabled!
 */
class ParticleRenderer : public mpu::gph::ModelViewProjection
{
public:
    ParticleRenderer();

    void draw();

    void setParticleBuffer(ParticleBuffer buffer); //!< set the particle buffer to be used in rendering

    void setViewportSize(glm::uvec2 viewport); //!< update the viewport size

    void setShaderSettings(Falloff style, bool perspectiveSize=true, bool roundParticles=true); //!< specify if particles further away should look smaller and if they are drawn round or squared and there falloff style

    void enableAdditiveBlending(bool enable); //!< add particle colors onto each other for blending
    void enableDepthTest(bool enable); //!< enable or disable depth testing
    void setSize(float size); //!< set the size the particles should be rendered with
    void setColor(glm::vec4 c); //!< set the color of the paricles
    void setBrightness(float b); //!< set the brightness of the particles (usefull with additive blending)

private:
    mpu::gph::ShaderProgram m_renderShader;
    mpu::gph::VertexArray m_vao;
    uint32_t m_numOfParticles{0};
    glm::vec2 m_vpSize{0};
    glm::vec4 m_color{1,1,1,1};
    float m_brightness{1};
    float m_size{0.05};
};

#endif //MPUTILS_PARTICLERENDERER_H
