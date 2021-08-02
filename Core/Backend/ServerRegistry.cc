#include "../pch.h"

#include "ServerRegistry.h"

namespace TinySpark
{

    ServerRegistry::ServerRegistry() { m_Id = ++s_IdRefCount; }

    ServerRegistry::ServerRegistry(const std::string &addrPort) : m_HostAndPort(addrPort)
    {
        m_Id = ++s_IdRefCount;
    }

    ServerRegistry::~ServerRegistry() { --s_IdRefCount; }

    uint32_t ServerRegistry::GetID() const { return m_Id; }

    void ServerRegistry::SetHostAndPort(const std::string &addrPort) { m_HostAndPort = addrPort; }
    std::string ServerRegistry::GetHostAndPort() const { return m_HostAndPort; }

    void ServerRegistry::SetAlive(const bool alive) { m_Alive = alive; }
    bool ServerRegistry::IsAlive() const { return m_Alive; }

    void ServerRegistry::SetLoad(const bool load) { m_Load = load; }
    bool ServerRegistry::HasLoad() const { return m_Load; }

    uint32_t ServerRegistry::s_IdRefCount = 0;
} // namespace TinySpark