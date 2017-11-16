/*
 * mpUtils
 * ModelViewProjection.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ModelViewProjection class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_MODELVIEWPROJECTION_H
#define MPUTILS_MODELVIEWPROJECTION_H

// includes
//--------------------
#include <glm/glm.hpp>
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

//-------------------------------------------------------------------
/**
 * class ModelViewProjection
 *
 * usage:
 * This class represents a full model view projection. All intermediate matrices will be updated whenever one of them is changed by the client.
 * It was recommended by Johannes to store this whole thing in a uniform buffer object.
 *
 */
class ModelViewProjection
{
public:
    ModelViewProjection() = default;

    void setModel(glm::mat4 model);
    void setView(glm::mat4 view);
    void setProjection(glm::mat4 projection);
    void setModelView(glm::mat4 model, glm::mat4 view);
    void setViewProjection(glm::mat4 view, glm::mat4 projection);
    void setModelViewProjection(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

    glm::mat4 getModel() { return m_model;}
    glm::mat4 getView() { return m_view;}
    glm::mat4 getProj() { return m_projection;}
    glm::mat4 getModelNormal() {return m_model_normal;}
    glm::mat4 getModelView() {return m_model_view;}
    glm::mat4 getModelViewNormal() {return m_model_view_normal;}
    glm::mat4 getViewProjektion() { return m_view_projection;}
    glm::mat4 getModelViewProjektion() {return m_model_view_projection;}

private:
    glm::mat4 m_model{ 1.f };
    glm::mat4 m_view{ 1.f };
    glm::mat4 m_projection{ 1.f };

    glm::mat4 m_model_normal{ 1.f};
    glm::mat4 m_model_view{ 1.f };
    glm::mat4 m_model_view_normal{ 1.f };
    glm::mat4 m_view_projection{ 1.f };
    glm::mat4 m_model_view_projection{ 1.f };
};

}}
#endif //MPUTILS_MODELVIEWPROJECTION_H
