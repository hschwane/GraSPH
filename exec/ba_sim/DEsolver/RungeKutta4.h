/*
 * mpUtils
 * RungeKutta4.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the RungeKutta4 class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_RUNGEKUTTA4_H
#define MPUTILS_RUNGEKUTTA4_H

// includes
//--------------------
#include "DEsolver.h"
//--------------------

//-------------------------------------------------------------------

/**
 * class RungeKutta4
 *
 * usage:
 * Perform Runge-Kutta order 4 integration on the particles. See DEsolver for usage info.
 * Runge-Kutta-4 uses 3 additional buffers which are bound to the different RK_*_BINDINGs.
 * This is handled automatically.
 */
class RungeKutta4 : public DEsolver
{
public:
    RungeKutta4();
    RungeKutta4(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt);
    virtual ~RungeKutta4() override = default;
    void setAccelerator(std::function<void(void)> accelerator) override;
    void setDT(double dt) override;
    void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number) override;

    void advanceTime() override;

private:
    mpu::gph::ShaderProgram m_rk4Shader;
    mpu::gph::ShaderProgram m_rkIntermediateShader;
    std::function<void(void)> m_calcAcceleration;

    uint32_t m_numParticles;
    uint32_t m_wgSize;
    double m_dt;

    mpu::gph::Buffer m_particleBuffer;
    mpu::gph::Buffer m_rkM2Buffer;
    mpu::gph::Buffer m_rkM3Buffer;
    mpu::gph::Buffer m_rkM4Buffer;
};


#endif //MPUTILS_RUNGEKUTTA4_H
