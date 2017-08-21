#ifndef MCAFFE_PREDICTOR
#define MCAFFE_PREDICTOR
#include <vector>
#include <string>
#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
/* The util to use caffe model to predict */
class mCaffePredictor {
    caffe::shared_ptr<caffe::Net<float> > _net;
    cv::Mat _mean;
    uint _num_channel;
    cv::Size _input_size;

    void setMean(const std::string &mean_path);
    void wrapInputLayer(std::vector<cv::Mat> * input_data);
    void preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data);
public:
    mCaffePredictor(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path);
    std::vector<float> predict(const cv::Mat & img);
};



#endif
