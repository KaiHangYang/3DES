#ifndef MCAFFE_PREDICTOR
#define MCAFFE_PREDICTOR
#include <vector>
#include <string>
#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
/* The util to use caffe model to predict */
class mCaffePredictor {
protected:
    caffe::shared_ptr<caffe::Net<float> > _net;
    cv::Mat _mean;
    uint _num_channel;
    cv::Size _input_size;
    
    void setMean(const std::string &mean_path);
    virtual void wrapInputLayer(std::vector<cv::Mat> * input_data);
    virtual void preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data);
public:
    mCaffePredictor(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path="");
    ~mCaffePredictor(){}
    std::vector<float> predict(const cv::Mat & img);
};



#endif
