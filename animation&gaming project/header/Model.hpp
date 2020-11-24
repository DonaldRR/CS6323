//
//  Model.hpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/6/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#ifndef Model_hpp
#define Model_hpp

#include "Scheduler.hpp"
#include "Object.hpp"

class Scheduler;

//------------------------------------------------- Model -------------------------------------------------------

class Model
{
protected:
    int _obj_index;
    std::string _model_name;
    int _model_id;
    glm::vec3 _model_color;
    glm::mat4 _model_mat;
    
    Scheduler* _scheduler = nullptr;
public:
    Model(int obj_index, int model_id, std::string model_name, glm::vec3 model_color, glm::mat4 model_mat): _obj_index{obj_index}, _model_id{model_id}, _model_name{model_name}, _model_color{model_color}, _model_mat{model_mat}{};
    ~Model(){};
    
    // setter
    void set_scheduler(Scheduler* scheduler){_scheduler = scheduler;};
    // getter
    int get_obj_index(){return _obj_index;};
    std::string get_model_name(){return _model_name;};
    glm::vec3 get_model_color(){return _model_color;};
    glm::mat4 get_model_mat(){return _model_mat;};
    Object* get_object();
};

//------------------------------------------------- Target -------------------------------------------------------

class Target : public Model{
private:
    std::vector<glm::vec2> _keypoints;
    std::vector<glm::vec2> _extended_keypoints;
    
    void _interpolate_keypoints();
    
public:
    Target(int obj_index, int model_id, std::string model_name, glm::vec3 model_color, glm::mat4 model_mat, std::vector<glm::vec2> keypoints): Model(obj_index, model_id, model_name, model_color, model_mat), _keypoints{keypoints}
    {
        _interpolate_keypoints();
    };

    bool hit_target(glm::vec2 pos);
    std::string state2string();
    
    std::vector<glm::vec2> get_keypoints(){ return _extended_keypoints;};
};

//------------------------------------------------- Player -------------------------------------------------------

class Player : public Model{
private:
    glm::vec2 _position;
    glm::vec2 _speed;
    glm::vec2 _acc;
    int _time_stamp;
    float _mass;
    
public:
    Player(int obj_index, int model_id, std::string model_name, glm::vec3 model_color, glm::mat4 model_mat, glm::vec2 position, float mass): Model(obj_index, model_id, model_name, model_color, model_mat), _position{position}, _mass{mass}
    {
        _speed[0] = 0.0f;
        _speed[1] = 0.0f;
//        if(model_name == "prey")
//        {
//            _speed[1] = 0.1f;
//        }
//        else{
//            _speed[1] = -0.1f;
//        }
        
        _acc = {0.0f,0.0f};
        
        _time_stamp = 0;
    };

    void update_speed();
    void update_acc(glm::vec2 acc);
    void update_position();
    glm::vec2 collide(glm::vec2 pos);
    glm::vec2 compute_action();
    std::string state2string();
    
    void set_speed(glm::vec2 speed){_speed = speed;};
    int get_model_id(){return _model_id;};
    glm::vec2 get_position(){return _position;};
    glm::vec2 get_speed(){return _speed;};
    float get_mass(){return _mass;};
};


#endif /* Model_hpp */
