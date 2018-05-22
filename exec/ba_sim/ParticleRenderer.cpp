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
                          {PROJECT_SHADER_PATH"ParticleRenderer/particleRenderer.frag"}},
                         {{"PARTICLES_ROUND"},{"PARTICLES_PERSPECTIVE"},{"PARTICLE_FALLOFF",{"falloffColor=color"}}})
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_vao.enableArray(RENDERER_POSITION_ARRAY);
    m_vao.enableArray(RENDERER_MASS_ARRAY);
    m_renderShader.uniform4f("color", m_color);
    m_renderShader.uniform1f("brightness", m_brightness);
    m_renderShader.uniform1f("render_size", m_size);
}

void ParticleRenderer::draw()
{
    m_vao.bind();
    m_renderShader.use();
    m_renderShader.uniformMat4("model_view_projection", getModelViewProjection());
    m_renderShader.uniformMat4("projection", getProj());
    glDrawArrays(GL_POINTS, 0, m_numOfParticles);
}

void ParticleRenderer::setParticleBuffer(ParticleBuffer buffer)
{
    m_vao.setBuffer(RENDERER_POSITION_BUFFER_BINDING,buffer.positionBuffer,0,sizeof(ParticleBuffer::posType));
    m_vao.setAttribFormat(RENDERER_POSITION_ARRAY, 3, 0);
    m_vao.setAttribFormat(RENDERER_MASS_ARRAY, 1, mpu::gph::offset_of(&glm::vec4::w));
    m_vao.addBinding(RENDERER_POSITION_ARRAY, RENDERER_POSITION_BUFFER_BINDING);
    m_vao.addBinding(RENDERER_MASS_ARRAY, RENDERER_POSITION_BUFFER_BINDING);
    m_numOfParticles = buffer.size();

    logINFO("Renderer") << "Set buffer for rendering and reconfigured vertex arrays. Buffer containing " << m_numOfParticles << " Particles.";
}

void ParticleRenderer::setViewportSize(glm::uvec2 viewport)
{
    m_vpSize = viewport;
    m_renderShader.uniform2f("viewport_size", m_vpSize);
}

void ParticleRenderer::setShaderSettings( Falloff style, bool perspectiveSize, bool roundParticles)
{
    std::vector<mpu::gph::glsl::Definition> definitions;
    if(perspectiveSize)
        definitions.push_back({"PARTICLES_PERSPECTIVE"});
    if(roundParticles)
        definitions.push_back({"PARTICLES_ROUND"});

    switch(style)
    {
        case Falloff::LINEAR:
            definitions.push_back({"PARTICLE_FALLOFF",{"falloffColor=color*(1-distFromCenter)"}});
            break;
        case Falloff::SQUARED:
            definitions.push_back({"PARTICLE_FALLOFF",{"falloffColor=color*(1-distFromCenter*distFromCenter)"}});
            break;
        case Falloff::CUBED:
            definitions.push_back({"PARTICLE_FALLOFF",{"falloffColor=color*(1-distFromCenter*distFromCenter*distFromCenter)"}});
            break;
        case Falloff::ROOT:
            definitions.push_back({"PARTICLE_FALLOFF",{"falloffColor=color*(1-sqrt(distFromCenter))"}});
            break;
        case Falloff::NONE:
        default:
            definitions.push_back({"PARTICLE_FALLOFF",{"falloffColor=color"}});
            break;
    }

    m_renderShader.rebuild({{PROJECT_SHADER_PATH"ParticleRenderer/particleRenderer.vert"},
                            {PROJECT_SHADER_PATH"ParticleRenderer/particleRenderer.frag"}},
                           definitions);

    m_renderShader.uniform2f("viewport_size", m_vpSize);
    m_renderShader.uniform4f("color", m_color);
    m_renderShader.uniform1f("brightness", m_brightness);
    m_renderShader.uniform1f("render_size", m_size);
//    m_renderShader.uniform1f("mass_thres", TOTAL_MASS / NUM_PARTICLES);
}

void ParticleRenderer::enableAdditiveBlending(bool enable)
{
    if(enable)
    {
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_BLEND);
    } else
        glDisable(GL_BLEND);
}

void ParticleRenderer::setColor(glm::vec4 c)
{
    m_color=c;
    m_renderShader.uniform4f("color", m_color);
}

void ParticleRenderer::setBrightness(float b)
{
    m_brightness=b;
    m_renderShader.uniform1f("brightness", m_brightness);
}

void ParticleRenderer::enableDepthTest(bool enable)
{
    if(enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void ParticleRenderer::setSize(float size)
{
    m_size = size;
    m_renderShader.uniform1f("render_size", m_size);
}
