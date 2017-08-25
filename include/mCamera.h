#pragma once
#include <iostream>
#include "mShader.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "mDefs.h"
class mCamera {
public:
    cv::VideoCapture m_camera;
    int wndWidth;
    int wndHeight;
    mCamera(int wndWidth, int wndHeight, mShader *camShader, bool isOpenCamera=true);
    ~mCamera();
    bool init();
    void drawFrame();
    void drawFrame(cv::Mat &frame);
private:
    GLuint VAO;
    mShader * camShader;
    GLuint vertexBuffer;
    GLuint uvBuffer;
    glm::mat4 MVP;
    GLuint textureID;
    GLuint samplerID;
    bool isOpenCamera;
    void initGLFrame();
    GLuint genTexture();
    void setTextureData(cv::Mat & frame);
};
