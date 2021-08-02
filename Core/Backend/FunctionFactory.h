#pragma once

#include "../pch.h"
#include "../Types.h"
#include "../PlatformDetection.h"

namespace TinySpark
{
    // Function Components
    struct FuncComp
    {
        int Id;
        std::string MapFormal;
        std::string MapRet;

        std::string ReduceFormal;
        std::string ReduceRet;

        std::string FilterFormal;
        std::string FilterRet;
        std::string HasFilter;

        TinySpark::Types::Types Type;
    };

    class FunctionFactory
    {
    public:

        static FunctionFactory& GetInstance();
        
        FunctionFactory(const FunctionFactory&) = delete;
        FunctionFactory(FunctionFactory&&) = delete;
        FunctionFactory& operator=(const FunctionFactory&) = delete;
        FunctionFactory& operator=(FunctionFactory&&) = delete;

        // Add function components to build function later
        void AddFunction(const FuncComp& components)
        {
            m_Id = components.Id;
            m_MapFormal = components.MapFormal;
            m_MapRet = components.MapRet;
            m_ReduceFormal = components.ReduceFormal;
            m_ReduceRet = components.ReduceRet;
            m_FilterFormal = components.FilterFormal;
            m_FilterRet = components.FilterRet;
            m_HasFilter = components.HasFilter;
            m_Type = TinySpark::Types::to_string(components.Type);
        }

        // TODO: Generalize to handle multiple args & types
        void BuildFunction()
        {
            std::ofstream cppfile("f" + std::to_string(m_Id) + ".cpp", std::ios_base::ate);

            cppfile <<  "#if defined(_WIN32) || defined(__WIN32__)\n";
            cppfile << "\t#define  EXPORT __declspec(dllexport)\n";
            cppfile << "#elif defined(linux) || defined(__linux) || defined(__linux__)\n";
            cppfile << "\t#define EXPORT extern \"C\"\n";
            cppfile << "#endif\n";
            
            cppfile << "#include <string>\n";
            cppfile << "#include <cmath>\n";

            cppfile << "EXPORT ";
            auto formal = SplitFormal(m_MapFormal);
            auto ret = MapRetPyToCpp(m_MapRet);
            cppfile << ret.RetType << " " << "map" << std::to_string(m_Id) << "(" << m_Type << " " << formal.Arg1 << ") {" << " return " << ret.Arg1 << "; }" << std::endl;
            
            cppfile << "EXPORT ";
            formal = SplitFormal(m_ReduceFormal);
            cppfile << ret.RetType << " " << "reduce" << std::to_string(m_Id) << "(" << ret.RetType << " " << formal.Arg1 << ", " << ret.RetType << " " << formal.Arg2 << ") {" << " return " << m_ReduceRet << "; }" << std::endl;

            if(TinySpark::Types::to_bool(m_HasFilter))
            {
                cppfile << "EXPORT ";
                formal = SplitFormal(m_FilterFormal);
                cppfile << ret.RetType << " " << "filter" << std::to_string(m_Id) << "(" << m_Type << " " << formal.Arg1 << ") {" << " return " << m_FilterRet << "; }" << std::endl;
            }
        }

        // TODO: Add support for different paths other than cmake's build dir
        void CompileFunction()
        {
        #ifdef PLATFORM_WINDOWS
            int sys = system(("call \"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" & cl.exe f" + std::to_string(m_Id) + ".cpp /link /DLL /OUT:f" + std::to_string(m_Id) + ".dll").c_str());
        #endif
        #ifdef PLATFORM_LINUX
            int sys = system(("g++ -Os -std=c++17 -fPIC -shared f" + std::to_string(m_Id) + ".cpp -o f" + std::to_string(m_Id) + ".so").c_str());
        #endif
            if(sys)
                throw std::runtime_error("TinySpark: Failed to compile Dynamic Library f" + std::to_string(m_Id) + std::strerror(errno));
        }

        // Deletes .cpp & .so/.dll files
        // TODO: Add deletion for Windows
        void CleanUpFunction()
        {
        #ifdef PLATFORM_WINDOWS
            // TBA
        #endif
        #ifdef PLATFORM_LINUX
            int sys = system(("rm -rf f" + std::to_string(m_Id) + ".cpp && rm -rf f" + std::to_string(m_Id) + ".so").c_str());
        #endif
        }

    protected:
        FunctionFactory() {}
        ~FunctionFactory() {}

    private:

        struct Args
        {
            std::string Arg1;
            std::string Arg2;
            
            std::string RetType;
        };

        // Splits formal args, passed as "x,y" (without quotes) by frontend,
        // into 2 args, if only 1 arg is passed, returs it
        // Type of args is determined by MapRetPyToCpp()
        // TODO: Add support for more than 2 args
        Args SplitFormal(const std::string& func)
        {
            auto it = std::find(func.begin(), func.end(), ',');

            if(it != func.end())
                return { std::string(func.begin(), it), std::string(it + 1, func.end()), std::string("") };
            else
                return { func, std::string(""), std::string("") };
        }

        // Maps Python's "special" return statement to C++
        // e.g power of number: "**" -> std::pow (Python & C++ respectively)
        // Uses same Args struct for convinience
        // Types of args are hardcoded
        // TODO: Refactor
        Args MapRetPyToCpp(const std::string &func)
        {   
            for(auto& binding : m_PyToCppMap)
            {
                auto it = m_MapRet.find(binding);

                if(it != std::string::npos)
                {
                    if(binding == "**")
                        return { std::string("std::pow(" + m_MapRet.substr(0, 1) + "," + m_MapRet.substr(it + 2, 3) + ")"), std::string(""), std::string("float") };
                    else if(binding == ".length")
                        return { std::string(m_MapRet.substr(0, m_MapRet.size()) + "()"), std::string(""), std::string("float") };
                }
            }

            // Should not reach here
            return { std::string(""), std::string(""), std::string("") };
        }

    private:
        int m_Id;
        std::string m_MapFormal;
        std::string m_MapRet;

        std::string m_ReduceFormal;
        std::string m_ReduceRet;

        std::string m_FilterFormal;
        std::string m_FilterRet;
        std::string m_HasFilter;
        std::string m_Type;

        // add more bindings here and update MapRetPyToCpp()
        static const std::vector<std::string> m_PyToCppMap;
    };

    FunctionFactory& FunctionFactory::GetInstance()
    {
        static FunctionFactory m_Instance;
        
        return m_Instance;
    }

    const std::vector<std::string> FunctionFactory::m_PyToCppMap{ "**", ".length" };
}