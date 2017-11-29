/*
 * mpUtils
 * Verlet.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Verlet class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_VERLET_H
#define MPUTILS_VERLET_H

// includes
//--------------------
#include "DEsolver.h"
//--------------------

//-------------------------------------------------------------------

/**
 * class Verlet
 *
 * usage:
 * Perform Verlet integration.
 * See DEsolver for usage information.
 * Remember to call start() before using advanceTime for the first time.
 * Verlet integration uses an aditional buffer bound to the VERLET_BUFFER_BINDING. This
 * is handled automatically by this class.
 *
 */
class Verlet : public DEsolver
{
public:
    Verlet();
    Verlet(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt);
    virtual ~Verlet() override = default;
    void setAccelerator(std::function<void(void)> accelerator) override;
    void setDT(double dt) override;
    void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number) override;

    void start() override;
    void advanceTime() override;

private:
    mpu::gph::ShaderProgram m_shader;
    mpu::gph::ShaderProgram m_startShader;
    std::function<void(void)> m_calcAcceleration;

    uint32_t m_numParticles;
    uint32_t m_wgSize;

    mpu::gph::Buffer verletBuffer;
};


#endif //MPUTILS_VERLET_H
