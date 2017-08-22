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

//#include "./include/mCaffePredictor.hpp"
#include "include/vnectUtils.hpp"

int main() {
    mVNectUtils predictor("./caffemodel/vnect_model.caffemodel", "./caffemodel/vnect_net.prototxt");
    std::vector<float> tmp;
    tmp = predictor.predict(cv::imread("./imgtest/mpii_3dhp_ts6/cam5_frame000133.jpg"));
    std::cout << tmp.size() << std::endl;
    for (int i=0; i < 21; ++i) {
        cv::Mat img (cv::Size(106, 56), CV_32FC1, &tmp[0] + i * 56 * 106);
        cv::imshow("test", img);
        cv::waitKey(0);
    }
    std::cout << "testing " << std::endl;

    // TODO Now I don't know how to free the _net so, after when main exit, some problem will happend, I will 
    // handle it later 
    //std::cout << "test" << std::endl;
    //cv::Mat img = cv::imread("./imgtest/truck10.png");
    //std::vector<float> result = predictor.predict(img);
    return 0;
}
