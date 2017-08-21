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

#include "mCaffePredictor.hpp"

using namespace caffe;

int main() {
    LOG(INFO) << "cifar10 test" << std::endl;
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(0);
    return 0;
}
