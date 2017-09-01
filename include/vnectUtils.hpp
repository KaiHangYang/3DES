#ifndef VNECT_UTILS
#define VNECT_UTILS
#include "mCaffePredictor.hpp"
#include <glm/glm.hpp>
#include "oneEuro.hpp"
#include "vnectJointsInfo.hpp"

// According to the paper, I got that the image is 
// represented by joint angle \theta and camera location d
// I will maintain a bone length array. 
// The length is 3D length  
// When fitting, I find something strange, the fitting process doesn't change the z of d, only changed the so called "angles"
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
    double * joints_2d[3];
    double * joints_3d[3];
    // Used for fitting! P^G_t P^G_t-1 P^G_t-2 and the same order of global_d
    double * joint_angles[3];
    double * global_d[3];
    one_euro_filter<double> *mFilters[joint_num];
    one_euro_filter<double> *mFilters_3d[joint_num];
    glm::mat4 mvp;

    cv::Mat padImage(const cv::Mat &img, cv::Size box_size);
    std::vector<int> crop_pos(bool type, int crop_offset=0);
    // Used to get the 3d location of all points by \theta and d
    void cal_3dpoints(const double * angles, const double * d, double * result);
public:
    ~mVNectUtils();
    mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path="");
    void predict(const cv::Mat &img, double * joint2d, double * joints3d);
};
#endif
