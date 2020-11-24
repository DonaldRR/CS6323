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
#include <algorithm>
#include <random>
#include <math.h>
#include <time.h>
#include <set>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Object.hpp"
#include "Comm.hpp"
#include "Model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


#define PLAYER_UPDATE_PERIOD 80
#define FORCE_APPLY_PERIOD 5
#define TIME_SLICES 5

class Scheduler;
class Comm;
class Model;
class Player;
class Target;


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
    int _time_stamp=0;
    
public:
    Scheduler();
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
    int enable_action();
    
    // Getter
    Object* get_object(int obj_index){return &_objects[obj_index];};
    std::vector<Model*> get_models(){ return _models;};
};


#endif /* scheduler_hpp */
