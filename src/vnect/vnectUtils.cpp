#include "../../include/vnectUtils.hpp"
#include "../../include/vnectJointsInfo.hpp"
#include <algorithm>

mVNectUtils::mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path):mCaffePredictor(model_path, deploy_path, mean_path) {
    _is_tracking = false;
    _box_size = cv::Size(0, 0);
    _crop_size = 368;
    _crop_scale = 1.0;
    _hm_factor = 8.0;
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
        _box_size = cv::Size(img.size().width, img.size().height);
        _is_tracking = true;
    }
    else {
        _box_size = cv::Size(_crop_size, _crop_size);
        float crop_offset = static_cast<int>(40.0/_crop_scale);
        // TODO the bbt need to be implement 
        // Now I just do the firt frame predict
    }

    // Once the crop is known, process the img
    // the data type as the blob 
    std::vector<cv::Mat> data;
    cv::Mat tmp_resize;
    // TODO:the input_size must be updated after the net is reshaped
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
std::vector<std::vector<int> > mVNectUtils::predict(const cv::Mat &img) {
    cv::Mat tmp;
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    _num_channel = img.channels();
    // Here according to the demo, the image is resized to [448, 848]
    cv::resize(img, tmp, cv::Size(424, 224));
    _input_size = tmp.size();
    if (_is_tracking) {
        // In tracking mode, the number of scales is 2, I don't know why, 
        // but this number can be changed.
        _scales.clear();
        for (int i=0; i < 2; ++i) {
            _scales.push_back(1.0 - 0.3 * static_cast<float>(i));
        }
        input_layer->Reshape(2, static_cast<int>(_num_channel), tmp.size().height, tmp.size().width);
    }
    else {
        // The multi-scales in the paper means, we need to scale the image to multi scale(eg 1.0, 0.8, 0.6). Then forward 
        // to get the average of the results of all scales.
        // The implement in the demo is 3 scales. Here my gpu's memory is not 
        // enough, so I use one scale to test.
        _scales.clear();
        for (int i=0; i < 3; ++i) {
            _scales.push_back(1.0 - 0.2 * static_cast<float>(i));
        }
        input_layer->Reshape(3, static_cast<int>(_num_channel), tmp.size().height, tmp.size().width);
    }
    
    _net->Reshape();

    std::vector<cv::Mat> input_data;

    wrapInputLayer(&input_data);

    preprocess(tmp, &input_data);

    _net->Forward();
    // There have been 4 blobs in the output the for confidence map is contained in them.
    std::vector<caffe::Blob<float> *> output_layer = _net->output_blobs();
    std::vector<std::vector<cv::Mat> > result;
    
    int o_width = output_layer[0]->width();
    int o_height = output_layer[0]->height();
    int o_channels = output_layer[0]->channels();
    int o_num = output_layer[0]->num();
    cv::Size hm_size = cv::Size(o_width, o_height);
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
    // them all the heatmaps is ready for calculate the 2D and 3D location
    // clear the joints stored in the vector
    joints_2d.clear();
    joints_3d.clear();
    for (int i=0; i < o_channels; ++i) {
        std::vector<int> p2({0, 0});
        std::vector<int> p3({0, 0, 0});
        
        cv::Mat hm;
        cv::resize(heatmaps[i], hm, _box_size);
        //cv::imshow("testaa", hm);
        //cv::waitKey();
        cv::minMaxIdx(hm, nullptr, nullptr, nullptr, &p2[0]);
        std::cout << "pos2d:" << p2[0] << ',' << p2[1] << std::endl;
        int posx = std::max(static_cast<int>(p2[0]/_hm_factor), 1);
        int posy = std::max(static_cast<int>(p2[1]/_hm_factor), 1);
        // Here, what you get is not the true (x, y, z), you need to minus the root joint 14
        p3[0] = 100 * xmaps[i].at<float>(posx, posy);
        p3[1] = 100 * ymaps[i].at<float>(posx, posy);
        p3[2] = 100 * zmaps[i].at<float>(posx, posy);
        //std::cout << "pos3d:" << p3[0] << ',' << p3[1] << ", " << p3[2]<< std::endl;
        joints_2d.push_back(p2);
        joints_3d.push_back(p3);
    }
    // TODO The 3D position seem a little strange
    for (int i=0; i < o_channels; ++i) {
        joints_3d[i][0] -= joints_3d[14][0];
        joints_3d[i][1] -= joints_3d[14][1];
        joints_3d[i][2] -= joints_3d[14][2];
        std::cout << "pos3d:" << joints_3d[i][0] << ',' << joints_3d[i][1] << ", " << joints_3d[i][2]<< std::endl;
    }

    
    return joints_2d;
}