//
//  Model.hpp
//  animation&gaming project
//
//  Created by Donald Zhou on 10/6/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

//#ifndef Model_hpp
//#define Model_hpp
//
//#include <stdio.h>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iostream>
//#include <algorithm>
//#include <vector>

//#include "Scheduler.hpp"
//#include "Object.hpp"

//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#endif /* Model_hpp */

//enum MODEL_TYPE{
//    GROUND,
//    WALL,
//    PLAYER
//};
//struct Position {
//    float x;
//    float y;
//};
//struct Speed {
//    float vx;
//    float vy;
//};
//
//class Model {
//public:
//    MODEL_TYPE _model_type;
//    int _model_idx;
//    glm::vec3 _color;
//    int _object_idx;
//    Scheduler* _scheduler;
//    glm::mat4 _model_mat;
//    
//public:
//    Model(MODEL_TYPE model_type, int model_idx, glm::vec3 color, int object_idx, glm::mat4 model_mat){
//        _model_type = model_type;
//        _model_idx = model_idx;
//        _color = color;
//        _object_idx = object_idx;
//        _model_mat = model_mat;
//    };
//    ~Model(){};
//    
//    void setScheduler(Scheduler* scheduler){_scheduler = scheduler};
//    void update_speed(){};
//    
//    Object getObject(){return _scheduler->getObject(_obejct_idx);};
//    glm::mat4 getModelMat(){return _model_mat;};
//    glm::vec3 getColor(){return _color;};
//    void setModelMat(glm:mat4 model_mat) {_model_mat = model_mat;};
//    
//};
