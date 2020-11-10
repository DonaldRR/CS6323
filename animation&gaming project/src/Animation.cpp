#include "Animation.hpp"

Animation::Animation()
{
    this->m_model_mat = glm::mat4(1.0f);
}

Animation::~Animation()
{
}

void Animation::init()
{
    reset();
}

void Animation::update(float delta_time)
{
}

void Animation::reset()
{
    x_rotation = 0;
    y_rotation = 0;
    z_rotation = 0;
    m_model_mat = glm::mat4(1.0f);
    m_model_mat = glm::translate(m_model_mat, glm::vec3(5.0f, 0.0f, 0.0f));
}
