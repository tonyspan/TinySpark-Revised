#pragma once

#if defined(_WIN32) || defined(__WIN32__)
	#define PLATFORM_WINDOWS
	#include <Windows.h>
#elif defined(linux) || defined(__linux) || defined(__linux__)
	#define PLATFORM_LINUX
	#include <unistd.h>
	#include <dlfcn.h>
#else
	#error "Unknown Platform"
#endif