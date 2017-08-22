// Include file for exporting a Windows DLL API, if we are compiling under Windows

#ifdef _WIN32

#ifdef PROFILER_EXPORTS  
#define PROFILER_API __declspec(dllexport)   
#else  
#define PROFILER_API __declspec(dllimport)   
#endif  

// Else create empty def for PROFILER_API

#else
#define PROFILER_API
#endif



