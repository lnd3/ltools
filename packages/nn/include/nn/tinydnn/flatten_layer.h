#pragma once

#include <vector>

#include <tiny_dnn/tiny_dnn.h>

namespace l::nn::tinydnn {

    using namespace tiny_dnn;

    class flatten_layer : public layer {
    public:
        flatten_layer(size_t in_height, size_t in_width, size_t in_channels = 1)
            : layer({ vector_type::data }, { vector_type::data }),
            in_height_(in_height), in_width_(in_width), in_channels_(in_channels),
            out_size_(in_height* in_width* in_channels) {
        }

        std::string layer_type() const override { return "flatten"; }

        void forward_propagation(const std::vector<tensor_t*>& in_data,
            std::vector<tensor_t*>& out_data) override {
            const tensor_t& input = *(in_data[0]);
            tensor_t& output = *(out_data[0]);
            for (size_t i = 0; i < input.size(); ++i) {
                output[i].resize(out_size_);
                size_t idx = 0;
                for (size_t c = 0; c < in_channels_; ++c) {
                    for (size_t h = 0; h < in_height_; ++h) {
                        for (size_t w = 0; w < in_width_; ++w) {
                            output[i][idx++] = input[i][c * in_height_ * in_width_ + h * in_width_ + w];
                        }
                    }
                }
            }
        }

        void back_propagation(const std::vector<tensor_t*>& in_data,
            const std::vector<tensor_t*>& out_data,
            std::vector<tensor_t*>& out_grad,
            std::vector<tensor_t*>& in_grad) override {
            const tensor_t& out_gradient = *(out_grad[0]);
            tensor_t& in_gradient = *(in_grad[0]);
            for (size_t i = 0; i < out_gradient.size(); ++i) {
                in_gradient[i].resize(out_size_);
                size_t idx = 0;
                for (size_t c = 0; c < in_channels_; ++c) {
                    for (size_t h = 0; h < in_height_; ++h) {
                        for (size_t w = 0; w < in_width_; ++w) {
                            in_gradient[i][c * in_height_ * in_width_ + h * in_width_ + w] =
                                out_gradient[i][idx++];
                        }
                    }
                }
            }
        }

        std::vector<index3d<size_t>> in_shape() const override {
            return { {in_height_, in_width_, in_channels_} };
        }

        std::vector<index3d<size_t>> out_shape() const override {
            return { {out_size_, 1, 1} };
        }

    private:
        size_t in_height_, in_width_, in_channels_, out_size_;
    };

} // 