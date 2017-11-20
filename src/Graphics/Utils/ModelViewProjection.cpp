/*
 * mpUtils
 * ModelViewProjection.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ModelViewProjection class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "ModelViewProjection.h"
#include <utility>
//--------------------

// namespace
//--------------------
namespace mpu {
namespace gph {
//--------------------

// function definitions of the ModelViewProjection class
//-------------------------------------------------------------------
void ModelViewProjection::setModel(glm::mat4 model)
{
    m_model = std::move(model);
    m_model_view = m_view * m_model;
    m_model_view_projection = m_projection * m_view * m_model;
    m_model_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model))));
    m_model_view_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model_view))));
}

void ModelViewProjection::setView(glm::mat4 view)
{
    m_view = std::move(view);
    m_model_view = m_view * m_model;
    m_model_view_projection = m_projection * m_view * m_model;
    m_view_projection = m_projection * m_view;
    m_model_view_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model_view))));
}

void ModelViewProjection::setProjection(glm::mat4 projection)
{
    m_projection = std::move(projection);
    m_model_view_projection = m_projection * m_view * m_model;
    m_view_projection = m_projection * m_view;
}

void ModelViewProjection::setModelView(glm::mat4 model, glm::mat4 view)
{
    m_model = std::move(model);
    m_view = std::move(view);
    m_model_view = m_view * m_model;
    m_model_view_projection = m_projection * m_view * m_model;
    m_view_projection = m_projection * m_view;
    m_model_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model))));
    m_model_view_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model_view))));
}

void ModelViewProjection::setViewProjection(glm::mat4 view, glm::mat4 projection)
{
    m_view = std::move(view);
    m_projection = std::move(projection);
    m_model_view = m_view * m_model;
    m_model_view_projection = m_projection * m_view * m_model;
    m_view_projection = m_projection * m_view;
    m_model_view_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model_view))));
}

void ModelViewProjection::setModelViewProjection(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
    m_model = std::move(model);
    m_view = std::move(view);
    m_projection = std::move(projection);
    m_model_view = m_view * m_model;
    m_model_view_projection = m_projection * m_view * m_model;
    m_view_projection = m_projection * m_view;
    m_model_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model))));
    m_model_view_normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(m_model_view))));
}

}}
