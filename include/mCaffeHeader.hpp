#ifndef MCAFFE_HEADER
#define MCAFFE_HEADER
/* Include the needed caffe headers and register the layers */
#include <caffe/common.hpp>
#include <caffe/layer.hpp>
#include <caffe/layer_factory.hpp>
#include <caffe/layers/input_layer.hpp>
#include <caffe/layers/inner_product_layer.hpp>
#include <caffe/layers/dropout_layer.hpp>
#include <caffe/layers/conv_layer.hpp>
#include <caffe/layers/relu_layer.hpp>
#include <caffe/layers/pooling_layer.hpp>

// Regist the required the layer 
namespace caffe {
    extern INSTANTIATE_CLASS(InputLayer);
    extern INSTANTIATE_CLASS(InnerProductLayer);
    extern INSTANTIATE_CLASS(DropoutLayer);
    extern INSTANTIATE_CLASS(ConvolutionLayer);
    extern INSTANTIATE_CLASS(ReLULayer);
    extern INSTANTIATE_CLASS(PoolingLayer);
}


#endif
