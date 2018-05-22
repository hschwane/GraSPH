/*
 * mpUtils
 * transform.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the transform class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "Transform.h"
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

// function definitions of the transform class
//-------------------------------------------------------------------
Transform::Transform(const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale)
        : position(position), scale(scale), rotation(rotation)
{

}

bool Transform::operator==(const Transform &other) const
{
    return position == other.position &&
           scale == other.scale &&
           rotation == other.rotation;
}

bool Transform::operator!=(const Transform &other) const
{
    return position != other.position ||
           scale != other.scale ||
           rotation != other.rotation;
}

Transform::Transform(const glm::mat4 mat)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, position, skew, perspective);
    rotation = glm::conjugate(rotation);
}

Transform::operator glm::mat4() const
{
    return glm::translate(glm::mat4(1.f), position) * glm::scale(glm::mat4(1.f), scale) * glm::toMat4(rotation);
}

}}