#pragma once

#include "../pch.h"

namespace TinySpark
{
    template <typename U>
    class MFR
    {
    public:
        MFR(const std::vector<U> &list)
        {
            m_Values.emplace<std::vector<U>>(std::move(list));
        }

        ~MFR() {}

        template <typename OutVecType, typename InputVecType, typename FUNC>
        void MFR_MapList(FUNC func)
        {
            std::vector<OutVecType> output;
            output.reserve(std::get<std::vector<InputVecType>>(m_Values).size());
            
            // map
            std::transform((std::get<std::vector<InputVecType>>(m_Values)).begin(), (std::get<std::vector<InputVecType>>(m_Values)).end(), std::back_inserter(output), func);
            
            m_Values.emplace<std::vector<OutVecType>>(output.begin(), output.end());
        }

        template <typename OutVecType, typename InputVecType, typename FUNC>
        void MFR_FilterList(FUNC func)
        {
            std::vector<OutVecType> output;
            output.reserve(std::get<std::vector<InputVecType>>(m_Values).size());
            
            // filter
            std::copy_if((std::get<std::vector<InputVecType>>(m_Values)).begin(), (std::get<std::vector<InputVecType>>(m_Values)).end(), std::back_inserter(output), func);
            
            m_Values.emplace<std::vector<OutVecType>>(output.begin(), output.end());
        }

        template <typename InputVecType, typename FUNC, typename T>
        T MFR_ReduceList(FUNC func, T initial)
        {
            // reduce
            return std::reduce((std::get<std::vector<InputVecType>>(m_Values)).begin(), (std::get<std::vector<InputVecType>>(m_Values)).end(), initial, func);
        }

        template <typename InputVecType, typename InputMapType, typename FUNC>
        void MFR_MapFile(FUNC func)
        {
            std::transform((std::get<std::vector<InputVecType>>(m_Values)).begin(), (std::get<std::vector<InputVecType>>(m_Values)).begin(), std::inserter(std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map), (std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map)).end()), func);
        }

        template <typename InputVecType, typename InputMapType>
        void MFR_ReduceFile(std::string_view what)
        {
            std::for_each((std::get<std::vector<InputVecType>>(m_Values)).begin(), (std::get<std::vector<InputVecType>>(m_Values)).end(), [&](const std::string& elem) { ++(std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map))[elem]; } );
            
            if (what == "wc")
            {
                ;
            }
            else if (what == "stoi")
            {
                std::for_each((std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map)).begin(), (std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map)).end(), [&](const auto& pair) { m_NonMapVal += std::stol(pair.first); } );
            }
            else if (what == "count")
            {
                std::for_each((std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map)).begin(), (std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map)).end(), [&](const auto& pair) { m_NonMapVal += pair.second; } );
            }
        }

        template <typename InputMapType>
        std::unordered_map<InputMapType, uint32_t>& MFR_ReturnMap()
        {
            return std::get<std::unordered_map<InputMapType, uint32_t>>(m_Map);
        }

        template <typename T>
        T MFR_ReturnNonMap()
        {
            return m_NonMapVal;
        }

    private:
        std::variant<std::vector<float>, std::vector<std::string>, std::vector<bool>, std::vector<char>> m_Values;

        typedef std::variant<std::unordered_map<std::string, uint32_t>, std::unordered_map<uint32_t, uint32_t>> map;
        map m_Map;

        uint32_t m_NonMapVal = 0;
    };
} // namespace TinySpark