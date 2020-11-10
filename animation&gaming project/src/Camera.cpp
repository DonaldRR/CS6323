////
////  Camera.cpp
////  animation&gaming project
////
////  Created by Donald Zhou on 10/5/20.
////  Copyright © 2020 Donald Zhou. All rights reserved.
////
//
//#include "Camera.hpp"
//
//void Camera::reset()
//{
//    this->position = ori_position;
//    this->front = ori_front;
//    this->up = ori_up;
//    this->right = ori_right;
//    this->zoom = ori_zoom;
//}
//
//void Camera::init()
//{
//    reset();
//}
//
//void Camera::process_keyboard(Camera_Movement direction, GLfloat delta_time)
//{
//    GLfloat move_velocity = delta_time * 10;
//    GLfloat rotate_velocity = delta_time * 50;
//    
//    if(direction == FORWARD) this->position += this->front * move_velocity;
//    if(direction == BACKWARD) this->position -= this->front * move_velocity;
//    if(direction == LEFT) this->position -= this->right * move_velocity;
//    if(direction == RIGHT) this->position += this->right * move_velocity;
//    if(direction == UP) this->position += this->up * move_velocity;
//    if(direction == DOWN) this->position -= this->up * move_velocity;
//    if(direction == ROTATE_X_UP) rotate_x(rotate_velocity);
//    if(direction == ROTATE_X_DOWN) rotate_x(-rotate_velocity);
//    if(direction == ROTATE_Y_UP) rotate_y(rotate_velocity);
//    if(direction == ROTATE_Y_DOWN) rotate_y(-rotate_velocity);
//    if(direction == ROTATE_Z_UP) rotate_z(rotate_velocity);
//    if(direction == ROTATE_Z_DOWN) rotate_z(-rotate_velocity);
//}
//
//void Camera::rotate_x(GLfloat angle)
//{
//    glm::vec3 up = this->up;
//    glm::mat4 rotation_mat(1.0);
//    rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), this->right);
//    this->up = glm::normalize(glm::vec3(rotation_mat * glm::vec4(up, 1.0)));
//    this->front = glm::normalize(glm::cross(this->up, this->right));
//}
//
//void Camera::rotate_y(GLfloat angle)
//{
//    glm::vec3 front = this->front;
//    glm::mat4 rotation_mat(1);
//    rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), this->up);
//    this->front = glm::normalize(glm::vec3(rotation_mat * glm::vec4(front, 1.0)));
//    this->right = glm::normalize(glm::cross(this->front, this->up));
//}
//
//void Camera::rotate_z(GLfloat angle)
//{
//    glm::vec3 right = this->right;
//    glm::mat4 rotation_mat(1);
//    rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), this->front);
//    this->right = glm::normalize(glm::vec3(rotation_mat * glm::vec4(right, 1.0)));
//    this->up = glm::normalize(glm::cross(this->right, this->front));
//}
//
//glm::mat4 Camera::get_view_mat()
//{
//    this->view_mat = glm::lookAt(this->position, this->position + this->front, this->up);
//    return this->view_mat;
//}
//
//glm::mat4 Camera::get_projection_mat()
//{
//    this->proj_mat = glm::perspective(this->zoom, (GLfloat)this->width / (GLfloat)this->height, this->near, this->far);
//    return this->proj_mat;
//}
