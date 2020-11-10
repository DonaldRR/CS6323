//
//  scheduler.cpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/14/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#include "Scheduler.hpp"

//------------------------------------------------- Comm -------------------------------------------------------

void Comm::init(){
    
    _serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(1238);
    bind(_serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    listen(_serv_sock, 20);
    
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    std::cout << "[Listening ...]" << std::endl;
    _clnt_sock = accept(_serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    std::cout << "[Connected]" << std::endl;

};

void Comm::send(std::string s)
{
    clear_buffer();
    
    memcpy(&_buffer, s.c_str(), s.size());

    ::send(_clnt_sock, _buffer, 1024, 0);
//    std::cout << "[Sent] " << s << std::endl;
}


void Comm::receive()
{
    clear_buffer();
    clear_message();

    while (true) {
        if (recv(_clnt_sock, _buffer, sizeof(_buffer), 0)) {
            break;
        }
    }
    
    std::string s_buffer = std::string(_buffer);
//    std::cout << "[Received] " << s_buffer << std::endl;
    
    int idx_delim = s_buffer.find("\n");
    std::string msg_type = s_buffer.substr(0, idx_delim);
    
    s_buffer.erase(0, idx_delim + 2);
//    std::cout << "trimmed string:" << s_buffer << std::endl;
    
    if (msg_type == "start") {
        _msg.type = START;
    }
    else if (msg_type == "action") {
        _msg.type = ACTION;
        
        int idx_item = s_buffer.find(";");
        while (idx_item >= 0) {
            
            glm::vec2 action{0.0f, 0.0f};
            // read ax
            float idx_n = s_buffer.find(",");
            action[0] = std::stof(s_buffer.substr(0, idx_n));
            s_buffer.erase(0, idx_n + 1);
            // read ay
            idx_n = s_buffer.find(",");
            action[1] = std::stof(s_buffer.substr(0, idx_n));
            s_buffer.erase(0, idx_n + 1);
            // read v
            action = glm::normalize(action);
            idx_n = s_buffer.find(";");
            action *= std::stof(s_buffer.substr(0, idx_n));
            s_buffer.erase(0, idx_n + 1);
            
            idx_item = s_buffer.find(";");
//            std::cout << "action:" << glm::to_string(action) << std::endl;
            _msg.vecs.push_back(action);
        }
    }
}

Message Comm::get_message()
{
    return _msg;
}

void Comm::clear_message()
{
    _msg.type = NONE;
    _msg.vecs.clear();
}

void Comm::clear_buffer()
{
    memset(&_buffer, 0, sizeof(_buffer));
}

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

void Player::update_speed(glm::vec2 acc)
{
    // Update object position with its speed calculated by current acceleration
    _speed = _speed + _acc / float(TIME_SLICES);
    float v = glm::length(_speed);
    if(v > 0)
    {
        v = std::min(v, 0.1f);
        _speed = glm::normalize(_speed) * v;
    }

    // Modify the acceleration
    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == 0) {
        // Add force and update acceleration
//        float force_angle = rand() % 360 / 180.0 * M_PI;
//        _acc = {cos(force_angle), sin(force_angle)};
        _acc = acc * float(0.05);
    }
    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == FORCE_APPLY_PERIOD) {
        // Clear the force (acceleration)
        _acc = {0.0f, 0.0f};
    }
    _time_stamp += 1;

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
    
    return ret;
}


glm::vec2 Player::collide(glm::vec2 pos)
{
    return _scheduler->in_board(pos);
}

//------------------------------------------------- Scheduler -------------------------------------------------------

void Scheduler::build_environment()
{
    std::vector<glm::mat4> scale_mats;
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(_board_x, _board_y, 1.0)));
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(1.0, _board_y, 1.0)));
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(_board_x, 1.0, 1.0)));
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(1.0, _board_y, 1.0)));
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(_board_x, 1.0, 1.0)));
    scale_mats.push_back(glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 2.0, 1.0)));
    
    std::vector<glm::mat4> rotate_mats;
    rotate_mats.push_back(glm::mat4(1.0f));
    rotate_mats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    rotate_mats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
    rotate_mats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    rotate_mats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
    rotate_mats.push_back(glm::mat4(1.0f));

    std::vector<glm::mat4> translate_mats;
    translate_mats.push_back(glm::mat4(1.0f));
    translate_mats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(_board_y / 2, 0.0f, 0.0f)));
    translate_mats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, _board_x / 2, 0.0f)));
    translate_mats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-_board_y / 2, 0.0f, 0.0f)));
    translate_mats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -_board_x / 2, 0.0f)));
    translate_mats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.01f)));
    
    std::vector<std::string> model_names = {"ground", "wall1", "wall2", "wall3", "wall4", "target"};
    glm::vec3 default_color(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < model_names.size(); i++) {
        glm::vec3 model_color = i == model_names.size() - 1? glm::vec3(0.8f, 0.8f, 0.8f) : default_color;

        if (i == model_names.size() - 1) {
            std::vector<glm::vec2> keypoints = {{-2.0f, -5.0f}, {-2.0f, -3.0f}, {2.0f, -3.0f}, {2.0f, -5.0f}};
            Target* model = new Target(0, i, model_names[i], model_color, translate_mats[i] * rotate_mats[i] * scale_mats[i], keypoints);
            model->set_scheduler(this);
            _models.push_back(model);
            _target = model;
        }
        else
        {
            Model* model = new Model(0, i, model_names[i], model_color, translate_mats[i] * rotate_mats[i] * scale_mats[i]);
            model->set_scheduler(this);
            _models.push_back(model);
        }
    }
    
}

void Scheduler::init_players()
{
    int prey = _n_prey, predator = _n_predator;
    
    float left_bottom[2] = {-_board_x / 2, -_board_y / 2};
    int n_col = _board_x / _ball_size, n_row = _board_y / _ball_size;
    int start_idx = _models.size();
    for (int i = 0; i < prey + predator; i ++ ) {
        
        glm::mat4 scale_mat{1.0f}, translate_mat{1.0f}, rotate_mat{1.0f};
        int j = i;
        if (j >= predator) {
            j = j - predator;
        }
        int row_offset = j / n_col, col_offset = j % n_col + 1;
        if (col_offset % 2 == 1) {
            col_offset = (n_col + 1) / 2 - int(col_offset / 2) - 1;
        } else {
            col_offset = (n_col + 1) / 2 + int(col_offset / 2) - 1;
        }
        
        float extra_offset = 0;
        glm::vec3 model_color{1.0f};
        std::string model_name;
        if (i < predator) {
            model_color =glm::vec3(1.0f, 0.0f, 0.0f);
            model_name = "predator";
        }
        else
        {
            row_offset = n_row - 1 - row_offset;
            model_color =glm::vec3(0.0f, 1.0f, 0.0f);
            model_name = "prey";
//            extra_offset = 0.5;
        }
        
        // Object position
        float target_x = left_bottom[0] + float(col_offset + extra_offset) * _ball_size + _ball_size / 2;
        float target_y = left_bottom[1] + float(row_offset) * _ball_size + _ball_size / 2;
        glm::vec2 position{target_x, target_y};

        // Scaling matrix and translation matrix
        scale_mat = glm::scale(scale_mat, glm::vec3(_ball_size, _ball_size, _ball_size));
        translate_mat = glm::translate(translate_mat, glm::vec3(target_x, target_y, 0.0f));
        
        // Add model
        Player* player = new Player(1, start_idx + i, model_name, model_color, translate_mat * scale_mat, position, pow(_ball_size, 3) * (4.0 / 3.0));
        player->set_scheduler(this);
        _models.push_back(player);
        _players.push_back(player);
    }
}

void Scheduler::update()
{
    Message msg;
    if (_stop) {
        _comm->receive();
//        std::cout << "received1" << std::endl;
        msg = _comm->get_message();
        if (msg.type == START)
            reset();
        _stop = false;
        _comm->send(static_state2string());
//        std::cout << "sent1" << std::endl;
    }
    if (!_stop){
        
        for (int t = 0; t < TIME_SLICES; t ++ ) {
            
            stop_detection();
            
            // send states
            _comm->send(dynamic_state2string());
//            std::cout << "sent2" << std::endl;
            if (_stop) {
//                std::cout << "Game terminated." << std::endl;
                break;
            }

            // receive decisions
            _comm->receive();
            msg = _comm->get_message();
//            std::cout << "received2" << std::endl;
            
            std::vector<glm::vec2> actions = compute_action();

            for (int i = 0; i < msg.vecs.size(); i ++ ) {
//                std::cout << i << ":" << glm::to_string(actions[i]) << "," << glm::to_string(msg.vecs[i]) << std::endl;
                actions[i] = msg.vecs[i];
            }
            
            // Clear colliding ids
            for (int i = 0; i < _players.size(); i ++ )
            {
//                std::cout << "a" << i << ":" << glm::to_string(actions[i]) << std::endl;
                _players[i]->update_speed(actions[i]);
            }

            collision_update();

            for (int i = 0; i < _players.size(); i ++ )
            {
                _players[i]->update_position();
            }
        }
    }
};

void Scheduler::stop_detection()
{
    for (auto player : _players) {
        if (player->get_model_name() == "prey" && _target->hit_target(player->get_position())) {
            _stop = true;
        }
    }
}

glm::vec2 Scheduler::in_board(glm::vec2 pos)
{
    // Return which wall the player hits on if it hits one, otherwise return [1.0, 1.0]
    float x = pos[0], y = pos[1];
    float bottom_left[2] = {-_board_x / 2, -_board_y / 2}, top_right[2] = {_board_x / 2, _board_y / 2};
    glm::vec2 wall_vector{0.0f, 0.0f};

    
    if(x - _ball_size / 2 < bottom_left[0])
    {
        wall_vector[0] = x - _ball_size / 2 - bottom_left[0];
    }
    if(x + _ball_size / 2 > top_right[0])
    {
        wall_vector[0] = x + _ball_size / 2 - top_right[0];
    }
    if(y - _ball_size / 2 < bottom_left[1])
    {
        wall_vector[1] = y - _ball_size / 2 - bottom_left[1];
    }
    if(y + _ball_size / 2 > top_right[1])
    {
        wall_vector[1] = y + _ball_size / 2 - top_right[1];
    }
    
    return wall_vector;
};

void Scheduler::collision_update()
{
    // Collision detection
    for (auto cur_player : _players) {
                
        glm::vec2 updated_speed = cur_player->get_speed();
        glm::vec2 cur_pos = cur_player->get_position();
        glm::vec2 next_pos = cur_pos + updated_speed / float(TIME_SLICES);

        // Weights for linear function that defines the track along the speed
        float A, B, C;
        glm::vec2 norm_speed = glm::normalize(updated_speed);
        A = norm_speed[1];
        B = - norm_speed[0];
        C = - A * next_pos[0] - B * next_pos[1];
        float search_r = (glm::length(updated_speed) / float(TIME_SLICES) + _ball_size * 2) / 2.0f;
        
        float m1 = cur_player->get_mass();
        for (auto player : _players)
        {
            if(cur_player != player)
            {
                glm::vec2 pos = player->get_position();
                                    
                float d1, d2;
                d1 = abs(A * pos[0] + B * pos[1] + C) / sqrt(pow(A, 2.0) + pow(B, 2.0));
                
                if (d1 <= _ball_size)
                {
                    float movement = glm::length(updated_speed) / float(TIME_SLICES);
                    d2 = glm::dot(norm_speed, pos - cur_pos);
                    if(d2 > 0 && (d2 < movement  || (d2 < movement + _ball_size && glm::distance(pos, next_pos) < _ball_size)))
                    {
                    
                        auto speed = player->get_speed();
                        
                        glm::vec2 relative_dir = glm::normalize(pos - cur_pos);
                        float m2 = player->get_mass();
                        float v1 = glm::dot(updated_speed, relative_dir);
                        float v2 = glm::dot(speed, relative_dir);
                        float v1_ = ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
                        float v2_ = ((m2 - m1) * v2 + 2 * m1 * v1) / (m1 + m2);
//                        std::cout << glm::to_string(updated_speed) << " ";
                        updated_speed = updated_speed + relative_dir * (v1_ - v1);
                        player->set_speed(speed + relative_dir * (v2_ - v2));
//                        std::cout << glm::to_string(updated_speed) << " " << glm::to_string(speed) << " " <<  glm::to_string(player->get_speed()) << std::endl;
                    }
                }
            }
        }
        cur_player->set_speed(updated_speed);
    }
}

std::unordered_map<int, int> Scheduler::cluster_players()
{
    std::unordered_map<int, int> clusters;
    for (int i = 0; i < _players.size(); i++) {
        int key_id = _players[i]->get_model_name() == "prey" ? 0 : 1;
        clusters[i] = key_id;
    }
    return clusters;
}

std::vector<glm::vec2> Scheduler::compute_action()
{

    std::unordered_map<int, int> clusters = cluster_players();
    std::vector<glm::vec2> target_keypoints = _target->get_keypoints();
    std::vector<glm::vec2> actions;
    
    for (int i = 0; i < _players.size(); i ++ ) {

        Player* cur_player = _players[i];
        glm::vec2 cur_pos = cur_player->get_position();
        int role = clusters[i];
        
        if (role == 0)
        {
            // Prey decision making
            
            glm::vec2 sum_rival_directions{0.0f, 0.0f};
            for (int j = 0; j < _players.size(); j ++ ) {
                if (i != j && role != clusters[j]) {
                    sum_rival_directions += cur_pos - _players[j]->get_position();
                }
            }
            
            glm::vec2 nearest_target_point;
            float nearest_target_distance = _board_x + _board_y;
            
            for (auto kp : target_keypoints) {
                float d2kp = glm::distance(kp, cur_pos);
                if (d2kp <= nearest_target_distance) {
                    nearest_target_distance = d2kp;
                    nearest_target_point = kp;
                }
            }
            
            float A, B, C_;
            A = (nearest_target_point[0] - cur_pos[0]) / (cur_pos[0] * nearest_target_point[1] - nearest_target_point[0] * cur_pos[1]);
            B = (cur_pos[1] - nearest_target_point[1]) / (cur_pos[0] * nearest_target_point[1] - nearest_target_point[0] * cur_pos[1]);
            glm::vec2 candidate_dir1, candidate_dir2;
            if (A == 0)
            {
                candidate_dir1 = {0.0f, 1.0f};
            }
            else if (B == 0)
            {
                candidate_dir1 = {1.0f, 0.0f};
            }
            else
            {
                candidate_dir1 = {1.0f, B / A * 1.0f};
            }
            candidate_dir2 = - candidate_dir1;
//            std::cout << "prey_pos:" << glm::to_string(cur_pos) << " target:" << glm::to_string(nearest_target_point) << std::endl;
            
            if (glm::distance(cur_pos + candidate_dir1, nearest_target_point) < glm::distance(cur_pos + candidate_dir2, nearest_target_point))
            {
                actions.push_back(glm::normalize(candidate_dir1) + glm::normalize(nearest_target_point - cur_pos));
            }
            else
            {
                actions.push_back(glm::normalize(candidate_dir2) + glm::normalize(nearest_target_point - cur_pos));
            }
        }
        else
        {
            // Predator decision making
            int nearest_prey = 0;
            float nearest_prey_distance = _board_x + _board_y;
            for (int j = 0; j < _players.size(); j ++ ) {
                if (i != j && role != clusters[j]) {
                    float distance = glm::distance(cur_player->get_position(), _players[j]->get_position());
                    if (distance < nearest_prey_distance) {
                        nearest_prey = j;
                    }
                }
            }
//            std::cout << "predator_pos:" << glm::to_string(cur_pos) << " prey_pos:" << glm::to_string(_players[nearest_prey]->get_position() - _players[i]->get_position()) << std::endl;
            
            actions.push_back(glm::normalize(_players[nearest_prey]->get_position() - _players[i]->get_position()));
        }
    }
    
    return actions;
}



std::string Scheduler::static_state2string()
{
    std::string s_states = "";
    
    s_states += "H:" + std::to_string(_board_y) + ";W:" + std::to_string(_board_x) + "\n";
    s_states += _target->state2string() + "\n";
    s_states += "ball_size:" + std::to_string(_ball_size) + ";" + "prey:" + std::to_string(_n_prey) + ";predator:" + std::to_string(_n_predator) + "\n";
    
    return s_states;
}


std::string Scheduler::dynamic_state2string()
{
    std::string s_states = "";
    
    for (int i = 0; i < _players.size(); i ++ ) {
        s_states += _players[i]->state2string() + "\n";
    }
    
    s_states += "terminal:" + std::to_string(int(_stop)) + "\n";
    
    return s_states;
}

void Scheduler::init(int prey, int predator)
{
    _n_prey = prey;
    _n_predator = predator;
}
    
void Scheduler::reset()
{
//    std::cout << "TEST" << std::endl;
    clear_buffer();
    build_environment();
    init_players();
}

void Scheduler::clear_buffer()
{
    for(auto model: _models)
    {
        delete model;
    }
    _models.clear();
    
//    for (auto player: _players)
//    {
//        delete player;
//    }
    _players.clear();
}
