/*
 * mpUtils
 * Leapfrog.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Leapfrog class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_LEAPFROG_H
#define MPUTILS_LEAPFROG_H

// includes
//--------------------
#include "DEsolver.h"
//--------------------

//-------------------------------------------------------------------

/**
 * class SemiImplicitEuler
 *
 * usage:
 * Perform leapfrog integration. Veloctiy will always be v(t-1/2dt) while Position is p(t).
 * See DEsolver for usage information.
 * Remember to call start() before using advanceTime for the first time.
 * If you need v(t) you can use catchUpTime() you must however call start() again after this.
 *
 */
class Leapfrog : public DEsolver
{
public:
    Leapfrog();
    Leapfrog(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt);
    virtual ~Leapfrog() override = default;
    void setAccelerator(std::function<void(void)> accelerator) override;
    void setDT(double dt) override;
    void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number) override;

    void start() override;
    void advanceTime() override;

    void catchUpTime(); //!< integrate time one half step so time is at v(t)

private:
    mpu::gph::ShaderProgram m_shader;
    std::function<void(void)> m_calcAcceleration;

    uint32_t m_numParticles;
    uint32_t m_wgSize;
    double m_dt;
};


#endif //MPUTILS_LEAPFROG_H
