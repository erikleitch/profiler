// Profiler.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DPROFILER_EXPORTS /LD Profiler.cpp  

#include "stdafx.h"  
#include "Functions.h"  

namespace profiler
{
	double Functions::Add(double a, double b)
	{
		return a + b;
	}

	double Functions::Multiply(double a, double b)
	{
		return a * b;
	}

	double Functions::AddMultiply(double a, double b)
	{
		return a + (a * b);
	}

	double Functions::someFn(double a, double b)
	{
		return a + (a * b);
	}
}

