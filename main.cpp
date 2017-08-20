#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mShader.h"
#include "mCamera.h"
#include "mDefs.h"
#include "mMeshRender.h"

using namespace glm;


GLFWwindow * InitWindow();
void SetOpenGLState();
void error_callback(int error, const char *description) {
    printf("glfwERROR: code %d, desc:%s\n", error,  description);
}

extern bool isMousePressed;
extern double initX, initY;
extern double curX, curY;
extern glm::mat4 rotateMat;
extern glm::mat4 model;


bool isMousePressed = false;
double initX, initY;
double curX, curY;
glm::mat4 rotateMat = glm::mat4(1.0);
glm::mat4 model = glm::mat4(1.0);

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);


void mouse_move_callback(GLFWwindow * window, double x, double y);

int main(void) {
    // init glfw 
    GLFWwindow * window;
    
    if ((window = InitWindow()) == nullptr) {
        system("pause");
        return -1;
    }

    SetOpenGLState();
    mShader camShader("/home/kaihang/Projects/3DES/shader/v.shader", "/home/kaihang/Projects/3DES/shader/f.shader");
    mShader objShader("/home/kaihang/Projects/3DES/shader/v2.shader", "/home/kaihang/Projects/3DES/shader/f2.shader");

    mCamera mcam(wndWidth, wndHeight, &camShader);
    if (false == mcam.init()) {
        system("pause");
        return -1;
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    //model = glm::mat4(1.0f);
    //model = glm::scale(model, glm::vec3(4, 4, 4));

    // model matrix
    glm::mat4 MVP = projection*view*model;

    mMeshRender meshes(view, projection, &objShader);
    meshes.addMesh("/home/kaihang/Projects/3DES/model/sphere.ply");
    meshes.addMesh("/home/kaihang/Projects/3DES/model/cylinder.ply");

    std::vector<float> vertexs({0.1f, 0.7f, 0.4f, 
                                0.003f, 0.5f, 0.1f,
                                0, 0.3f, 0,
                                0, 0, 0,
                                0, -0.3f, 0,
                                -0.3f, -0.6f, 0.3f,
                                -0.4f, -0.8f, 0.35f,
                                -0.5f, -0.85f, 0.45f,
                                0.3f, -0.6f, 0.3f,
                                0.4f, -0.8f, 0.35f,
                                0.5f, -0.85f, 0.45f,
                                -0.4f, 0.25f, 0.3f,
                                -0.48f, -0.1f, 0.5f,
                                0.4f, 0.25f, 0.3f,
                                0.48f, -0.1f, 0.5f});
    std::vector<unsigned int> indics({    
                                        0, 1,
                                        1, 2,
                                        2, 3,
                                        3, 4, 
                                        4, 5,
                                        5, 6,
                                        6, 7,
                                        4, 8, 
                                        8, 9,
                                        9, 10,
                                        2, 11, 
                                        11, 12,
                                        2, 13,
                                        13, 14
                                                });

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mcam.drawFrame();
        
        
        glm::mat4 curModel;
        if (isMousePressed && (initX != curX || initY != curY)) {
            float tmpZ2;
            float tmpinitX = ((float)(wndWidth - initX - 1) / (float)wndHeight - 0.5)*2;
            float tmpinitY = ((float)initY / (float)wndHeight - 0.5)*2;
            tmpZ2 = 1 - tmpinitX*tmpinitX - tmpinitY*tmpinitY;
            glm::vec3 initVec(1.0);

            if (tmpZ2 < 0) {
                float tLen = sqrt(tmpinitY*tmpinitY + tmpinitX*tmpinitX);
                glm::vec4 from(tmpinitX / tLen, tmpinitY / tLen, 0, 1.0);
                glm::mat4 rmat = glm::rotate(glm::mat4(1.0), 1 - tLen, glm::cross(glm::vec3(tmpinitX, tmpinitY, 0), glm::vec3(0, 0, 1)));
                
                glm::vec4 tmpm = rmat* from;
                initVec.x = tmpm.x;
                initVec.y = tmpm.y;
                initVec.z = tmpm.z;
            }
            else {
                initVec = glm::normalize(glm::vec3(tmpinitX, tmpinitY, sqrt(tmpZ2)));
            }
            
            float tmpcurX = ((float)(wndWidth - curX - 1) / (float)wndHeight - 0.5)*2;
            float tmpcurY = ((float)curY / (float)wndHeight - 0.5)*2;
            glm::vec3 curVec;

            tmpZ2 = 1 - tmpcurX*tmpcurX - tmpcurY*tmpcurY;

            if (tmpZ2 < 0) {
                float tLen = sqrt(tmpcurY*tmpcurY + tmpcurX*tmpcurX);
                glm::vec4 from(tmpcurX / tLen, tmpcurY / tLen, 0, 1.0);
                glm::mat4 rmat = glm::rotate(glm::mat4(1.0), 1 - tLen, glm::cross(glm::vec3(tmpcurX, tmpcurY, 0), glm::vec3(0, 0, 1)));

                glm::vec4 tmpm = rmat* from;
                curVec.x = tmpm.x;
                curVec.y = tmpm.y;
                curVec.z = tmpm.z;
            }
            else {
                curVec = glm::normalize(glm::vec3(tmpcurX, tmpcurY, sqrt(tmpZ2)));
            }
            
            rotateMat = glm::rotate(glm::mat4(1.0), glm::acos(glm::dot(initVec, curVec)), glm::cross(curVec, initVec));

            curModel = rotateMat * model;
        }
        else {
            curModel = model;
        }
        meshes.render(vertexs, indics, curModel);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    
    return 0;
}

GLFWwindow * InitWindow() {
    glfwSetErrorCallback(error_callback);
    if (false == glfwInit()) {
        std::cout << "glfwInit failed!" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);// ? Don't know what's it;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(wndWidth, wndHeight, "3D Pose Estimate", nullptr, nullptr);
    if (!window) {
        std::cout << "Window create failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW\n" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return window;
}

void SetOpenGLState() {
    // enable depth test and accept fragment if it closer to the camera than the former one
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            printf("Pressed left Key!\n");
            initX = curX;
            initY = curY;
            isMousePressed = true;
            break;
        }
    }
    else {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            printf("Release left Key!\n");
            isMousePressed = false;
            model = rotateMat * model;
            //rotateMat = glm::mat4(1.0);
            break;
        }
    }
}

void mouse_move_callback(GLFWwindow * window, double x, double y) {
    curX = x;
    curY = y;

    if (isMousePressed) {
        std::cout << "cursor is at (" << curX << ", " << curY << ")" << std::endl;
    }
}
