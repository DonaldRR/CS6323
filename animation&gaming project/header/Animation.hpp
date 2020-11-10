#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Animation
{
public:
    float x_rotation;
    float y_rotation;
    float z_rotation;

    Animation();
    ~Animation();

    void init();
    void update(float delta_time);

    void reset();
    glm::mat4 get_model_mat() { return m_model_mat; };
//    glm::
    
private:
    glm::mat4 m_model_mat;
    glm::mat4 m_view_mat;
};

