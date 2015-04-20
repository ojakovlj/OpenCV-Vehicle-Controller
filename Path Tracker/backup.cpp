#include "stdafx.h"
#include "CVDetectorDLL.h"
#include <stdexcept>

using namespace std;

namespace MathFuncs
{
	extern "C"{
    __declspec(dllexport) double __cdecl MyMathFuncs::Add(double a, double b)
    {
        return a + b;
    }

    double MyMathFuncs::Subtract(double a, double b)
    {
        return a - b;
    }

    double MyMathFuncs::Multiply(double a, double b)
    {
        return a * b;
    }

    double MyMathFuncs::Divide(double a, double b)
    {
        if (b == 0)
        {
            throw invalid_argument("b cannot be zero!");
        }

        return a / b;
    }
	}
}


