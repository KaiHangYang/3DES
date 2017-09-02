#include "../../include/vnectUtils.hpp"
#include "../../include/vnectJointsInfo.hpp"
#include "../../include/mDefs.h"
#include "../../include/mFittingUtils.hpp"
#include <algorithm>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <time.h>

mVNectUtils::mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path):mCaffePredictor(model_path, deploy_path, mean_path) {
    _is_tracking = false;
    _box_size = cv::Size(0, 0);
    _crop_size = 368;
    _pad_offset = std::vector<int>({0, 0});
    _crop_scale = 1.0;
    _crop_rect = std::vector<int>({0, 0, _crop_size, _crop_size});
    _hm_factor = 8.0;
    _is_first_frame = true;
    _time_stamp = 0;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), ratio_w / ratio_h, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    mvp = projection * view;
    for (int i=0; i < 3; ++i) {
        joints_2d[i] = new double[2*joint_num];
        joints_3d[i] = new double[3*joint_num];
        joint_angles[i] = new double[3*(joint_num-1)];
        global_d[i] = new double[3];
    }

    // initialize the filters 
    for (int i=0; i < joint_num; ++i) {
        mFilters[i] = new one_euro_filter<double> (1, 1.7, 0.3, 1);
        mFilters_3d[i] = new one_euro_filter<double> (1, 0.8, 0.4, 1);
    }
}

mVNectUtils::~mVNectUtils() {
    for (int i=0; i < 3; ++i) {
        delete joint_angles[i];
        delete global_d[i];
        delete joints_2d[i];
        delete joints_3d[i];
    }
    for (int i=0; i < joint_num; ++i) {
        delete mFilters[i];
        delete mFilters_3d[i];
    }
}
std::vector<int> mVNectUtils::crop_pos(bool type, int crop_offset) {
    std::vector<int> result({0, 0});
    if (!type) {
        // 0 get the min
        result[0] = 65536;
        result[1] = 65536;
        for (int i=0; i < joint_num; ++i) {
            double tmp1 = (joints_2d[0][2*i + 0]+0.5) * vnect_resize_height;
            double tmp2 = (joints_2d[0][2*i + 1]+0.5) * vnect_resize_width;
            if (tmp1 < result[0]) {
                result[0] = tmp1;
            }
            if (tmp2 < result[1]) {
                result[1] = tmp2;
            }
        }

        result[0] = result[0] - crop_offset;
        result[1] = result[1] - crop_offset;

    }
    else {
        // get the max
        for (int i=0; i < joint_num; ++i) {
            double tmp1 = (joints_2d[0][2*i + 0]+0.5) * vnect_resize_height;
            double tmp2 = (joints_2d[0][2*i + 1]+0.5) * vnect_resize_width;
            if (tmp1 > result[0]) {
                result[0] = tmp1;
            }
            if (tmp2 > result[1]) {
                result[1] = tmp2;
            }
        }
        result[0] = result[0] + crop_offset;
        result[1] = result[1] + crop_offset;
    }
    return result;
}
void mVNectUtils::wrapInputLayer(std::vector<cv::Mat> * input_data) {
    // Before call this function the net has been reshaped
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    input_data->clear(); 
    int width = input_layer->width();
    int height = input_layer->height();
    // Get the pointer point to the input data layer
    float * data = input_layer->mutable_cpu_data();

    // map the pointer to the cv::mat struct
    // cause the input may have a lot of scales, the upper is multipled by num()
    for (int i=0; i < input_layer->channels()*input_layer->num(); ++i) {
        cv::Mat channel(height, width, CV_32FC1, data);
        input_data->push_back(channel);
        data += width*height;
    }
}
void mVNectUtils::preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data) {
    cv::Mat tmp;
    // VNect's input image is 3 channels
    if (img.channels() == 4) {
        cv::cvtColor(img, tmp, cv::COLOR_BGRA2BGR);
    }
    else if (img.channels() == 1) {
        cv::cvtColor(img, tmp, cv::COLOR_GRAY2BGR);
    }
    else {
        tmp = img;
    }
    
    if (!_is_tracking) {
        // the first frame
        _box_size = cv::Size(tmp.size().width, tmp.size().height);
        _is_tracking = true;
    }
    else {
        _box_size = cv::Size(_crop_size, _crop_size);
        //float crop_offset = static_cast<int>(40.0/_crop_scale);
        float crop_offset = static_cast<int>(40.0);
        std::vector<int> min_crop = crop_pos(0, crop_offset);
        std::vector<int> max_crop = crop_pos(1, crop_offset);
        
        std::vector<int> old_crop = _crop_rect;
        //_crop_rect[0] = std::max(min_crop[0], 0);
        //_crop_rect[1] = std::max(min_crop[1], 0);

        //_crop_rect[2] = std::min(max_crop[0], tmp.size().width) - _crop_rect[0];
        //_crop_rect[3] = std::min(max_crop[1], tmp.size().height) - _crop_rect[1];
        if (_is_first_frame) {
            float mu = 0.8;
            for (int i=0; i < 4; ++i) {
                _crop_rect[i] = (1-mu) * _crop_rect[i] + mu * old_crop[i];
            }
        }
        _crop_rect[0] = std::max(min_crop[1], 0);
        _crop_rect[1] = std::max(min_crop[0], 0);

        _crop_rect[2] = std::min(max_crop[1], tmp.size().width) - _crop_rect[0];
        _crop_rect[3] = std::min(max_crop[0], tmp.size().height) - _crop_rect[1];


        cv::Rect crop_rect(_crop_rect[0], _crop_rect[1], _crop_rect[2], _crop_rect[3]);
        tmp = tmp(crop_rect);

        cv::Size tmp_size = tmp.size();
        _crop_scale = (_crop_size - 2.0) / static_cast<float>(std::max(tmp_size.width, tmp_size.height));
        cv::resize(tmp, tmp, cv::Size(0, 0), _crop_scale, _crop_scale);
        tmp_size = tmp.size();
        if (tmp_size.width > tmp_size.height) {
            _pad_offset[0] = 0;
            _pad_offset[1] = (_crop_size - tmp_size.height)/2.0;
        }
        else {
            _pad_offset[1] = 0;
            _pad_offset[0] = (_crop_size - tmp_size.width)/2.0;
        }
        tmp = padImage(tmp, _box_size);
    }

    // Once the crop is known, process the img
    // the data type as the blob 
    std::vector<cv::Mat> data;
    cv::Mat tmp_resize;
    cv::Size hehe = tmp.size();
    if (tmp.size() != _input_size) {
        cv::resize(tmp, tmp_resize, _input_size);
    }
    else {
        tmp_resize = tmp;
    }

    cv::Mat tmp_float;
    // just for the next normalization
    if (_num_channel == 3) {
        tmp_resize.convertTo(tmp_float, CV_32FC3);
    } 
    else {
        tmp_resize.convertTo(tmp_float, CV_32FC1);
    }
    tmp_float = tmp_float/255.0 - 0.4;

    cv::Mat tmpScaleImg;
    cv::Mat tmpPadding;
    for (int i=0; i < _scales.size(); ++i) {
        // put all the scaled img to the data
        cv::resize(tmp_float, tmpScaleImg, cv::Size(0, 0), _scales[i], _scales[i]);
        tmpPadding = padImage(tmpScaleImg, _box_size);
        data.push_back(tmpPadding);
    }
    // put all the images to the input_layer
    for (int i=0; i < data.size(); ++i) {
        std::vector<cv::Mat> tmpchannels;
        cv::split(data.at(i), tmpchannels);
        for (int j=0; j < 3; ++j) {
            tmpchannels.at(j).copyTo((*input_data).at(j + _num_channel * i));
        }
    }
}
// Mat is like a shared_ptr
cv::Mat mVNectUtils::padImage(const cv::Mat &img, cv::Size box_size) {
    cv::Size pad_size = (box_size-img.size());
    cv::Mat dst = cv::Mat(box_size, img.type(), cv::Scalar::all(0));
    // Pay attention to this ,the third and forth parm is the width and the height of the rect
    cv::Rect rect = cv::Rect(pad_size.width/2, pad_size.height/2, img.size().width, img.size().height);
    img.copyTo(dst(rect));
    return dst;
}
void mVNectUtils::cal_3dpoints(const double * angles, const double * d, double * result) {
    result[3*14] = d[0];
    result[3*14 + 1] = d[1]; 
    result[3*14 + 2] = d[2];
    
    // Then calculate all the points from the root point.
    for (int i=0; i < joint_num - 1; ++i) {
        int from = joint_indics.at(2*i);
        int to = joint_indics.at(2*i+1);
        // Here the "from" point is already known
        result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * std::cos(glm::radians(angles[3*i + 0]));
        result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * std::cos(glm::radians(angles[3*i + 1]));
        result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * std::cos(glm::radians(angles[3*i + 2]));
    }
}

void mVNectUtils::predict(const cv::Mat &img, double * joint2d, double * joint3d) {
    cv::Mat tmp;
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    _num_channel = img.channels();
    // now the frequency is not very high
    // I made some mistakes. The instance of one_euro is only for one point...
    _time_stamp += 1.0/7;
    cv::resize(img, tmp, cv::Size(vnect_resize_width, vnect_resize_height));
    // Here according to the demo, the image is resized to [448, 848]
    // TODO: Change to only reshape the net once.
    
    if (_is_tracking) {
        // In tracking mode, the number of scales is 2, I don't know why, 
        int scale_size = 2;
        _input_size = cv::Size(_crop_size, _crop_size);
        _scales.clear();
        for (int i=0; i < scale_size; ++i) {
            _scales.push_back(1.0 - 0.3 * static_cast<float>(i));
        }
        input_layer->Reshape(scale_size, static_cast<int>(_num_channel), _input_size.height, _input_size.width);
    }
    else {
        // The multi-scales in the paper means, we need to scale the image to multi scale(eg 1.0, 0.8, 0.6). Then forward 
        // to get the average of the results of all scales.
        // The implement in the demo is 3 scales. Here my gpu's memory is not 
        // enough, so I use one scale to test.
        int scale_size = 3;
        _time_stamp = 0;
        _scales.clear();
        for (int i=0; i < scale_size; ++i) {
            _scales.push_back(1.0 - 0.2 * static_cast<float>(i));
        }
        _input_size = tmp.size();    // but this number can be changed.
        input_layer->Reshape(scale_size, static_cast<int>(_num_channel), _input_size.height, _input_size.width);
        
        // when tracking the input_size is the crop_size
    }
    
    _net->Reshape();

    std::vector<cv::Mat> input_data;

    wrapInputLayer(&input_data);

    preprocess(tmp, &input_data);

    _net->Forward();
    // There have been 4 blobs in the output the for confidence map is contained in them.
    std::vector<caffe::Blob<float> *> output_layer = _net->output_blobs();
    // Store the result heatmap and location map
    std::vector<std::vector<cv::Mat> > result;
    
    int o_width = output_layer[0]->width();
    int o_height = output_layer[0]->height();
    int o_channels = output_layer[0]->channels();
    int o_num = output_layer[0]->num();
    cv::Size hm_size = cv::Size(o_width, o_height);
    //std::cout << "hm_size:"<< hm_size.width << "," << hm_size.height << std::endl;
    // Get all the result, result[0] -> heatmap, result[1] -> x_location_map, result[2] -> y_location_map, result[3] -> z_location_map
    for (int i=0; i < output_layer.size(); ++i) {
        const float * begin = output_layer[i]->cpu_data();
        // get all the output data. The num is equal to the image you input 
        //const float * end = begin + o_width * o_height * o_channels * o_num;
        std::vector<cv::Mat> map_elm;
        for (int j=0; j < o_num * o_channels; ++j) {
            const float * start = begin + j * o_width * o_height;
            cv::Mat channel_elm(hm_size, CV_32FC1, (void *)start);
            map_elm.push_back(channel_elm);
        }
        result.push_back(map_elm);
    }
    for (int i = 0; i < o_num; ++i) {
        // Cause the image size in every scale is not equal, we need to resize the image
        if (std::abs(_scales.at(i) - 1.0) <= 0.0001) {
            // the raw image size
            continue;
        }
        for (int j = 0; j < o_channels; ++j) {
            //change the sise for every channel
            cv::Mat tmp;
            cv::Size pad_size;
            for (int k = 0; k < 4; ++k) {
                cv::resize(result[k][i*o_channels + j], tmp, cv::Size(hm_size.width/_scales.at(i), hm_size.height/_scales.at(i)));
                pad_size = tmp.size() - result[k][i*o_channels + j].size();
                cv::Rect content_rect(pad_size.width/2, pad_size.height/2, hm_size.width, hm_size.height);
                // Here I previously just let result to the tmprect, but 
                // the memory maybe released, may be it's because "rect" doesn't
                // increase the pointer num in the mat.
                tmp(content_rect).copyTo(result[k][i*o_channels + j]);
            }
        }
    }
    // Then I need to average the result, if the number of scale is not one.
    // 3 scales = 3 x 21 channel heatmap(cv::Mat)
    std::vector<cv::Mat> heatmaps;
    std::vector<cv::Mat> xmaps;
    std::vector<cv::Mat> ymaps;
    std::vector<cv::Mat> zmaps;

    for (int i = 0; i < o_channels; ++i) {
        // all map is single channel
        cv::Mat heatmap(hm_size, CV_32FC1, cv::Scalar::all(0));
        cv::Mat xmap(hm_size, CV_32FC1, cv::Scalar::all(0));
        cv::Mat ymap(hm_size, CV_32FC1, cv::Scalar::all(0));
        cv::Mat zmap(hm_size, CV_32FC1, cv::Scalar::all(0));
        
        for (int j = 0; j < o_num; ++j) {
            heatmap += 1.0/o_num * result[0][j * o_channels + i];
            xmap += 1.0/o_num * result[1][j * o_channels + i];
            ymap += 1.0/o_num * result[2][j * o_channels + i];
            zmap += 1.0/o_num * result[3][j * o_channels + i];
        }
        // Then push the average map into the vector
        heatmaps.push_back(heatmap);
        xmaps.push_back(xmap);
        ymaps.push_back(ymap);
        zmaps.push_back(zmap);
    }
    // then all the heatmaps is ready for calculate the 2D and 3D location
    // clear the joints stored in the vector
    // Here cause there is a bb, I need to change the location to the previous picture
    if (!_is_first_frame) {
        memcpy(joints_2d[2], joints_2d[1], sizeof(double)*2*joint_num);
        memcpy(joints_2d[1], joints_2d[0], sizeof(double)*2*joint_num);
        memcpy(joints_3d[2], joints_3d[1], sizeof(double)*3*joint_num);
        memcpy(joints_3d[1], joints_3d[0], sizeof(double)*3*joint_num);
    }
    memset(joints_2d[0], 0, sizeof(double) * 2 * joint_num);
    memset(joints_3d[0], 0, sizeof(double) * 3 * joint_num);

    for (int i=0; i < o_channels; ++i) {
        std::vector<int> p2({0, 0});
        std::vector<double> p3({0, 0, 0});
        
        cv::Mat hm;
        cv::resize(heatmaps[i], hm, _box_size);
        //if (i == 9) {
            //cv::imshow("left ankle", hm);
            //cv::waitKey();
        //}
        //if (i == 13) {
            //cv::imshow("right jiaohuai", hm);
            //cv::waitKey();
        //}

        //cv::imshow("testaa", hm);
        cv::minMaxIdx(hm, nullptr, nullptr, nullptr, &p2[0]);
        //int posx = static_cast<int>((*mFilters[i])(std::max(static_cast<int>(p2[0]/_hm_factor), 1), _time_stamp));
        //int posy = static_cast<int>((*mFilters[i])(std::max(static_cast<int>(p2[1]/_hm_factor), 1), _time_stamp));
        int posx = std::max(static_cast<int>(p2[0]/_hm_factor), 1);
        int posy = std::max(static_cast<int>(p2[1]/_hm_factor), 1);

        // Here, what you get is not the true (x, y, z), you need to minus the root joint 14
        //p3[0] = (*mFilters_3d[i])(100 * xmaps[i].at<float>(posx, posy) / _crop_scale, _time_stamp);
        //p3[1] = (*mFilters_3d[i])(100 * ymaps[i].at<float>(posx, posy) / _crop_scale, _time_stamp);
        //p3[2] = (*mFilters_3d[i])(100 * zmaps[i].at<float>(posx, posy) / _crop_scale, _time_stamp);
        p3[0] = 100 * xmaps[i].at<float>(posx, posy) / _crop_scale;
        p3[1] = 100 * ymaps[i].at<float>(posx, posy) / _crop_scale;
        p3[2] = 100 * zmaps[i].at<float>(posx, posy) / _crop_scale;

        // change them here
        //p2[0] = (*mFilters[i])(p2[0]/_crop_scale - _pad_offset[1]/_crop_scale + _crop_rect[1], _time_stamp); // row
        //p2[1] = (*mFilters[i])(p2[1]/_crop_scale - _pad_offset[0]/_crop_scale + _crop_rect[0], _time_stamp); // col
        p2[0] = p2[0]/_crop_scale - _pad_offset[1]/_crop_scale + _crop_rect[1]; // row
        p2[1] = p2[1]/_crop_scale - _pad_offset[0]/_crop_scale + _crop_rect[0]; // col

        // TODO: Here, the 2d points is normalized to [-0.5, 0.5]
        joints_2d[0][2*i + 0] = static_cast<double>(p2[0]) / vnect_resize_height - 0.5; // y
        joints_2d[0][2*i + 1] = static_cast<double>(p2[1]) / vnect_resize_width - 0.5; // x 
        joints_3d[0][3*i + 0] = p3[0];
        joints_3d[0][3*i + 1] = p3[1];
        joints_3d[0][3*i + 2] = p3[2];

        //std::cout << "pos2d:" << p2[0] << ',' << p2[1] << std::endl;
    }
    // Do this according to the demo code
    // Get the normalized 3d location of joints
    // TODO: But the 3d points is normalized to [-1, 1]
    for (int i=0; i < joint_num; ++i) {
        joints_3d[0][3*i + 0] = (joints_3d[0][3*i + 0] - joints_3d[0][14 * 3 + 0])/1600.0;
        joints_3d[0][3*i + 1] = -1*(joints_3d[0][3*i + 1] - joints_3d[0][14 * 3 + 1])/1600.0;
        joints_3d[0][3*i + 2] = -1*(joints_3d[0][3*i + 2] - joints_3d[0][14 * 3 + 2])/1600.0;
    }

    // return the 3D points directory
    memcpy(joint3d, joints_3d[0], sizeof(double)*3*joint_num);
    memcpy(joint2d, joints_2d[0], sizeof(double)*2*joint_num);
    // change the _time_stamp;
    // Calculate the joint angles (I do this as vector).
    double tmp_angles[3*(joint_num-1)];
    for (int i=0; i < joint_num - 1; ++i) {
        int posa = joint_indics[2 * i + 1];
        int posb = joint_indics[2 * i];
        double bone_length = std::sqrt(
                std::pow(joints_3d[0][posa*3 + 0] - joints_3d[0][posb*3 + 0],2) + \
                std::pow(joints_3d[0][posa*3 + 1] - joints_3d[0][posb*3 + 1],2) + \
                std::pow(joints_3d[0][posa*3 + 2] - joints_3d[0][posb*3 + 2],2));
        tmp_angles[3*i + 0] = 180.0 * std::acos((joints_3d[0][posa*3 + 0]-joints_3d[0][posb*3 + 0])/bone_length) / M_PI;
        tmp_angles[3*i + 1] = 180.0 * std::acos((joints_3d[0][posa*3 + 1]-joints_3d[0][posb*3 + 1])/bone_length) / M_PI;
        tmp_angles[3*i + 2] = 180.0 * std::acos((joints_3d[0][posa*3 + 2]-joints_3d[0][posb*3 + 2])/bone_length) / M_PI;
    }
    // then give the angles to joint_angles;
    // vector is already deep copy
    if (!_is_first_frame) {
        memcpy(joint_angles[2], joint_angles[1], sizeof(double) * (joint_num-1)*3);
        memcpy(joint_angles[1], joint_angles[0], sizeof(double) * (joint_num-1)*3);
    }
    memcpy(joint_angles[0], tmp_angles, sizeof(double) * (joint_num-1)*3);
    if (_is_first_frame) {
        memcpy(joint_angles[1], tmp_angles, sizeof(double) * (joint_num-1)*3);
        memcpy(joint_angles[2], tmp_angles, sizeof(double) * (joint_num-1)*3);
        // initialize the global_d.
        for(int i=0; i < 3; ++i) {
            global_d[i][0] = 0;
            global_d[i][1] = 0;
            global_d[i][2] = 0;
        }
    }
    // TODO: It's very strange, the global_d's z never changed! It's not what I think! 
    double start, end;
    start = clock();
    mFitting::fitting(joints_2d, joints_3d, mvp, joint_angles, global_d[0]);
    end = clock();
    std::cout << "Fitting time:" << (end-start)/CLOCKS_PER_SEC << std::endl;
    cal_3dpoints(joint_angles[0], global_d[0], joint3d);
    // then fitting!
    // after this, you need to fitting it using the energy function.
    if (_is_first_frame) {
        _is_first_frame = false;
    }
}
