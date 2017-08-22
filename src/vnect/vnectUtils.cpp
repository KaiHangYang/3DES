#include "../../include/vnectUtils.hpp"
#include "../../include/vnectJointsInfo.hpp"


mVNectUtils::mVNectUtils(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path):mCaffePredictor(model_path, deploy_path, mean_path) {
    _is_tracking = false;
    _box_size = cv::Size(0, 0);
    _crop_size = 368;
    _crop_scale = 1.0;
}
void mVNectUtils::wrapInputLayer(std::vector<cv::Mat> * input_data) {
    // Before call this function the net has been reshaped
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    input_data->empty(); 
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
    for (int i=0; i < _scales.size(); ++i) {
        // put all the scaled img to the data
        cv::resize(tmp_float, tmpScaleImg, cv::Size(0, 0), _scales[i], _scales[i]);
        data.push_back(padImage(tmpScaleImg, _box_size));
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
std::vector<float> mVNectUtils::predict(const cv::Mat &img) {
    cv::Mat tmp;
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    _num_channel = img.channels();
    // Here according to the demo, the image is resized to [448, 848]
    cv::resize(img, tmp, cv::Size(848, 448));
    _input_size = tmp.size();
    if (_is_tracking) {
        // in the tracking mode the scale is 2 the the num of the pic is 2
        _scales.empty();
        for (int i=0; i < 2; ++i) {
            _scales.push_back(1.0 - 0.3 * static_cast<float>(i));
        }
        input_layer->Reshape(2, static_cast<int>(_num_channel), tmp.size().height, tmp.size().width);
    }
    else {
        _scales.empty();
        for (int i=0; i < 1; ++i) {
            _scales.push_back(1.0 - 0.2 * static_cast<float>(i));
        }
        input_layer->Reshape(1, static_cast<int>(_num_channel), tmp.size().height, tmp.size().width);
    }
    
    _net->Reshape();

    std::vector<cv::Mat> input_data;

    wrapInputLayer(&input_data);

    preprocess(tmp, &input_data);

    _net->Forward();
    // There have been 4 blobs in the output the for confidence map is contained in them.
    caffe::Blob<float> * output_layer = _net->output_blobs()[0];
    std::cout << output_layer->num() << " " << output_layer->height() << " " << output_layer->width() << " " << output_layer->channels() << std::endl;
    const float * begin = output_layer->cpu_data();
    const float * end = begin + output_layer->width() * output_layer->height() * output_layer->channels();
    return std::vector<float>(begin, end);
    //caffe::Blob<float> * output_layer = _net->output_blobs()[0];
}
