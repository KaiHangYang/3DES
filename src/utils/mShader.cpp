#include "mShader.h"


#include <fstream>
#include <iostream>
#include <vector>


mShader::~mShader() {
    glDeleteProgram(ID);
}
mShader::mShader(std::string vs_path, std::string fs_path) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string vertexShaderCode;
    std::ifstream vertexShaderStream(vs_path, std::ios::in);
    std::string fragShaderCode;
    std::ifstream fragShaderStream(fs_path, std::ios::in);
    try {
        if (vertexShaderStream.is_open()) {
            std::string line = "";
            while (getline(vertexShaderStream, line)) {
                vertexShaderCode += "\n" + line;
            }
            vertexShaderStream.close();
        }
        if (fragShaderStream.is_open()) {
            std::string line = "";
            while (getline(fragShaderStream, line)) {
                fragShaderCode += "\n" + line;
            }
            fragShaderStream.close();
        }
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR: Shader files read failed!" << std::endl;
    }


    GLint result = GL_FALSE;
    int infoLogLength;

    char const * vertexSourcePointer = vertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (!result) {
        std::vector<char> vertexShaderErrorMsg(infoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMsg[0]);
        printf("%s\n", &vertexShaderErrorMsg[0]);
    }

    char const * fragSourcePointer = fragShaderCode.c_str();
    glShaderSource(fragShaderID, 1, &fragSourcePointer, NULL);
    glCompileShader(fragShaderID);
    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (!result) {
        std::vector<char> fragShaderErrorMsg(infoLogLength + 1);
        glGetShaderInfoLog(fragShaderID, infoLogLength + 1, NULL, &fragShaderErrorMsg[0]);
        printf("%s\n", &fragShaderErrorMsg[0]);
    }

    // Link the program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShaderID);
    glAttachShader(ID, fragShaderID);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &result);
    glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (result > 0) {
        std::vector<char> programErrorMsg(infoLogLength + 1);
        glGetShaderInfoLog(ID, infoLogLength + 1, NULL, &programErrorMsg[0]);
        printf("%s\n", &programErrorMsg[0]);
    }
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragShaderID);
}
void mShader::use() {
    glUseProgram(ID);
}
void mShader::setVal(const char * val_name, glm::mat4 val) {
    GLuint valPos = glGetUniformLocation(ID, val_name);
    glUniformMatrix4fv(valPos, 1, GL_FALSE, &val[0][0]);
}
void mShader::setVal(const char * val_name, unsigned int val) {
    GLuint valPos = glGetUniformLocation(ID, val_name);
    glUniform1ui(valPos, val);
}
void mShader::setVal(const char * val_name, float val) {
    GLuint valPos = glGetUniformLocation(ID, val_name);
    glUniform1f(valPos, val);
}
void mShader::setVal(const char * val_name, int val) {
    GLuint val_pos = glGetUniformLocation(ID, val_name);
    glUniform1i(val_pos, val);
}
void mShader::setVal(const char * val_name, glm::vec3 val) {
    GLuint val_pos = glGetUniformLocation(ID, val_name);
    glUniform3fv(val_pos, 1, &val[0]);
}
