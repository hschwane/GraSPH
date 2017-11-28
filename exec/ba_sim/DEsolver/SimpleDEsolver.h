/*
 * mpUtils
 * ExplicitEuler.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the SimpleDEsolver class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_EXPLICITEULER_H
#define MPUTILS_EXPLICITEULER_H

// includes
//--------------------
#include "DEsolver.h"
#include "../Common.h"
//--------------------

//-------------------------------------------------------------------
/**
 * class SimpleDEsolver
 *
 * usage:
 * Template for simple differential equation solvers that only need one
 * shader to be called after the acceleration update.
 *
 */
class SimpleDEsolver : public DEsolver
{
public:
    SimpleDEsolver(std::string shaderPath);
    SimpleDEsolver(std::string shaderPath, std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt);
    virtual ~SimpleDEsolver() override = default;
    void setAccelerator(std::function<void(void)> accelerator) override;
    void setDT(double dt) override;
    void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number) override;

    void advanceTime() override;

private:
    mpu::gph::ShaderProgram m_shader;
    std::function<void(void)> m_calcAcceleration;

    uint32_t m_numParticles;
    uint32_t m_wgSize;
};

//-------------------------------------------------------------------
/**
 * class ExplicitEuler
 *
 * usage:
 * Perform Explicit Euler integration on the particles. See DEsolver for usage info.
 *
 */
class ExplicitEuler : public SimpleDEsolver
{
public:
    ExplicitEuler() : SimpleDEsolver(PROJECT_SHADER_PATH"DEsolver/explicit-euler.comp"){}
    ExplicitEuler(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt) : SimpleDEsolver(PROJECT_SHADER_PATH"DEsolver/explicit-euler.comp",accelerator,particleBuffer,number,dt) {}
    virtual ~ExplicitEuler() override = default;
};

//-------------------------------------------------------------------
/**
 * class SemiImplicitEuler
 *
 * usage:
 * Perform Semi-Implcit (symplectic) Euler integration on the particles. See DEsolver for usage info.
 *
 */
class SemiImplicitEuler : public SimpleDEsolver
{
public:
    SemiImplicitEuler() : SimpleDEsolver(PROJECT_SHADER_PATH"DEsolver/semi-implicit-euler.comp"){}
    SemiImplicitEuler(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number, double dt) : SimpleDEsolver(PROJECT_SHADER_PATH"DEsolver/semi-implicit-euler.comp",accelerator,particleBuffer,number,dt) {}
    virtual ~SemiImplicitEuler() override = default;
};

#endif //MPUTILS_EXPLICITEULER_H
