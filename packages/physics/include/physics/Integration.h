#pragma once

#include "logging/Log.h"
#include "Constants.h"
#include "Algorithm.h"

namespace l {
namespace physics {

	template <class T, class V>
	void attractionFriction(T& p1, T& p2, T& v1, T& v2, V friction, V dt) {
		//T vdiff = (v1 - v2) * dt;
		T dir = p1 - p2 /*+ vdiff*/;
		V length = dir.length();
		if (length > 0.0001) {
			T n = dir / length;
			T vdiff = (v1 - v2);
			V t = -vdiff.dot(n);

			friction = 1.0 - friction;

			v1 += n * t * 0.5 * friction;
			v2 -= n * t * 0.5 * friction;
		}
	}

	enum ForceType {
		Const,
		Linear,
		Square,
		Cube,
		Power4,
		Power5,
		InverseLinear,
		InverseSquare,
		InverseCube,
		InversePower4,
		InversePower5

	};

	template <class T>
	T isTypeInverse(ForceType type) {
		switch (type) {
		case Const:
		case Linear:
		case Square:
		case Cube:
		case Power4:
		case Power5:
			return 0;
		case InverseLinear:
		case InverseSquare:
		case InverseCube:
		case InversePower4:
		case InversePower5:
			return 1;
		default:
			return 0;
		}
	}

	template <class T>
	T getTypePower(ForceType type) {
		switch (type) {
		case Const:
			return 0.0;
		case InverseLinear:
		case Linear:
			return 1.0;
		case InverseSquare:
		case Square:
			return 2.0;
		case InverseCube:
		case Cube:
			return 3.0;
		case InversePower4:
		case Power4:
			return 4.0;
		case InversePower5:
		case Power5:
			return 5.0;
		default:
			return 0.0;
		}
	}

	template <class V>
	V computeForceLambda(V r, V rDesired, V forceConstant, ForceType type) {
		V epsilon = 1.0;
		V epsilon2 = 1e-40;
		V expansionLimit = 0.0001;

		V power = 0.0;
		V lambda = 0.0;

		switch (type) {
		case Const:
			power = 0.0;
			break;
		case InverseLinear:
		case Linear:
			power = 1.0;
			break;
		case InverseSquare:
		case Square:
			power = 2.0;
			break;
		case InverseCube:
		case Cube:
			power = 3.0;
			break;
		case InversePower4:
		case Power4:
			power = 4.0;
			break;
		case InversePower5:
		case Power5:
			power = 5.0;
			break;
		}
		if (rDesired < expansionLimit) {
			rDesired = expansionLimit;
		}

		V distance = 1.0 + abs(r - rDesired);
		lambda = distance;
		if (distance < expansionLimit) {
			distance = expansionLimit;
		}

		if (power == 0.0) {
			lambda = rDesired - r > 0 ? 1 : -1;
		}
		else {
			lambda = - pow(lambda, power);
		}

		lambda *= 1.0 / forceConstant;
		V forceLimit = static_cast<V>(1.0 / epsilon2);
		if (lambda > forceLimit) {
			LOG(LogInfo) << "Force limit exceeded: " << lambda;
			//lambda = forceLimit;
		}
		if (lambda < -forceLimit) {
			LOG(LogInfo) << "Force limit exceeded: " << lambda;
			//lambda = -forceLimit;
		}

		switch (type) {
		case InverseLinear:
		case InverseSquare:
		case InverseCube:
		case InversePower4:
		case InversePower5:
			if (abs(lambda) <= expansionLimit) {
				//lambda = lambda >= 0.0 ? expansionLimit : -expansionLimit;
			}
			lambda = 1.0 / (lambda);
			break;
		default:
			break;
		}
		if (distance < rDesired) {
			//lambda = lambda * pow(distance / rDesired, 0.25);
		}
		V freq = 2.25;

		V gravity = - 1.0 / (1 + pow(r * freq, power));
		V modulation = (cos(l::constants::pi * (1 + 2 * r * 0.1 * freq)) * 0.25 + 0.75);
		V cutoff = (1.0 - exp(-r));

		//V mod = cos(l::constants::pi * (1.0 + 2 * distance * freq)) * 0.5 + 0.5;
		//return lambda * mod;

		return gravity * modulation * cutoff;
	}

	template <class V>
	V computeForceLambda2(V r, V forceConstant, V power) {
		V freq = 1.0;
		V gravity = -power * forceConstant / (pow(1 + r, power));
		V modulation = (cos(l::constants::pi * (1 + 2 * r * freq * power)) * 0.25 + 0.75);
		V cutoff = (1.0 - exp(-r));

		return gravity * modulation * cutoff;
	}

	template <class V>
	V computeForceLambda3(V r, V forceConstant, V power, V size) {
		return - 0.1*forceConstant * 2.0 * sqrt(size) * r / (size + power * r * r);
	}

	template <class V>
	V computeForceLambda4(V r, V forceConstant, V power, V size) {
		if (power < 2.0) {
			return -forceConstant * 2.0 * sqrt(size) * r / (size + power * r * r);
		}
		return -forceConstant*(power / (power - 1.0)) * pow((power - 1.0) * size, 1.0 / power) * r / (1.0 + size * pow(r, power));
	}

	template <class T, class V>
	T computeReactionImpulse(T directionP1MinusP2, V rDesired, V forceConstant, ForceType type) {
		V rSquared = directionP1MinusP2.sqr();
		V r = sqrt(rSquared);
		V e = 0.00000001;
		if (r < e) {
			return {};
		}
		T dirNorm = directionP1MinusP2 / r;
		V lambda = computeForceLambda4(r, forceConstant, getTypePower<V>(type), 1.125);
		return dirNorm * lambda;
	}

	template <class T, class V>
	V computePotentialEnergy(T& p1, T& p2, V m1, V m2, V rDesired, V forceConstant, ForceType type, int iterations) {
		T dir = p1 - p2;
		V rSquared = dir.sqr();
		if (rSquared <= 0.0) {
			return {};
		}
		V r = sqrt(rSquared);
		V distance = rDesired - r;
		V rSampleDelta = distance / iterations;
		V energy = 0.0;
		V v1 = 0.0;
		V v2 = 0.0;
		auto s = abs(rSampleDelta);

		for (int i = 0; i < iterations; i++) {
			V f = abs(computeForceLambda(r, rDesired, forceConstant, type));

			if (f >= 0.00001) {
				auto x1 = l::algorithm::solve_quad_eq(m1 * 2.0 * v1 / f, -2.0 * m1 * s / f);
				if (x1) {
					auto dt = x1->first > 0.0 ? x1->first : x1->second;
					v1 += f * dt / m1;
				}
				auto x2 = l::algorithm::solve_quad_eq(m2 * 2.0 * v2 / f, -2.0 * m2 * s / f);
				if (x2) {
					auto dt = x2->first > 0.0 ? x2->first : x2->second;
					v2 += f * dt / m2;
				}
			}
			r += rSampleDelta;
		}
		energy += v1 * v1 * m1 * 0.5;
		energy += v2 * v2 * m2 * 0.5;

		return energy * 0.5;
	};

	template <class T, class V>
	V computePotentialEnergy2(T& p1, T& p2, V m1, V m2, V rDesired, V forceConstant, ForceType type, int iterations) {
		T dir = p1 - p2;
		V rSquared = dir.sqr();
		if (rSquared <= 0.0) {
			return {};
		}
		V r = sqrt(rSquared);
		V distance = rDesired - r;
		V rSampleDelta = distance / iterations;
		V energy = 0.0;
		V s = abs(rSampleDelta);

		for (int i = 0; i < iterations; i++) {
			V f = abs(computeForceLambda(r, rDesired, forceConstant, type));
			if (f >= 0.0000001) {
				V dt1 = sqrt(s * 2.0 * m1 / f);
				V dt2 = sqrt(s * 2.0 * m2 / f);
				V v1 = s / dt1;
				V v2 = s / dt2;
				V e1 = v1 * v1 * m1;
				V e2 = v2 * v2 * m2;
				energy += e1 + e2;
			}
			r += rSampleDelta;
		}

		return energy;
	};

	template <class T, class V>
	V computePotentialEnergy3(T& p1, T& p2, V m1, V m2, V rDesired, V forceConstant, ForceType type, int iterations) {
		T dir = p1 - p2;
		V rSquared = dir.sqr();
		if (rSquared <= 0.0) {
			return {};
		}
		V r = sqrt(rSquared);
		V distance = rDesired - r;
		V rSampleDelta = distance / iterations;
		V energy = 0.0;
		V s = abs(rSampleDelta);

		for (int i = 0; i < iterations; i++) {
			V f = abs(computeForceLambda(r, rDesired, forceConstant, type));
			if (f >= 0.000001) {
				energy += f * s;
			}
			r += rSampleDelta;
		}
		V m = m1 > m2 ? m1 : m2;
		return energy * m / (m1 + m2);
	};

	template <class T, class V>
	V computePotentialEnergy4(T& p1, T& p2, V m1, V m2, V rDesired, V forceConstant, ForceType type, int iterations) {
		T dir = p1 - p2;
		V rSquared = dir.sqr();
		if (rSquared <= 0.0) {
			return {};
		}
		V r = sqrt(rSquared);
		V distance = rDesired - r;
		V rSampleDelta = distance / iterations;
		V energy = 0.0;
		V s = abs(rSampleDelta);

		for (int i = 0; i < iterations; i++) {
			V f = abs(computeForceLambda(r, rDesired, forceConstant, type));
			if (f >= 0.000001) {
				energy += f * s;
			}
			r += rSampleDelta;
		}
		V m = m1 > m2 ? m1 : m2;
		return energy * m / (m1 + m2);
	};

	template <class T, class V>
	V computeKineticEnergy(T&& v, V m) {
		return (v / m).sqr() * m * 0.5;
	};

	template <class T, class V>
	V computePotentialEnergyX2(T& distance, V totalMass, V k, ForceType type) {
		V dist = distance.length() + 1.0;
		switch (type) {
		case Const:
			return totalMass * (1.0 - 1.0 / dist);
			break;
		case InverseLinear:
		case Linear:
			return totalMass * (1.0 - 1.0 / (dist* dist));
			break;
		case InverseSquare:
		case Square:
			return totalMass * (1.0 - 1.0 / (dist* dist* dist));
			break;
		case InverseCube:
		case Cube:
			return totalMass * (1.0 - 1.0 / (dist * dist* dist* dist));
		case InversePower4:
		case Power4:
			return k * totalMass * (1.0 - 1.0 / (dist * dist * dist));
			break;
		case InversePower5:
		case Power5:
			return k * totalMass * (1.0 - 1.0 / (dist * dist * dist * dist));
			break;
		}
	};

	template <class T, class V>
	V computeKineticEnergy2(T& p1, T& p2, T& v1, T& v2, V m1, V m2, V dt) {
		T dir = p2 - p1;
		V rSquared = dir.sqr();
		if (rSquared <= 0.000000001) {
			dir = (v1*m1 + v2*m2) / (m1 + m2);
			rSquared = dir.sqr();
		}
		V r = sqrt(rSquared);
		T dirNorm = dir / r;

		V v1Norm = dirNorm.dot(v1*dt);
		V v2Norm = dirNorm.dot(v2*dt);
		V vDiff = v2Norm - v1Norm; // vDiff > 0 particles moves apart
		V vInvariant1 = - vDiff * m2 / (m1 + m2);
		V vInvariant2 = vDiff * m1 / (m1 + m2);
		V energy = (vInvariant1 * vInvariant1 * m1 + vInvariant2 * vInvariant2 * m2) * 0.5;
		return energy / dt;
	};

	template <class T, class V>
	bool isPassingPole(const T& pPrev, const T& pNow, const T& vPrev, const T& vNow, const T& polePosPrev, const T& polePosNow, const T& poleVelPrev, const T& poleVelNow) {
		T dir0 = pPrev - polePosPrev;
		T dir1 = pNow - polePosNow;
		V dirFlip = dir0.dot(dir1);
		if (dirFlip < 0.0) {
			//return true;
		}
		V dot0 = (vPrev - poleVelPrev).dot(dir0); // particles are closing in when dot is negative
		V dot1 = (vNow - poleVelNow).dot(dir1);
		if (dot0 < 0.0 && dot1 > 0.0) {
			return true;
		}
		return false;
	}

	/*
	Recommended use when:
	 * velocity changes from moving towards the pole to away from the pole
	 * it computes the conjugate position along the direction vector
	*/
	template <class T, class V>
	T&& computeSymplecticAdjustment2(const T& pPrev, const T& pNow, const T& polePosPrev, const T& polePosNow) {
		T v = pNow - pPrev;
		V vr = sqrt(v.sqr());
		T n = v / vr;
		T dir0 = pPrev - polePosPrev;
		V t = -dir0.dot(n);

		if (t < 0.5*vr) {
			t = 0.5*vr;
		}
		if (t > 2.0 * vr) {
			t = 2.0 * vr;
		}

		return n * t;
	}

	template <class T, class V>
	void computeSymplecticAdjustment(const T& pPrev, T& pNow, const T& polePosPrev, T& polePosNow) {
		T v = pNow - pPrev + polePosPrev - polePosNow;
		V vr = sqrt(v.sqr());
		T n = v / vr;
		T dir0 = pPrev - polePosPrev;
		T dir1 = pNow - polePosNow;
		V t0 = -dir0.dot(n);
		V t1 = -dir1.dot(n);

		pNow = pPrev + n * t0 * static_cast<V>(1);
		polePosNow = polePosPrev - n * t0 * static_cast<V>(1);
	}

	template <class T>
	T reflect(const T& vec, const T& dir) {
		auto t = vec.dot(dir) * 2.0;
		auto dirStep = dir * t / dir.sqr();
		return vec - dirStep;
	};

	template <class T>
	T lineDistVec(const T& vecPos, const T& vecDir, const T& pos) {
		auto a = vecPos - pos;
		auto t = a.dot(vecDir);
		auto b = vecDir * t / vecDir.sqr();
		return a - b;
	};

	template <class VectorType>
	auto detectPole2(
		const VectorType& p0,
		const VectorType& v0dt,
		const VectorType& a0dtdt,
		const VectorType& p1,
		const VectorType& v1dt,
		const VectorType& a1dtdt)
	{
		auto e = 0.0000000001;
		auto dvNext = v0dt - v1dt;
		auto dvNextSqr = dvNext.sqr();
		if (dvNextSqr <= e) {
			return -1.0;
		}
		auto dp = p0 + v0dt - p1 - v1dt;
		auto dpDist = dp.sqr();
		if (dpDist <= e) {
			return -1.0;
		}
		auto tMidpoint = -dp.dot(dvNext) / dvNextSqr;
		auto daNext = a0dtdt - a1dtdt;
		auto f2 = dp.dot(daNext);
		if (tMidpoint > 0.0 && tMidpoint < 1.0 - e && f2 < 0.0) {
			return tMidpoint;
		}
		return -1.0;
	}

	template<class ScalarType, class VectorType>
	using PoleResolver = std::function<void(
		const VectorType& p0Change,
		const VectorType& p1Change,
		const VectorType& mv0dtChange,
		const VectorType& mv1dtChange
		)>;

	template <class VectorType, class ScalarType>
	bool detectPole(
		const VectorType& p0, 
		const VectorType& v0dt, 
		const VectorType& a0dtdt,
		const VectorType& p1, 
		const VectorType& v1dt, 
		const VectorType& a1dtdt,
		ScalarType dt,
		PoleResolver<ScalarType, VectorType> computePoleStateChange)
	{
		 // pair positional change estimate over time step
		auto dp0Next = v0dt + a0dtdt;
		auto dp1Next = v1dt + a1dtdt;

		// integrated velocity change estimate over time step
		auto dvNext = dp0Next - dp1Next;

		// acceleration change estimate over time step
		auto daNext = a0dtdt - a1dtdt;

		auto e = 0.0000000001;
		auto dvNextSqr = dvNext.sqr();
		if (dvNextSqr <= e) {
			return false; 
		}

		// positional direction of constraint pair
		auto dp = p0 + v0dt - p1 - v1dt;

		auto dpDist = dp.sqr();
		if (dpDist <= e) {
			return false;
		}

		// distance to midpoint along integrated velocity estimate
		auto tMidpoint = -dp.dot(dvNext) / dvNextSqr;

		// second derivate is used to exclude the distance maxima solution
		auto f2 = dp.dot(daNext);

		// step symmetrically over the pole and accumulate the position change separatively for all pairs
		// by linearizing all forces acting on the pair, we can step symmetrically over the pole in one go
		// 1) detect if we are going to pass the pole during this time step
		// 2) linearize acceleration so we can compute positional change and impulse over the pole

		if (tMidpoint > e && tMidpoint < 1.0 - e) {
			if (f2 < 0.0) {
				dp0Next *= tMidpoint;
				dp1Next *= tMidpoint;
				auto p0Midpoint = p0 + dp0Next;
				auto p1Midpoint = p1 + dp1Next;

				// we are now at the midpoint of the pair's center of mass over this time step 
				// we can now compute the direction of the acceleration at this point and
				// integrate velocity to be able to take one more partial step
				auto dpMidpoint = p0Midpoint - p1Midpoint;
				auto dpMidpointLength = dpMidpoint.length();

				if (dpMidpointLength <= e) {
					return false;
				}


				// move to symmetric position
				auto p0Change = dp0Next * tMidpoint;
				auto p1Change = dp1Next * tMidpoint;

				auto v0dtChange = -p0Change * dt * 0.0;
				auto v1dtChange = -p1Change * dt * 0.0;

				// add acceleration projection from midpoint direction twice to simulate symmetric forces
				// this should translate into velocity adjustment 
				auto a0dtdtChange = dpMidpoint * a0dtdt.dot(dpMidpoint) * (1.0 / dpMidpointLength) - p0Change * dt * dt * 0.5;
				auto a1dtdtChange = dpMidpoint * a1dtdt.dot(dpMidpoint) * (1.0 / dpMidpointLength) - p1Change * dt * dt * 0.5;


				computePoleStateChange(p0Change, p1Change, v0dtChange, v1dtChange);
				return true;
			}
		}
		return false;
	}

	template <class T, class V>
	void applyPotentialStabilization1(T& pOld0, T& p0, T& v0, T& pOld1, T& p1, T& v1, V dt) {
		T v = v0 * dt;
		T w = v1 * dt;
		T dV1 = v - w;

		if (true) {
			// reconstruct old position from current velocity to see if this has any effect
			pOld0 = p0 - v;
			pOld1 = p1 - w;
		}
		T dP0 = pOld1 - pOld0;
		V t = dP0.dot(dV1) / dV1.sqr();
		if (t >= 0 && t < 1.0) {
			p0 = pOld0 + v * t * 2;
			p1 = pOld1 + w * t * 2;

			// Error checking
			if (true) {
				T dP1 = p1 - p0;
				V dP0Sqr = dP0.sqr();
				V dP1Sqr = dP1.sqr();
				if (abs(dP0Sqr - dP1Sqr) > 0.00001) {
					LOG(LogError) << "Something went wrong";
				}
			}
		}
	}

	template <class T, class V>
	void applyPotentialStabilization2(T& p0, T& p1, T& v1, T& polePos0, T& polePos1, T& poleVel1, V dt) {
		T v = v1 * dt;
		T w = poleVel1 * dt;

		T dP0 = polePos0 - p0;
		V v1Sqr = v.sqr();
		V v1v2 = v.dot(w);

		V c1 = v.dot(dP0);
		V c2 = w.dot(dP0);

		if (v1Sqr != 0 && v1v2 - v1Sqr != 0) {
			V s = (c2 * v1Sqr - c1) / (v1v2 - v1Sqr);
			V r = (c1 + s * v1v2) / v1Sqr;

			if (s >= 0.0 && s <= 1.0 || r >= 0.0 && r <= 1.0) {
				if (s < 0.0 || s > 1.0) {
					s = r;
				}
				p1 = p0 + v * s * 2;
				polePos1 = polePos0 + w * s * 2;

				// Error checking
				if (true) {
					T dP1 = polePos1 - p1;
					V dP0Sqr = dP0.sqr();
					V dP1Sqr = dP1.sqr();
					if (abs(dP0Sqr - dP1Sqr) > 0.01) {
						LOG(LogError) << "Something went wrong";
					}
				}
			}
		}
	}

	template <class T, class V>
	void applyPotentialStabilization3(T& p0, T& p1, T& v1, T& polePos0, T& polePos1, T& poleVel1, V dt) {
		auto d = (poleVel1 - v1) * dt;
		auto dSqr = d.sqr();
		if (false && dSqr > 0) {
			T p = polePos1 - p1;
			V j = (-2.0 * p.sqr() - p.dot(d)) / dSqr;
			if (j > 0 && j <= 1) {
				p1 = p0 + v1 * j * dt * 2;
				polePos1 = polePos0 + poleVel1 * j * dt * 2;
				{// reflect previous velocity to be able to orbit pole with zero potential energy change
					//auto n = lineDistVec(p0, v1, polePos0);
					//v1 = reflect(v1, n);
					//poleVel1 = reflect(poleVel1, n);
				}
			}
		}
	}

	template <class T>
	void forwardEuler(const T& p0, T& p1, T& v1, double dt) {
		p1 = p0 + v1 * dt;
	};

	template <class T>
	void eulerStep(const T& p0, T& p1, T& v1, double dt) {
		p1 = p0 + v1 * dt;
	};
}
}
