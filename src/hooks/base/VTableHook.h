#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace Hooks::Detail
{

class VTableHook
{
public:
    bool attach(void* object, const void* expectedVTable, size_t slotCount);
    void detach();

    bool replace(size_t slot, void* function);
    void* original(size_t slot) const;

    bool attachedTo(const void* object) const;

private:
    void* m_object = nullptr;
    void** m_originalVTable = nullptr;
    void** m_current = nullptr;
    std::vector<std::unique_ptr<void*[]>> m_copies;
    std::vector<std::pair<size_t, void*>> m_replacements;
};

}
