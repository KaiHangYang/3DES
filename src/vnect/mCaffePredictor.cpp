#include "../../include/mCaffePredictor.hpp"

using namespace caffe;

mCaffePredictor::mCaffePredictor(const std::string &model_path, const std::string &deploy_path, const std::string &mean_path) {
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(0);
    
    /* Load the net and the weight */
    _net.reset(new caffe::Net<float>(deploy_path, caffe::TEST));
    _net->CopyTrainedLayersFrom(model_path);

    caffe::Blob<float> * input_data = _net->input_blobs()[0];
    
    _num_channel = static_cast<uint>(input_data->channels());
    _input_size = cv::Size(input_data->width(), input_data->height());

    /* Load the mean file */
    if (mean_path != "") {
        setMean(mean_path);
    }
}

void mCaffePredictor::setMean(const std::string &mean_path) {
    caffe::BlobProto blob_proto;
    caffe::ReadProtoFromBinaryFileOrDie(mean_path.c_str(), &blob_proto);
    
    /* Create blob from blob_proto */
    caffe::Blob<float> mean_blob;
    mean_blob.FromProto(blob_proto);
    CHECK_EQ(mean_blob.channels(), _num_channel) << "Mean file channels is not fit for the Net";
    std::vector<cv::Mat> channels;
    float * data = mean_blob.mutable_cpu_data();

    for (uint i = 0; i < _num_channel; ++i) {
        /* The blob store data in the formate (N, C, H, W) row main */
        cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        channels.push_back(channel);
        data += mean_blob.height() * mean_blob.width();
    }
    cv::Mat mean;
    /* merge the channels to a image. The reason why we do this is that the data store in mean.binaryproto is in the formate of caffe Blob */
    cv::merge(channels, mean);

    cv::Scalar channel_mean = cv::mean(mean);
    _mean = cv::Mat(_input_size, mean.type(), channel_mean);
    std::cout << "Mean file size: " << _mean.size << std::endl;
}
void mCaffePredictor::wrapInputLayer(std::vector<cv::Mat> * input_data) {
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float * data = input_layer->mutable_cpu_data();

    for (int i=0; i < input_layer->channels(); ++i) {
        /* This is set the element to the user allocated data */
        cv::Mat channel(height, width, CV_32FC1, data);
        input_data->push_back(channel);
        data += width*height;
    }

}
void mCaffePredictor::preprocess(const cv::Mat & img, std::vector<cv::Mat> * input_data) {
    /* conver the image to the size that fit for the net */
    cv::Mat tmp;
    if (img.channels() == 3 && _num_channel == 1) {
        cv::cvtColor(img, tmp, cv::COLOR_BGR2GRAY);
    }
    else if (img.channels() == 4 && _num_channel == 1) {
        cv::cvtColor(img, tmp, cv::COLOR_BGRA2GRAY);
    }
    else if (img.channels() == 4 && _num_channel == 3) {
        cv::cvtColor(img, tmp, cv::COLOR_BGRA2BGR);
    }
    else if (img.channels() == 1 && _num_channel == 3) {
        cv::cvtColor(img, tmp, cv::COLOR_GRAY2BGR);
    }
    else {
        tmp = img;
    }
    
    /* TODO:  This part of the image preprocess need to be changed  */
    cv::Mat tmp_resized;
    if (tmp.size() != _input_size) {
        cv::resize(tmp, tmp_resized, _input_size);
    }
    else {
        tmp_resized = tmp;
    }
    
    cv::Mat tmp_float;

    if (_num_channel == 3) {
        tmp_resized.convertTo(tmp_float, CV_32FC3);
    }
    else {
        tmp_resized.convertTo(tmp_float, CV_32FC1);
    }

    cv::Mat tmp_normalized;
    cv::subtract(tmp_float, _mean, tmp_normalized);

    cv::split(tmp_normalized, *input_data);

    CHECK(reinterpret_cast<float *>(input_data->at(0).data) == _net->input_blobs()[0]->cpu_data()) << "Input channels are not wrapping the input layer of the network";

}
std::vector<float> mCaffePredictor::predict(const cv::Mat &img) {
    /* Reshape the net to fit for the input */
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    input_layer->Reshape(1, static_cast<int>(_num_channel), _input_size.height, _input_size.width);
    /* Forward all dimension change to all layers */
    _net->Reshape();

    /* create a vector and put the pointer in the new input_layer to the it */
    std::vector<cv::Mat> input_channels;
    wrapInputLayer(&input_channels);

    preprocess(img, &input_channels);
    /* Layer pointer mapped and put the image data in it. Then forward */
    _net->Forward();
    /* Copy the data out */
    caffe::Blob<float> * output_layer = _net->output_blobs()[0];
    const float * begin = output_layer->cpu_data();
    const float * end = begin + output_layer->channels(); /* Because the blob data format is (n, c, h, w) */

    return std::vector<float>(begin, end);
}
