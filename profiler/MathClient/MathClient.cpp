// MathClient.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "Ws2_32.lib")

#include "stdafx.h"
#include "Functions.h"
#include "ProfString.h"
#include "Profiler.h"
#include <iostream>
#include <windows.h>

using namespace profiler;

int main()
{
	Functions fn;
	std::cout << " test - " << Functions::Add(1.0, 2.0) << std::endl;
	std::cout << " test - " << fn.Add(1.0, 2.0) << std::endl;
	std::cout << " test - " << fn.someFn(1.0, 2.0) << std::endl;

	String str("this is a test");
	std::cout << "Contains test: " << str.contains("test") << std::endl;
        Profiler::profile("start", "test", false, true);
	Sleep(2000);
        Profiler::profile("stop", "test", false, true);
        Profiler::profile("debug", "test", false, true);
        Profiler::profile("dump", "C:\\Users\\eleitch\\Desktop\\prof.txt", false, true);
	Sleep(2000);
	return 0;
}

