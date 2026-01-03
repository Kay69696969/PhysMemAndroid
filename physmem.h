#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <stdint.h>

namespace physmem {
    bool InitDriver();
    int get_pid(const char* package_name);
    uintptr_t get_module_base(int pid, const char* mod_name);

    bool ReadRaw(int pid, uintptr_t address, void* buffer, size_t size);
    bool WriteRaw(int pid, uintptr_t address, void* buffer, size_t size);

    template <typename T>
    T Read(int pid, uintptr_t address) {
        T buffer{};
        ReadRaw(pid, address, &buffer, sizeof(T));
        return buffer;
    }
}

#endif