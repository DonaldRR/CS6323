//
//  scheduler.cpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/14/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#include "Scheduler.hpp"




//------------------------------------------------- Scheduler -------------------------------------------------------


Scheduler::Scheduler(){
    _board_x = 10;
    _board_y = 10;
    _ball_size = 1;
    srand(time(NULL));
    
    _comm = new Comm();
};

int Scheduler::enable_action()
{

    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == 0)
        return 1;
    if (_time_stamp % (PLAYER_UPDATE_PERIOD * TIME_SLICES) == FORCE_APPLY_PERIOD)
        return 2;

    return 0;
}

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
    
    std::vector<int> col_idxs;
    for (int i = 0; i < n_col; i ++)
        col_idxs.push_back(i);
    
    std::vector<int> prey_col_idxs, predator_col_idxs;
    for (int i = 0; i < n_col; i ++) {
        prey_col_idxs.push_back(i);
        predator_col_idxs.push_back(i);
    }
    
    std::vector<std::vector<int> > position_idxs;
    for (int i = 0; i < n_col; i ++)
        for (int j = 0; j < n_row; j ++ )
            position_idxs.push_back({i, j});
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(position_idxs.begin(), position_idxs.end(), g);
//    std::shuffle(prey_col_idxs.begin(), prey_col_idxs.end(), g);
//    std::shuffle(predator_col_idxs.begin(), predator_col_idxs.end(), g);
    
    for (int i = 0; i < prey + predator; i ++ ) {
        
        glm::mat4 scale_mat{1.0f}, translate_mat{1.0f}, rotate_mat{1.0f};
        int j = i;
        if (j >= predator) {
            j = j - predator;
        }
        
        int row_offset, col_offset;
//        row_offset = j / n_col;
//        col_offset = j % n_col + 1;
//        if (col_offset % 2 == 1) {
//            col_offset = (n_col + 1) / 2 - int(col_offset / 2) - 1;
//        } else {
//            col_offset = (n_col + 1) / 2 + int(col_offset / 2) - 1;
//        }
        
        float extra_offset = 0;
        glm::vec3 model_color{1.0f};
        std::string model_name;
        if (i < predator) {
            while (true) {
                std::vector<int> idxs = position_idxs.back();
                position_idxs.pop_back();
                if (idxs[1] < 7) {
                    col_offset = idxs[0];
                    row_offset = idxs[1];
                    break;
                }
            }
//            col_offset = predator_col_idxs[j];
//            row_offset += n_row / 2 - 1;
            model_color =glm::vec3(1.0f, 0.0f, 0.0f);
            model_name = "predator";
        }
        else
        {
            while (true) {
                std::vector<int> idxs = position_idxs.back();
                position_idxs.pop_back();
                if (idxs[1] >= 5) {
                    col_offset = idxs[0];
                    row_offset = idxs[1];
                    break;
                }
            }
//            col_offset = prey_col_idxs[j];
//            row_offset = n_row - 1 - row_offset;
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
        
        for (int t = 0; t < TIME_SLICES; t ++ )
        {
            stop_detection();
            
            // send states
            _comm->send(dynamic_state2string());
//            std::cout << "sent2" << std::endl;
            if (_stop) {
//                std::cout << "Game terminated." << std::endl;
                break;
            }

            // change force
            if (enable_action() > 0) {
                std::vector<glm::vec2> actions;
                
                // add force
                if (enable_action() == 1)
                {
                    // receive decisions
                    _comm->receive();
                    msg = _comm->get_message();
                    actions = compute_action();
                    for (int i = 0; i < msg.vecs.size(); i ++ )
                        actions[i] = msg.vecs[i];
                }
                // eliminate force
                else if (enable_action() == 2)
                {
                    for (int i = 0; i < _players.size(); i ++)
                        actions.push_back(glm::vec2{0.0f, 0.0f});
                }
                // update force
                for (int i = 0; i < _players.size(); i ++)
                    _players[i]->update_acc(actions[i]);
            }
            
            // Clear colliding ids
            for (int i = 0; i < _players.size(); i ++ )
                _players[i]->update_speed();

            collision_update();

            for (int i = 0; i < _players.size(); i ++ )
            {
                _players[i]->update_position();
            }
            
            _time_stamp += 1;
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
        float A, B, C, v;
        v = glm::length(updated_speed);
        glm::vec2 norm_speed =  v > 0 ? glm::normalize(updated_speed) : glm::vec2{1.0f, 1.0f};
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
                
                if (v == 0 or (v > 0 and d1 <= _ball_size))
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
//            std::cout << "pos:" << glm::to_string(cur_pos) << "->" << glm::to_string(nearest_target_point) << std::endl;
            actions.push_back(glm::normalize(nearest_target_point - cur_pos) * 0.7f);
            
//            float A, B, C_;
//            A = (nearest_target_point[0] - cur_pos[0]) / (cur_pos[0] * nearest_target_point[1] - nearest_target_point[0] * cur_pos[1]);
//            B = (cur_pos[1] - nearest_target_point[1]) / (cur_pos[0] * nearest_target_point[1] - nearest_target_point[0] * cur_pos[1]);
//            glm::vec2 candidate_dir1, candidate_dir2;
//            if (A == 0)
//            {
//                candidate_dir1 = {0.0f, 1.0f};
//            }
//            else if (B == 0)
//            {
//                candidate_dir1 = {1.0f, 0.0f};
//            }
//            else
//            {
//                candidate_dir1 = {1.0f, B / A * 1.0f};
//            }
//            candidate_dir2 = - candidate_dir1;
////            std::cout << "prey_pos:" << glm::to_string(cur_pos) << " target:" << glm::to_string(nearest_target_point) << std::endl;
//            std::cout << "dir1:" << glm::to_string(candidate_dir1) << " dir2:" << glm::to_string(candidate_dir2) << std::endl;
//
//            if (glm::distance(cur_pos + candidate_dir1, nearest_target_point) < glm::distance(cur_pos + candidate_dir2, nearest_target_point))
//            {
//                actions.push_back(glm::normalize(candidate_dir1) + glm::normalize(nearest_target_point - cur_pos));
//                std::cout << "action:" << glm::to_string(actions.back()) << std::endl;
//            }
//            else
//            {
//                actions.push_back(glm::normalize(candidate_dir2) + glm::normalize(nearest_target_point - cur_pos));
//                std::cout << "action:" << glm::to_string(actions.back()) << std::endl;
//            }
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
    
    s_states += "force:" + std::to_string(enable_action()) + "\n";
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
    _time_stamp = 0;
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
