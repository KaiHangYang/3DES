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

void drawPoint(cv::Mat &img, std::vector<std::vector<int> > pos) {
    for (int i=0; i < pos.size(); ++i) {

        int x = pos[i][0];
        int y = pos[i][1];
        for (int j=-2; j < 3; ++j) {
            for (int k=-2; k < 3; ++k) {
                if (x+j < 0 || x+j >= img.size().height || y+k >= img.size().width || y+k < 0) {
                    std::cout << "out of range" <<std::endl;
                    continue;
                }
                // TODO:Here you need to get to know how the oepncv store data and how to 
                //      access the data in the mat.
                img.at<float>(pos[i][0]+j, pos[i][1]+k, 0) = 1;
                //img.at<char>(pos[i][0]+j, pos[i][1]+k, 2) = 1;
            }
        }
        
    }

}

int main() {
    mVNectUtils predictor("./caffemodel/vnect_model.caffemodel", "./caffemodel/vnect_net.prototxt");
    std::vector<std::vector<int> > tmp, pretmp;

    for (int i=131; i <= 160; ++i) {
        char pathname[200];
        sprintf(pathname, "./imgtest/mpii_3dhp_ts6/cam5_frame000%d.jpg", i);
        cv::Mat img = cv::imread(pathname);
        tmp = predictor.predict(img);
        if (i != 131) {
            for (int i=0; i < 21; ++i) {
                std::cout << "(" << pretmp[i][0] - tmp[i][0] << ", " << pretmp[i][1] - tmp[i][1] << ", " << pretmp[i][2] - tmp[i][2] << ")" << std::endl;
            }
        }
        pretmp = tmp;
        //cv::Mat tmp2;
        //cv::resize(img, tmp2, cv::Size(424, 224));
        //drawPoint(tmp2, tmp);
        //cv::imshow("hehe", tmp2);
        cv::waitKey();
    }

        std::cout << "testing " << std::endl;

    // TODO Now I don't know how to free the _net so, after when main exit, some problem will happend, I will 
    // handle it later 
    //std::cout << "test" << std::endl;
    //cv::Mat img = cv::imread("./imgtest/truck10.png");
    //std::vector<float> result = predictor.predict(img);
    return 0;
}
