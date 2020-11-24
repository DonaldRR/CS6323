//
//  Model.cpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/6/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#include "Model.hpp"

//------------------------------------------------- Model -------------------------------------------------------

Object* Model::get_object(){return _scheduler->get_object(_obj_index);};

//------------------------------------------------- Target -------------------------------------------------------

void Target::_interpolate_keypoints()
{
    
    int N = 5;
    for (int i = 0; i < _keypoints.size(); i ++) {
        
        _extended_keypoints.push_back(_keypoints[i]);
        glm::vec2 cur_point = _keypoints[i], next_point = i == _keypoints.size() - 1 ? _keypoints[0] : _keypoints[i + 1];
        glm::vec2 norm_vec = glm::normalize(next_point - cur_point);
        float d = glm::distance(cur_point, next_point);
        
        for (int j = 1; j < N + 1; j ++) {
            glm::vec2 interpolated_point = cur_point + norm_vec * (d * float(j)/(N + 1));
            _extended_keypoints.push_back(interpolated_point);
        }
    }
};

bool Target::hit_target(glm::vec2 pos)
{
    float sign = 0;
    bool hit = true;
    
    glm::vec3 pos_vec3{pos[0], pos[1], 0.0f};
    for (int i = 0; i < _keypoints.size(); i ++ ) {
        
        glm::vec3 cur_pos, next_pos;
        cur_pos = {_keypoints[i][0], _keypoints[i][1], 0.0f};
        if (i == _keypoints.size() - 1) {
            next_pos = {_keypoints[0][0], _keypoints[0][1], 0.0f};
        }
        else{
            next_pos = {_keypoints[i + 1][0], _keypoints[i + 1][1], 0.0f};
        }
        
        glm::vec3 axb = glm::cross(pos_vec3 - cur_pos, next_pos - cur_pos);

        if (i > 0 && sign * axb[2] < 0) {
            hit = false;
        }
    
        sign = axb[2];
    }
    
    return hit;
}

std::string Target::state2string()
{
    std::string ret = "";
    
    for (int i = 0; i < _extended_keypoints.size(); i ++ ) {
        std::string tmp = std::to_string(i) + ":";
        tmp += "(";
        tmp += std::to_string(_extended_keypoints[i][0]);
        tmp += ",";
        tmp += std::to_string(_extended_keypoints[i][1]);
        tmp += ");";
        ret += tmp;
    }
    
    return ret;
}

//------------------------------------------------- Player -------------------------------------------------------

void Player::update_acc(glm::vec2 acc)
{
    _acc = acc * float(0.2);
}

void Player::update_speed()
{
    // Update object position with its speed calculated by current acceleration
    float dece = 0.003f / float(TIME_SLICES);
    _speed = _speed + _acc / float(TIME_SLICES);
    float v = glm::length(_speed);
//    std::cout << v << "->";
    v = std::max(v - dece, 0.0f);
//    std::cout << v << "->";

    v = std::min(v, 0.4f);
    if (glm::length(_speed) > 0) {
        _speed = glm::normalize(_speed) * v;
    }

//    std::cout << v << ":" << std::to_string(_speed[0]) << "," << std::to_string(_speed[1]) << std::endl;

//    // Modify the acceleration
//    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == 0) {
//        // Add force and update acceleration
////        float force_angle = rand() % 360 / 180.0 * M_PI;
////        _acc = {cos(force_angle), sin(force_angle)};
//        _acc = acc * float(0.02);
//    }
//    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == FORCE_APPLY_PERIOD) {
//        // Clear the force (acceleration)
//        _acc = {0.0f, 0.0f};
//    }
//    _time_stamp += 1;

    // Update player speed if it hit the wall
    glm::vec2 wall_modifiers;
    wall_modifiers = collide(_position + _speed / float(TIME_SLICES));
    if (wall_modifiers[0] != 0) {
        int sign = wall_modifiers[0] > 0 ? -1 : 1;
        _speed[0] = sign * abs(_speed[0]);
    }
    if (wall_modifiers[1] != 0) {
        int sign = wall_modifiers[1] > 0 ? -1 : 1;
        _speed[1] = sign * abs(_speed[1]);
    }
}

void Player::update_position()
{
    glm::vec2 movement = _speed / float(TIME_SLICES);

    glm::vec2 wall_modifiers;
    wall_modifiers = collide(_position + movement);
    float coef = 1.0f;
    if (wall_modifiers[0] != 0.0 || wall_modifiers[1] != 0.0) {
        int exceeded_axis = abs(wall_modifiers[0]) > abs(wall_modifiers[1]) ? 0 : 1;
        coef = float(1.0 - wall_modifiers[exceeded_axis] / movement[exceeded_axis]);
    }

    movement = movement * coef;
    _position = _position + movement;

    _model_mat = _model_mat * glm::translate(glm::mat4{1.0f}, glm::vec3{movement[0], movement[1], 0.0f});
//    std::cout << "Position:" << glm::to_string(_speed) <<  glm::to_string(_position) << std::endl;
}

std::string Player::state2string()
{
    std::string ret = "";

    int type = _model_name == "prey" ? 0 : 1;
    ret += "type:" + std::to_string(type) + ";";
    ret += "pos:(" + std::to_string(_position[0]) + "," + std::to_string(_position[1]) + ");";
    ret += "speed:(" + std::to_string(_speed[0]) + "," + std::to_string(_speed[1]) + ");";
//    std::cout << std::to_string(_speed[0]) << "," << std::to_string(_speed[1]) << std::endl;
    return ret;
}


glm::vec2 Player::collide(glm::vec2 pos)
{
    return _scheduler->in_board(pos);
}
