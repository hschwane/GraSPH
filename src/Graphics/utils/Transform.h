/*
 * mpUtils
 * transform.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the transform class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_TRANSFORM_H
#define MPUTILS_TRANSFORM_H

// includes
//--------------------
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

//-------------------------------------------------------------------
/**
 * class transform
 *
 * usage:
 * A class which represents a full transformation including translation scale and rotation. It can be casted implicit to glm::mat4
 * and can be explicit constructed out of one.
 *
 */
class Transform
{
public:
    // constructor
    Transform() = default;
    explicit Transform(glm::mat4 mat);
    explicit Transform(glm::vec3 position, glm::quat rotation = glm::angleAxis(0.f, glm::vec3(0)), glm::vec3 scale = {1,1,1});

    // comparision operators
    bool operator==(const Transform &other) const;
    bool operator!=(const Transform &other) const;

    // cast operators
    operator glm::mat4() const;

    // transform something
    glm::vec4 operator * (const glm::vec4 v) const;

    // public members for easy access
    glm::vec3 position{ 0, 0, 0 };
    glm::vec3 scale{ 1, 1, 1 };
    glm::quat rotation = glm::angleAxis(0.f, glm::vec3(0));
};



}}
#endif //MPUTILS_TRANSFORM_H
