#pragma once
#include <iostream>
#include "mShader.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
class mCamera {
public:
    cv::VideoCapture m_camera;
    int wndWidth;
    int wndHeight;
    mCamera(int wndWidth, int wndHeight, mShader *camShader);
    ~mCamera();
    bool init();
    void drawFrame();
private:
    GLuint VAO;
    mShader * camShader;
    GLuint vertexBuffer;
    GLuint uvBuffer;
    glm::mat4 MVP;
    GLuint textureID;
    GLuint samplerID;

    void initGLFrame();
    GLuint genTexture();
    void setTextureData(cv::Mat & frame);
};
