#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <math.h>

// GL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

enum ObjectType{
    PLANE,
    BALL
};

class Object
{
public:
    struct Vertex {
        // Position
        glm::vec3 Position;
        // Normal
        glm::vec3 Normal;
        // TexCoords
        glm::vec2 TexCoords;
    };

    struct Vertex_Index {
        int pos_idx;
        int normal_idx;
        int texcoord_idx;
    };

    struct Face_Index {
        Vertex_Index vertex[3];
    };

    // veo and vao vector
    std::vector<Vertex> vao_vertices;
    std::vector<unsigned int> veo_indices;

    // obj original data vector
    std::vector<glm::vec3> ori_positions;
    std::vector<glm::vec3> ori_normals;
    std::vector<glm::vec2> ori_texcoords;

    // obj face index vector
    std::vector<Face_Index> indexed_faces;

    glm::vec3 obj_center;
    
    glm::vec4 obj_color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    GLfloat shininess = 32.0f;

    std::string m_obj_path;
    std::string obj_name;

    GLuint vao, vbo;

public:
    void clear_buffer()
    {
        this->vao_vertices.clear();
        this->veo_indices.clear();
        this->indexed_faces.clear();

        this->ori_positions.clear();
        this->ori_normals.clear();
        this->ori_texcoords.clear();
    }
    
    void make_ball()
    {
        this->clear_buffer();
        float radius = 0.5;
        int n_layers = 180, n_sectors = 360;
        // Store vertices
        this->ori_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        for (int i  = 1; i < n_layers; i ++ ) {
            
            float alpha = M_PI * (i / float(n_layers));
            float z = radius * (1 - cos(alpha));
            for (int j = 0; j < n_sectors; j ++ ) {
                float theta = 2 * M_PI * (j / float(n_sectors));
                float x, y;
                x = radius * sin(alpha) * cos(theta);
                y = radius * sin(alpha) * sin(theta);
                this->ori_positions.push_back(glm::vec3(x, y, z));
            }
        }
        this->ori_positions.push_back(glm::vec3(0.0f, 0.0f, radius * 2));
        
        // Store faces
        for (int i = 0; i < n_layers; i ++) {
            for (int j = 0; j < n_sectors; j ++) {
                if (i == 0)
                {
                    int vec1_idx = 0;
                    int vec2_idx = j + 1;
                    int vec3_idx = j + 2;
                    Vertex vert1, vert2, vert3;
                    vert1.Position = this->ori_positions[vec1_idx];
                    vert2.Position = this->ori_positions[vec2_idx];
                    vert3.Position = this->ori_positions[vec3_idx];
                    vao_vertices.push_back(vert2);
                    vao_vertices.push_back(vert1);
                    vao_vertices.push_back(vert3);
                }
                else if (i == n_layers - 1)
                {
                    int vec1_idx = i * n_sectors + j + 1;
                    int vec2_idx = vec1_idx + 1 + 1;
                    int vec3_idx = (i + 1) * n_sectors + 1;
                    Vertex vert1, vert2, vert3;
                    vert1.Position = this->ori_positions[vec1_idx];
                    vert2.Position = this->ori_positions[vec2_idx];
                    vert3.Position = this->ori_positions[vec3_idx];
                    vao_vertices.push_back(vert1);
                    vao_vertices.push_back(vert2);
                    vao_vertices.push_back(vert3);
                }
                else{
                    int vec1_idx = i * n_sectors + j + 1;
                    int vec2_idx = vec1_idx + 1 + 1;
                    int vec3_idx = (i + 1) * n_sectors + j + 1;
                    int vec4_idx = vec3_idx + 1 + 1;
                    
                    Vertex vert1, vert2, vert3, vert4;
                    vert1.Position = this->ori_positions[vec1_idx];
                    vert2.Position = this->ori_positions[vec2_idx];
                    vert3.Position = this->ori_positions[vec3_idx];
                    vert4.Position = this->ori_positions[vec4_idx];
                    vert1.Normal = vert1.Position - glm::vec3(0.0f, 0.0f, radius);
                    vert2.Normal = vert2.Position - glm::vec3(0.0f, 0.0f, radius);
                    vert3.Normal = vert3.Position - glm::vec3(0.0f, 0.0f, radius);
                    vert4.Normal = vert4.Position - glm::vec3(0.0f, 0.0f, radius);
                    vao_vertices.push_back(vert1);
                    vao_vertices.push_back(vert2);
                    vao_vertices.push_back(vert3);
                    vao_vertices.push_back(vert3);
                    vao_vertices.push_back(vert2);
                    vao_vertices.push_back(vert4);
                }
            }
        }
    }
    
    void make_plane()
    {
        this->clear_buffer();
        glm::vec3 vec1(-0.5f, 0.5f, 0.0f), vec2(0.5f, -0.5f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec4(-0.5f, -0.5f, 0.0f);
        glm::vec3 default_normal(0.0f, 0.0f, 1.0f);
        
        this->ori_positions.push_back(vec1);
        this->ori_positions.push_back(vec2);
        this->ori_positions.push_back(vec3);
        this->ori_positions.push_back(vec4);
        for (int i = 0; i < 4; i ++) {
            this->ori_normals.push_back(default_normal);
        }
            
        Vertex vert1, vert2, vert3, vert4;
        vert1.Position = vec1;
        vert2.Position = vec2;
        vert3.Position = vec3;
        vert4.Position = vec4;
        vert1.Normal = default_normal;
        vert2.Normal = default_normal;
        vert3.Normal = default_normal;
        vert4.Normal = default_normal;
        
        vao_vertices.push_back(vert2);
        vao_vertices.push_back(vert3);
        vao_vertices.push_back(vert4);
        vao_vertices.push_back(vert4);
        vao_vertices.push_back(vert3);
        vao_vertices.push_back(vert1);
        veo_indices.push_back(0);
        veo_indices.push_back(1);
        veo_indices.push_back(2);
        veo_indices.push_back(3);
        veo_indices.push_back(4);
        veo_indices.push_back(5);
    }
    
    void load_obj(std::string obj_path)
    {
        int path_str_length = obj_path.size();
        std::string suffix = obj_path.substr(path_str_length - 3, path_str_length);

        if (suffix == "obj") {
            this->clear_buffer();

            std::ifstream ifs;
            // Store original data vector
            try {
                ifs.open(obj_path);
                std::string one_line;
                while (getline(ifs, one_line)) {
                    std::stringstream ss(one_line);
                    std::string type;
                    ss >> type;
                    if (type == "v") {
                        glm::vec3 vert_pos;
                        ss >> vert_pos[0] >> vert_pos[1] >> vert_pos[2];
                        this->ori_positions.push_back(vert_pos);
//                        std::cout << type << glm::to_string(vert_pos) << std::endl;
                    }
                    else if (type == "vt") {
                        glm::vec2 tex_coord;
                        ss >> tex_coord[0] >> tex_coord[1];
                        this->ori_texcoords.push_back(tex_coord);
//                        std::cout << type << glm::to_string(tex_coord) << std::endl;
                    }
                    else if (type == "vn") {
                        glm::vec3 vert_norm;
                        ss >> vert_norm[0] >> vert_norm[1] >> vert_norm[2];
                        this->ori_normals.push_back(vert_norm);
//                        std::cout << type << glm::to_string(vert_norm) << std::endl;
                    }
                    else if (type == "f") {
                        Face_Index face_idx;
                        // Here we only accept face number 3
                        for (int i = 0; i < 3; i++) {
                            std::string s_vertex;
                            ss >> s_vertex;
                            int pos_idx = -1;
                            int tex_idx = -1;
                            int norm_idx = -1;
                            sscanf(s_vertex.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
                            // We have to use index -1 because the obj index starts at 1
                            // Incorrect input will be set as -1
                            face_idx.vertex[i].pos_idx = pos_idx > 0 ? pos_idx - 1 : -1;
                            face_idx.vertex[i].texcoord_idx = tex_idx > 0 ? tex_idx - 1 : -1;
                            face_idx.vertex[i].normal_idx = norm_idx > 0 ? norm_idx - 1 : -1;
                        }
                        indexed_faces.push_back(face_idx);
                    }
                }
            }
            catch (const std::exception&) {
                std::cout << "Error: Obj file cannot be read\n";
            }
            
            // Retrieve data from index and assign to vao and veo
            for (int i = 0; i < indexed_faces.size(); i++) {
                Face_Index cur_idx_face = indexed_faces[i];
                // If no normal: recalculate for them
                glm::vec3 v0 = ori_positions[cur_idx_face.vertex[0].pos_idx];
                glm::vec3 v1 = ori_positions[cur_idx_face.vertex[1].pos_idx];
                glm::vec3 v2 = ori_positions[cur_idx_face.vertex[2].pos_idx];
                glm::vec3 new_normal = glm::cross(v1 - v0, v2 - v0);

                for (int j = 0; j < 3; j++) {
                    Vertex cur_vertex;
                    Vertex_Index cur_idx_vertex = cur_idx_face.vertex[j];
                    if (cur_idx_vertex.pos_idx >= 0) {
                        cur_vertex.Position = ori_positions[cur_idx_vertex.pos_idx];
                    }
                    if (cur_idx_vertex.normal_idx >= 0) {
                        cur_vertex.Normal = ori_normals[cur_idx_vertex.normal_idx];
                    }
                    else {
                        cur_vertex.Normal = new_normal;
                    }
                    if (cur_idx_vertex.texcoord_idx >= 0) {
                        cur_vertex.TexCoords = ori_texcoords[cur_idx_vertex.texcoord_idx];
                    }
                    vao_vertices.push_back(cur_vertex);
                    veo_indices.push_back(i * 3 + j);
                }
            }
        }
    };

    void calculate_center()
    {
        glm::vec3 max_bound(INT_MIN);
        glm::vec3 min_bound(INT_MAX);
        for (auto vertex : this->vao_vertices) {
            max_bound[0] = std::max(vertex.Position[0], max_bound[0]);
            max_bound[1] = std::max(vertex.Position[1], max_bound[1]);
            max_bound[2] = std::max(vertex.Position[2], max_bound[2]);
            min_bound[0] = std::min(vertex.Position[0], min_bound[0]);
            min_bound[1] = std::min(vertex.Position[1], min_bound[1]);
            min_bound[2] = std::min(vertex.Position[2], min_bound[2]);
        }
        this->obj_center = (max_bound + min_bound) * 0.5f;

    };
    

    Object(std::string obj_path) {
        this->m_obj_path = obj_path;
        load_obj(this->m_obj_path);
        calculate_center();
    };
    
    Object(ObjectType obj_type)
    {
        switch(obj_type)
        {
            case PLANE:
                make_plane();
                break;
            case BALL:
                make_ball();
                break;
            default:
                std::cout << obj_type << " is not an available object" << std::endl;
        }
//        make_ball();

        calculate_center();
    }

    ~Object() {};
};
