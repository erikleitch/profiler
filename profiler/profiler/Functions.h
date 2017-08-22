#pragma once

#ifndef PROFILER_FUNCTIONS_H
#define PROFILER_FUNCTIONS_H

#include "export.h"

namespace profiler
{
	// This class is exported from the Profiler.dll  
	class Functions
	{
	public:
		// Returns a + b  
		static PROFILER_API double Add(double a, double b);

		// Returns a * b  
		static PROFILER_API double Multiply(double a, double b);

		// Returns a + (a * b)  
		static PROFILER_API double AddMultiply(double a, double b);

		PROFILER_API double someFn(double a, double b);
	};
}

#endif
