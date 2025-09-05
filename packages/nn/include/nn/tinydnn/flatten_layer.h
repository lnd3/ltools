#pragma once

#include <vector>

#include <tiny_dnn/tiny_dnn.h>

namespace l::nn::tinydnn {

    using namespace tiny_dnn;

    class flatten_layer : public layer {
    public:
        flatten_layer(size_t in_height, size_t in_width)
            : layer({ vector_type::data }, { vector_type::data }),
            in_height_(in_height), in_width_(in_width), out_size_(in_height* in_width) {}

        std::string layer_type() const override { return "flatten"; }

        virtual void forward_propagation(const std::vector<tensor_t*>& in_data,
            std::vector<tensor_t*>& out_data) override {
            const tensor_t& input = *(in_data[0]);
            tensor_t& output = *(out_data[0]);
            for (size_t i = 0; i < input.size(); ++i) {
                output[i].resize(out_size_);
                for (size_t h = 0; h < in_height_; ++h) {
                    for (size_t w = 0; w < in_width_; ++w) {
                        output[i][h * in_width_ + w] = input[i][h * in_width_ + w];
                    }
                }
            }
        }

        virtual void back_propagation(const std::vector<tensor_t*>& in_data,
            const std::vector<tensor_t*>& out_data,
            std::vector<tensor_t*>& out_grad,
            std::vector<tensor_t*>& in_grad) override {
            const tensor_t& out_gradient = *(out_grad[0]);
            tensor_t& in_gradient = *(in_grad[0]);
            for (size_t i = 0; i < out_gradient.size(); ++i) {
                in_gradient[i].resize(in_height_ * in_width_);
                for (size_t h = 0; h < in_height_; ++h) {
                    for (size_t w = 0; w < in_width_; ++w) {
                        in_gradient[i][h * in_width_ + w] = out_gradient[i][h * in_width_ + w];
                    }
                }
            }
        }

        std::vector<index3d<size_t>> in_shape() const override {
            return { {in_height_, in_width_, 1} };
        }

        std::vector<index3d<size_t>> out_shape() const override {
            return { {out_size_, 1, 1} };
        }

    private:
        size_t in_height_, in_width_, out_size_;
    };

} // 