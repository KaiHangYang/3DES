#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

class mShader {
public:
    GLuint ID;
    mShader() {}
    mShader(std::string vs_path, std::string fs_path);
    ~mShader();
    void use();
    void setVal(const char * val_name, glm::vec3 val);
    void setVal(const char * val_name, glm::mat4 val);
    void setVal(const char * val_name, float val);
    void setVal(const char * val_name, unsigned int val);
    void setVal(const char * val_name, int val);
    
};
