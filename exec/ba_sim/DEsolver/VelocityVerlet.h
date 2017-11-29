/*
 * mpUtils
 * VelocityVerlet.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the VelocityVerlet class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_VELOCITYVERLET_H
#define MPUTILS_VELOCITYVERLET_H

// includes
//--------------------
#include "DEsolver.h"
//--------------------

//-------------------------------------------------------------------
/**
 * class VelocityVerlet
 *
 * usage:
 * Perform VelocityVerlet integration.
 * See DEsolver for usage information.
 * Remember to call start() before using advanceTime for the first time.
 * VelocityVerlet integration uses an aditional buffer bound to the VERLET_BUFFER_BINDING. This
 * is handled automatically by this class.
 *
 */
class VelocityVerlet : public DEsolver
{
public:
    VelocityVerlet();
    VelocityVerlet(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt);
    virtual ~VelocityVerlet() override = default;
    void setAccelerator(std::function<void(void)> accelerator) override;
    void setDT(double dt) override;
    void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number) override;

    void start() override;
    void advanceTime() override;

private:
    mpu::gph::ShaderProgram m_shader;
    std::function<void(void)> m_calcAcceleration;

    uint32_t m_numParticles;
    uint32_t m_wgSize;
    double m_dt;

    mpu::gph::Buffer verletBuffer;
};


#endif //MPUTILS_VELOCITYVERLET_H
