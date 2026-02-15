#include "pch.h"
#include "QuadraticSolver.h"
#include <MMath.h>

using namespace MATH;
using namespace GEOMETRY;

void QuadraticSolution::print() const
{
	if (numSolutions == NumSolutions::zero) {
		std::cout << "There are no solutions." << std::endl;
	}
	else if (numSolutions == NumSolutions::one) {
		std::cout << "There is only one solution: " << firstSolution << std::endl;
	}
	else if (numSolutions == NumSolutions::two) {
		std::cout << "There are two solutions: \n" <<
					 "First Solution: " << firstSolution <<
					 "\nSecond Solution: " << secondSolution << std::endl;
	}
	
}

QuadraticSolution QuadraticSolution::solveQuadratic(float a, float b, float c)
{
	//-b +- sqrt(pow(b, 2) - (4 * a * c)) / 2 * a;

	float discriminat = pow(b, 2) - (4 * a * c);

	float divB = -b / (2 * a);

	// one real root (the ray grazes the shape, one intersection)
	if (discriminat == 0.0f) {
		numSolutions = NumSolutions::one;

		firstSolution = -b / (2 * a);
	}
	// two real roots (the ray pierces the shape, two intersection points)
	else if (discriminat > 0.0f) {
		numSolutions = NumSolutions::two;

		float squareRoot = sqrtf(discriminat) / (2 * a);

		firstSolution = divB + squareRoot;
		secondSolution = divB - squareRoot;

		float smallestRoot = std::min(firstSolution, secondSolution);

		firstSolution = smallestRoot;
	}
	// two complex roots (there is no intersection)
	else if (discriminat < 0.0f) {
		numSolutions = NumSolutions::zero;

		/*float squareRoot = sqrtf(-discriminat) / (2 * a);

		firstSolution = divB + squareRoot;
		secondSolution = divB - squareRoot;*/
	}

	//print();

	return QuadraticSolution();
}