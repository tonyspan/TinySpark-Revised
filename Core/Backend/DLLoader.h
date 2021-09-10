#pragma once

#include "../pch.h"

#include "../PlatformDetection.h"

namespace TinySpark
{
	class DLLoader
	{
	public:
		DLLoader() {}

		~DLLoader() { UnloadFile(); }

		void LoadFile(const std::string& filepathToDL)
		{
		#ifdef PLATFORM_WINDOWS
			m_Handler = LoadLibrary(filepathToDL.c_str());
		#endif
		#ifdef PLATFORM_LINUX
			m_Handler = dlopen(filepathToDL.c_str(), RTLD_NOW | RTLD_GLOBAL);
		#endif
			if (m_Handler == nullptr)
			{
				std::string what = "Could not load " + std::filesystem::path(filepathToDL).filename().string() + " from " + std::filesystem::path(filepathToDL).parent_path().string();
				throw std::runtime_error(what.c_str());
			}
		}

		template<typename Ret, typename... Args>
		std::function<Ret(Args...)> Get(const std::string& funcName)
		{
		#ifdef PLATFORM_WINDOWS
			FARPROC funcID = GetProcAddress(m_Handler, funcName.c_str());
		#endif
		#ifdef PLATFORM_LINUX
			void* funcID = dlsym(m_Handler, funcName.c_str());
		#endif
			if (funcID == nullptr)
			{
				std::string what = "Could not locate the function " + funcName;
				throw std::runtime_error(what.c_str());
			}
		#ifdef PLATFORM_WINDOWS
			return std::function<Ret(Args...)>(reinterpret_cast<Ret(__stdcall*)(Args...)>(funcID));
		#endif
		#ifdef PLATFORM_LINUX
			return std::function<Ret(Args...)>(reinterpret_cast<Ret(*)(Args...)>(funcID));
		#endif
		}
	private:
		void UnloadFile()
		{
			if (m_Handler)
			{
			#ifdef PLATFORM_WINDOWS
				FreeLibrary(m_Handler);
			#endif
			#ifdef PLATFORM_LINUX
				dlclose(m_Handler);
			#endif
				m_Handler = nullptr;
			}
		}
	private:
	#ifdef PLATFORM_WINDOWS
		HINSTANCE m_Handler = nullptr;
	#endif
	#ifdef PLATFORM_LINUX
		void* m_Handler = nullptr;
	#endif
	};
}