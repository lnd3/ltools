#include "physics/Integration.h"

#include "physics/VecX.h"

#include "math/MathAlgorithm.h"

#include "testing/Test.h"

#define EPSILON 0.0000001

using namespace l;

TEST(Integration, Bisect) {

	double x = math::algorithm::bisect<double>(0.2, 5.0, EPSILON, 10, [](double x) -> double {
		return -1.0 + (x - 1.0) * (x - 1.0);
		});

	TEST_FUZZY(x, 2.0, 0.0001, "Failed to bisect square function");

	return 0;
}
TEST(Integration, Potential) {
	{
		double g = 9.81;
		vec::Data2<double> p1(100.0, 0.0);
		vec::Data2<double> p2(0.0, 0.0);
		double fallDistance = (p2 - p1).length();
		double fallTime = sqrt(fallDistance * 2.0 / g);
		double fallVelocity = g * fallTime; // wrong. velocity at end position is not median velocity

		for (int i = 0; i < 10; i++) {
			double distance = 10.0 + 10.0 * i;
			double pot = physics::computePotentialEnergy(p1, p2, 1.0, distance, 0.0, g, physics::ForceType::Const, 30);
			double pot2 = distance * g;
			//TEST_FUZZY(pot, pot2, EPSILON, "Energy calculations for constant force fields failed");

		}
	}
	return 0;
}

TEST(Integration, Potential2) {
	{
		double g = 9.81;
		vec::Data2<double> p1(100.0, 0.0);
		vec::Data2<double> p2(0.0, 0.0);
		double fallDistance = (p2 - p1).length();
		double fallTime = sqrt(fallDistance * 2.0 / g);
		double fallVelocity = g * fallTime; // wrong. velocity at end position is not median velocity
		auto v1 = vec::Data2<double>(fallVelocity, 0.0);
		auto v2 = vec::Data2<double>();

		double pot1 = physics::computePotentialEnergy(p1, p2, 1.0, 100.0, 0.0, g, physics::ForceType::Const, 30);
		double kin1 = physics::computeKineticEnergy(vec::Data2<double>(fallVelocity, 0.0), 1.0);
		//TEST_FUZZY(pot1, kin1, EPSILON, "Energy calculations for constant force fields failed");

		double pot2 = physics::computePotentialEnergy2(p1, p2, 1.0, 100.0, 0.0, g, physics::ForceType::Const, 30);
		//TEST_FUZZY(pot2, 981.0, EPSILON, "Energy calculations for constant force fields failed");

		double pot3 = physics::computePotentialEnergy3(p1, p2, 1.0, 2.0, 0.0, g, physics::ForceType::Const, 30);
		double kin2 = physics::computeKineticEnergy2(p1, p2, v1, v2, 1.0, 2.0, 1.0);
		//TEST_FUZZY(kin2, pot3, EPSILON, "Energy calculations for constant force fields failed");

		{
			double pot = physics::computePotentialEnergy3(p1, p2, 1.0, 100.0, 0.0, g, physics::ForceType::Const, 30);
			double kin = physics::computeKineticEnergy2(p1, p2, v1, v2, 1.0, 100.0, 1.0);
			//TEST_FUZZY(kin, pot, EPSILON, "Energy calculations for constant force fields failed");
		}
		{
			double kin = physics::computeKineticEnergy(vec::Data2<double>(fallVelocity, 0.0), 1.0);
			double pot = physics::computePotentialEnergy4(p1, p2, 1.0, 100.0, 0.0, g, physics::ForceType::Const, 30);
			//TEST_FUZZY(kin, pot, EPSILON, "Energy calculations for constant force fields failed");
		}


		{
			double g = 9.81;
			vec::Data2<double> p1(10.0, 0.0);
			vec::Data2<double> p2(0.0, 0.0);
			double fallDistance = (p2 - p1).length();
			double m = 1.0;
			double j0 = 1.0;
			double fallTime = std::pow(fallDistance * 6.0 / j0, 1.0 / 3.0);
			double fallVelocity = fallTime * fallTime * j0 / 2.0;
			double pot = physics::computePotentialEnergy3(p1, p2, 1.0, 1.0, 0.0, j0, physics::ForceType::Square, 300);
			double kin = physics::computeKineticEnergy2(p1, p2, v1, v2, 1.0, 1.0, fallTime);
			//TEST_FUZZY(kin, pot, EPSILON, "Energy calculations for constant force fields failed");
		}
	}

	return 0;
}

TEST(Integration, Stabilization) {

	int numObjects = 2;
	int degree = 2; // position, velocity
	double dt = 1 / 30.0;

	std::vector<vec::Data4<double>> newState;
	std::vector<vec::Data4<double>> oldState;
	newState.clear();
	oldState.clear();
	newState.resize(numObjects * degree);
	oldState.resize(numObjects * degree);

	newState[0] = vec::Data4<double>(0.0, 0.0, 0.0, 0.0);
	newState[1] = vec::Data4<double>(10.0, 0.0, 0.0, 0.0);
	newState[2] = vec::Data4<double>(0.1, 0.1, 0.0, 0.0);
	newState[3] = vec::Data4<double>(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < numObjects * degree; i++) {
		oldState[i] = newState[i];
	}

	uint32_t offset0 = degree * 0;
	uint32_t offset1 = degree * 1;

	for (int i = 0; i < numObjects * degree; i += degree) {
		newState[i] += newState[i + 1] * dt;
	}

	//physics::applyPotentialStabilization(oldState[offset0], newState[offset0], newState[offset0 + 1], oldState[offset1], newState[offset1], newState[offset1 + 1], dt);
	//TEST_FUZZY(newState[offset0].x1, 0.2, EPSILON, "Failed");

	return 0;
}
