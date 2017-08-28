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

#include "include/mShader.h"
#include "include/mCamera.h"
#include "include/mDefs.h"
#include "include/mMeshRender.h"

// VNect test
#include "include/vnectJointsInfo.hpp"
#include "include/vnectUtils.hpp"

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

void drawPoint(cv::Mat &img, std::vector<std::vector<int> > pos);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void joints_scale_3d(std::vector<std::vector<double> > & joints3d, std::vector<float> & result);
void mouse_move_callback(GLFWwindow * window, double x, double y);

int main(void) {
    // init glfw 
    GLFWwindow * window;
    
    cv::VideoCapture m_cam = cv::VideoCapture("./imgtest/a.mp4");
    cv::Mat frame;
    m_cam.set(CV_CAP_PROP_FRAME_WIDTH, wndWidth);
    m_cam.set(CV_CAP_PROP_FRAME_HEIGHT, wndHeight);

    mVNectUtils predictor("./caffemodel/vnect_model.caffemodel", "./caffemodel/vnect_net.prototxt");

    if ((window = InitWindow()) == nullptr) {
        system("pause");
        return -1;
    }

    SetOpenGLState();
    mShader camShader("/home/kaihang/Projects/3DES/shader/v.shader", "/home/kaihang/Projects/3DES/shader/f.shader");
    mShader objShader("/home/kaihang/Projects/3DES/shader/v2.shader", "/home/kaihang/Projects/3DES/shader/f2.shader");

    mCamera mcam(wndWidth, wndHeight, &camShader, false);
    if (false == mcam.init()) {
        system("pause");
        return -1;
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), ratio_w / ratio_h, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    //model = glm::mat4(1.0f);
    //model = glm::scale(model, glm::vec3(4, 4, 4));

    // model matrix
    glm::mat4 MVP = projection*view*model;

    mMeshRender meshes(view, projection, &objShader);
    meshes.addMesh("/home/kaihang/Projects/3DES/model/sphere.ply");
    meshes.addMesh("/home/kaihang/Projects/3DES/model/cylinder.ply");
    // 宽 7 高 4
    std::vector<float> vertexs({
                                0.1f, 0.7f, 0.4f, 
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
                                0.48f, -0.1f, 0.5f
                                });
    std::vector<unsigned int> indics({    
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

    std::vector<std::vector<int> > tmp;
    std::vector<std::vector<double>> tmp3d;
    indics = joint_indics;
    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (!m_cam.read(frame)) {
            continue;
        }
        tmp = predictor.predict(frame, tmp3d);
        joints_scale_3d(tmp3d, vertexs);
        drawPoint(frame, tmp);
        cv::flip(frame, frame, 1);
        mcam.drawFrame(frame);
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
void drawPoint(cv::Mat &img, std::vector<std::vector<int> > pos) {
    for (int i=0; i < pos.size(); ++i) {

        int x = pos[i][0] * img.size().width / vnect_resize_width;
        int y = pos[i][1] * img.size().height / vnect_resize_height;
        for (int j=-2; j < 3; ++j) {
            for (int k=-2; k < 3; ++k) {
                if (x+j < 0 || x+j >= img.size().height || y+k >= img.size().width || y+k < 0) {
                    std::cout << "out of range" <<std::endl;
                    continue;
                }
                // TODO:Here you need to get to know how the oepncv store data and how to 
                //      access the data in the mat.
                img.at<float>(x+j, y+k, 0) = 1;
                //img.at<char>(pos[i][0]+j, pos[i][1]+k, 2) = 1;
            }
        }
        
    }

}
void joints_scale_3d(std::vector<std::vector<double> > & joints3d, std::vector<float> & result) {
    result.clear();
    int scale_size = 4;
    for (int i=0; i < joints3d.size(); ++i) {
        // here I need to normalize them
        //
        
        result.push_back(scale_size * static_cast<double>(joints3d[i][0]));
        result.push_back(scale_size * static_cast<double>(joints3d[i][1]));
        result.push_back(scale_size * static_cast<double>(joints3d[i][2]));
        //std::cout << i << " "<< result[i*3 + 0] << ',' << result[i*3 + 1] << ", " << result[i*3 + 2]<< std::endl;
    }
}
