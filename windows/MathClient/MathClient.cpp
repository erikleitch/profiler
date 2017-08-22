// MathClient.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "Ws2_32.lib")

#include "stdafx.h"
#include "ProfString.h"
#include "Profiler.h"
#include <iostream>
#include <windows.h>

using namespace profiler;

int main()
{
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

