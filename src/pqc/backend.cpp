// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/backend.h>

#include <pqc/backend_oqs.h>
#include <pqc/backend_test.h>

#include <map>
#include <mutex>

namespace pqc {
namespace {
std::map<std::string, std::shared_ptr<PQCBackend>>& Registry()
{
    static std::map<std::string, std::shared_ptr<PQCBackend>> registry;
    return registry;
}
std::mutex& RegistryMutex()
{
    static std::mutex mutex;
    return mutex;
}
} // namespace

void RegisterPQCBackend(std::shared_ptr<PQCBackend> backend)
{
    if (!backend) return;
    std::lock_guard<std::mutex> lock(RegistryMutex());
    Registry()[backend->Name()] = std::move(backend);
}

std::shared_ptr<PQCBackend> GetPQCBackend(const std::string& name)
{
    std::lock_guard<std::mutex> lock(RegistryMutex());
    const auto it{Registry().find(name)};
    if (it == Registry().end()) return nullptr;
    return it->second;
}

std::vector<std::string> ListPQCBackends()
{
    std::lock_guard<std::mutex> lock(RegistryMutex());
    std::vector<std::string> names;
    for (const auto& [name, _] : Registry()) names.push_back(name);
    return names;
}

void RegisterDefaultPQCBackends()
{
    RegisterPQCBackend(std::make_shared<TestBackend>());
#ifdef HAVE_LIBOQS
    RegisterPQCBackend(std::make_shared<OqsBackend>());
#endif
}

} // namespace pqc
