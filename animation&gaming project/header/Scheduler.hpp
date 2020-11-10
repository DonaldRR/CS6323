//
//  scheduler.hpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/14/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#ifndef Scheduler_hpp
#define Scheduler_hpp

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>
#include <time.h>
#include <set>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Object.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#endif /* scheduler_hpp */

#define PLAYER_UPDATE_PERIOD 50
#define FORCE_APPLY_PERIOD 10
#define TIME_SLICES 5

class Scheduler;

//------------------------------------------------- Communication -------------------------------------------------------

typedef enum {
    START,
    STATE,
    ACTION,
    NONE,
} MESSAGE_TYPE;

struct Message
{
    MESSAGE_TYPE type;
    std::vector<glm::vec2> vecs;
};

class Comm{
    
private:
    int _serv_sock;
    int _clnt_sock;
    char _buffer[1024];
    Message _msg;
    
public:
    Comm(){
        init();
    };
    
    ~Comm(){
        
        close(_serv_sock);
        close(_clnt_sock);
    };
    
    void init();
    void send(std::string s);
    void receive();
    Message get_message();
    void clear_buffer();
    void clear_message();
};


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
        if(model_name == "prey")
        {
            _speed[1] = 0.1f;
        }
        else{
            _speed[1] = -0.1f;
        }
        
        _acc = {0.0f,0.0f};
        
        _time_stamp = 0;
    };

    void update_speed(glm::vec2 acc);
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

//------------------------------------------------- Scheduler -------------------------------------------------------

class Scheduler {

private:
    std::vector<Object> _objects;
    std::vector<Model*> _models;
    std::vector<Player*> _players;
    Target* _target;
    Comm* _comm;
    float _board_x, _board_y;
    float _ball_size;
    int _n_prey, _n_predator;
    bool _stop = true;
    
public:
    Scheduler(){
        _board_x = 10;
        _board_y = 10;
        _ball_size = 1;
        srand(time(NULL));
        
        _comm = new Comm();
    };
    ~Scheduler(){
        clear_buffer();
        
        delete _target;
        delete _comm;
    };

    // Add 3d object
    void add_object(Object &object){ _objects.push_back(object);};
    // Build play ground
    void build_environment();
    // Init players
    void init_players();
    // Update players' states
    void update();
    // Check if the position pos is in the playground, return the offset exceeding the boarder if it is not
    glm::vec2 in_board(glm::vec2 pos);
    // Update the speed of each player simulating the collision process
    void collision_update();
    // Update the next action of each player
    std::vector<glm::vec2> compute_action();
    // cluster players into prey and predator
    std::unordered_map<int, int> cluster_players();
    //
    void stop_detection();
    // Transfer environment(playground&target) states to string
    std::string static_state2string();
    // Transfer players' states to string
    std::string dynamic_state2string();
    //
    void init(int prey, int predator);
    void reset();
    void clear_buffer();
    
    // Getter
    Object* get_object(int obj_index){return &_objects[obj_index];};
    std::vector<Model*> get_models(){ return _models;};
};

