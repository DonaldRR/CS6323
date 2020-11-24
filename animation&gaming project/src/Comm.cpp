//
//  Comm.cpp
//  animation&gaming project
//
//  Created by Donald Zhou on 11/16/20.
//  Copyright © 2020 Donald Zhou. All rights reserved.
//

#include "Comm.hpp"

//------------------------------------------------- Comm -------------------------------------------------------

void Comm::init(){
    
    _serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(1254);
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
//    std::cout << "[Received] " << s_buffer << std::endl;
}

Message Comm::get_message()
{
    std::string s_buffer = std::string(_buffer);
    
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
            idx_n = s_buffer.find(";");
//            idx_n = s_buffer.find(",");
            action[1] = std::stof(s_buffer.substr(0, idx_n));
            s_buffer.erase(0, idx_n + 1);
            
            float v = glm::distance(glm::vec2{0.0f, 0.0f}, action);
            action = glm::normalize(action) * v;
            
            idx_item = s_buffer.find(";");
//            std::cout << "action:" << glm::to_string(action) << std::endl;
            _msg.vecs.push_back(action);
        }
    }
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
