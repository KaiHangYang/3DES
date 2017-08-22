#ifndef VNECT_UTILS
#define VNECT_UTILS
#include "mCaffePredictor.hpp"

class mVNectUtils: public mCaffePredictor {
protected:
    virtual void preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data);
    virtual void wrapInputLayer(std::vector<cv::Mat> * input_data);
private:
    bool _is_tracking;
    std::vector<float> _scales;
    cv::Size _box_size;   
    int _crop_size;
    float _crop_scale;

    cv::Mat padImage(const cv::Mat &img, cv::Size box_size);
public:
    mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path="");
    std::vector<float> predict(const cv::Mat &img);
};
#endif
