#pragma once

#include <cmath>
#include "VecX.h"
#include "MathSmooth.h"


namespace l {
namespace curves {


	template<class T = double, class V = vec::Data4<T>>
	class Bezier {
	public:
		Bezier() {}

		void addPoint(const V& vec) {
			points.push_back(vec);
		}

		void dropPoint() {
			points.erase(points.begin());
		}

		/*
		 t [1 - points.size(), 0] - the time step from current state

		
		*/
		V sampleAt(T t, size_t degree = 3) {
			V position;
			
			if (points.size() < 3) {
				degree = points.size();
			}

			auto it = points.end() - 3;
			for (; it < points.end();it++) {
				position += *it;
			}

			return position;
		}

	private:
		std::vector<V> points;
	};


}
}
