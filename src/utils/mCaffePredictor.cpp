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
    setMean(mean_path);
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
std::vector<float> mCaffePredictor::predict(const cv::Mat &img) {
    /* Reshape the net to fit for the input */
    caffe::Blob<float> * input_layer = _net->input_blobs()[0];
    input_layer->Reshape(1, static_cast<int>(_num_channel), _input_size.height, _input_size.width);
    /* Forward all dimension change to all layers */
    _net->Reshape();


}
