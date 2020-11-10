////
////  Shader.cpp
////  animation&gaming project
////
////  Created by Donald Zhou on 10/6/20.
////  Copyright © 2020 Donald Zhou. All rights reserved.
////
//
//#include "Shader.hpp"
//
//void Shader::use()
//{
//    glUseProgram(this->program);
//}
//
//void Shader::check_compile_error(GLuint shader, std::string type)
//{
//    GLint success;
//    GLchar info_log[1024];
//    if (type == "PROGRAM")
//    {
//        glGetProgramiv(shader, GL_LINK_STATUS, &success);
//        if (!success)
//        {
//            glGetShaderInfoLog(shader, 1024, NULL, info_log);
//            std::cout << "| Error:: PROGRAM-LINKING-ERROR of type: " << type << "|\n" << info_log << "\n|-------------------------------------|\n";
//        }
//    }
//    else if (type == "VERTEX" || type == "FRAGMENT" || type == "GEOMETRY")
//    {
//        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//        if (!success)
//        {
//            glGetShaderInfoLog(shader, 1024, NULL, info_log);
//            std::cout << "| Error:: SHADER-COMPILATION-ERROR of type: " << type << "|\n" << info_log << "\n|------------------------------|\n";
//        }
//    }
//    else{
//        std::cout << "Error: incorrect input type\n";
//    }
//}
