#ifndef VNECT_UTILS
#define VNECT_UTILS
#include "mCaffePredictor.hpp"

// According to the paper, I got that the image is 
// represented by joint angle \theta and camera location d
// I will maintain a bone length array. 
// The length is 3D length  
class mVNectUtils: public mCaffePredictor {
protected:
    virtual void preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data);
    virtual void wrapInputLayer(std::vector<cv::Mat> * input_data);
private:
    double _time_stamp; // used for filter
    bool _is_tracking;
    std::vector<float> _scales;
    cv::Size _box_size; 
    
    bool _is_first_frame;
    std::vector<int> _crop_rect;
    std::vector<int> _pad_offset;
    int _crop_size;
    float _crop_scale;
    // The scale of the input and the output. Now it's 8.
    float _hm_factor; 
    std::vector<std::vector<int> > joints_2d;
    std::vector<std::vector<int> > joints_3d; 
    cv::Mat padImage(const cv::Mat &img, cv::Size box_size);
    std::vector<int> crop_pos(bool type, int crop_offset=0);
public:
    mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path="");
    std::vector<std::vector<int> > predict(const cv::Mat &img, std::vector<std::vector<int> > &joints3d);
};
#endif
