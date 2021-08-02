#pragma once

namespace TinySpark
{
    class ServerRegistry
    {
    public:
        ServerRegistry();
        ServerRegistry(const std::string &addrPort);
        ~ServerRegistry();

        uint32_t GetID() const;

        void SetHostAndPort(const std::string &addrPort);
        std::string GetHostAndPort() const;

        void SetAlive(const bool alive);
        bool IsAlive() const;

        void SetLoad(const bool load);
        bool HasLoad() const;

    private:
        uint32_t m_Id;                // unique server number
        static uint32_t s_IdRefCount; // server counter
        std::string m_HostAndPort;
        bool m_Alive = false;
        bool m_Load = false;
    };
} // namespace TinySpark