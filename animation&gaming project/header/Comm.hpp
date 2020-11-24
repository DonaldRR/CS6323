//
//  Comm.hpp
//  animation&gaming project
//
//  Created by Donald Zhou on 11/16/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#ifndef Comm_hpp
#define Comm_hpp

#include <stdio.h>
#include <vector>

#include "Scheduler.hpp"

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

#endif /* Comm_hpp */
