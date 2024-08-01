#include "physics/Integration.h"

#include "physics/VecX.h"
#include "physics/Algorithm.h"

#include "testing/Test.h"

#define EPSILON 0.0000001

using namespace l;



using PoleResolver = std::function<void(
	const vec::Data2<double>& p0Change,
	const vec::Data2<double>& p1Change,
	const vec::Data2<double>& v0dtChange,
	const vec::Data2<double>& v1dtChange,
	const vec::Data2<double>& a0dtdtChange,
	const vec::Data2<double>& a1dtdtChange
	)>;

bool detectPole(
	const vec::Data2<double>& p0,
	const vec::Data2<double>& v0dt,
	const vec::Data2<double>& a0dtdt,
	const vec::Data2<double>& p1,
	const vec::Data2<double>& v1dt,
	const vec::Data2<double>& a1dtdt,
	double dt,
	PoleResolver resolver)
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
	auto dp = p0 - p1;

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

	if (f2 < 0.0) {
		if (tMidpoint > e && tMidpoint < 1.0 - e) {

			// make a partial step to arrive at the midpoint
			auto p0Midpoint = p0 + dp0Next * tMidpoint;
			auto p1Midpoint = p1 + dp1Next * tMidpoint;

			// move to symmetric position
			auto p0Change = dp0Next * (tMidpoint * 2.0 - 0.5);
			auto p1Change = dp1Next * (tMidpoint * 2.0 - 0.5);

			auto v0dtChange = -p0Change * dt * 0.0;
			auto v1dtChange = -p1Change * dt * 0.0;

			auto a0dtdtChange = a0dtdt * (tMidpoint * 1.0) * 0.0;
			auto a1dtdtChange = a1dtdt * (tMidpoint * 1.0) * 0.0;


			// we are now at the midpoint of the pair's center of mass over this time step 
			// we can now compute the direction of the acceleration at this point and
			// integrate velocity to be able to take one more partial step
			auto dpMidpoint = p0Midpoint - p1Midpoint;
			if (dpMidpoint.sqr() > 4*e) {
				auto dpMidpointLength = dpMidpoint.length();
				dpMidpoint *= 1.0 / dpMidpointLength;
				// add acceleration projection from midpoint direction twice to simulate symmetric forces
				// this should translate into velocity adjustment 
				a0dtdtChange += a0dtdt * a0dtdt.dot(dpMidpoint) * (tMidpoint * 0.0);
				a1dtdtChange += a1dtdt * a1dtdt.dot(dpMidpoint) * (tMidpoint * 0.0);
			}

			resolver(p0Change, p1Change, v0dtChange, v1dtChange, a0dtdtChange, a1dtdtChange);

			return true;
		}
	}
	return false;
}



TEST(PoleResolver, BasicPoleSteps1D) {
	{
		// pole should be at 1.5x
		vec::Data2<double> p0(1.0, 0.0);
		vec::Data2<double> p1(2.0, 0.0);
		vec::Data2<double> v0(1.0, 0.0);
		vec::Data2<double> v1(-1.0, 0.0);
		vec::Data2<double> a0(2.0, 0.0);
		vec::Data2<double> a1(-2.0, 0.0);

		vec::Data2<double> changes[6];

		double dt = 0.5;
		bool result = detectPole(p0, v0 * dt, a0 * dt * dt * 0.5, p1, v1 * dt, a1 * dt * dt * 0.5, dt, [&](
			const vec::Data2<double>& p0Change,
			const vec::Data2<double>& p1Change,
			const vec::Data2<double>& v0dtChange,
			const vec::Data2<double>& v1dtChange,
			const vec::Data2<double>& a0dtdtChange,
			const vec::Data2<double>& a1dtdtChange) {
				changes[0] = p0Change;
				changes[1] = p1Change;
				changes[2] = v0dtChange;
				changes[3] = v1dtChange;
				changes[4] = a0dtdtChange;
				changes[5] = a1dtdtChange;

			});

		auto v0Next = v0.x1 + changes[2].x1 + changes[4].x1;
		TEST_FUZZY(v0Next, 1.00, EPSILON, "v0Next change wrong");

		auto p0Next = p0.x1 + changes[0].x1 + v0Next * dt;
		TEST_FUZZY(p0Next, 2.125, EPSILON, "p0Next change wrong");


		TEST_FUZZY(changes[0].x1, 0.625, EPSILON, "p0 change wrong");
		TEST_FUZZY(changes[1].x1, -0.625, EPSILON, "p1 change wrong");
		TEST_FUZZY(changes[2].x1, 0.0, EPSILON, "v0dt change wrong");
		TEST_FUZZY(changes[3].x1, 0.0, EPSILON, "v1dt change wrong");
		TEST_FUZZY(changes[4].x1, 0.0, EPSILON, "a0dtdt change wrong");
		TEST_FUZZY(changes[5].x1, 0.0, EPSILON, "a1dtdt change wrong");


		TEST_TRUE(result, "pole result was false");
	}

	return 0;
}

