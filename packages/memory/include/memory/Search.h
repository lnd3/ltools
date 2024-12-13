#pragma once

#include <cmath>

namespace l::search {

	template <typename T>
	int BinarySearch(const T* arr, int l, int r, T x, int stride, int quantization) {
		if (r >= l) {
			int mid = l + (r - l) / 2;
			int x_round = quantization * static_cast<int>(0.5f + std::round(x) / quantization);
			int arr_round = quantization * static_cast<int>(std::round(arr[mid * stride]) / quantization);
			if (arr_round == x_round) {
				return mid;
			}
			int out = 0;

			if (arr_round > x_round) {
				out = BinarySearch(arr, l, mid - 1, x, stride, quantization);
			}
			else {
				out = BinarySearch(arr, mid + 1, r, x, stride, quantization);
			}
			if (out == -1) {
				if (arr_round < x_round) {
					out = BinarySearch(arr, l, mid - 1, x, stride, quantization);
				}
				else {
					out = BinarySearch(arr, mid + 1, r, x, stride, quantization);
				}
			}
			return out;
		}
		return -1;
	}

}
