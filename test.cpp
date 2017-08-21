#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cassert>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <caffe/caffe.hpp>
#include <caffe/util/io.hpp>
#include <caffe/blob.hpp>

#include "./include/mCaffePredictor.hpp"

using namespace caffe;

int main() {
    mCaffePredictor predictor("./caffemodel/vnect_model.caffemodel", "./caffemodel/vnect_net.prototxt");
    std::cout << "testing " << std::endl;
    //std::cout << "test" << std::endl;
    //cv::Mat img = cv::imread("./imgtest/truck10.png");
    //std::vector<float> result = predictor.predict(img);
    return 0;
}
