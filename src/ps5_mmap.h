#ifndef PS5_MMAP_H
#define PS5_MMAP_H

#include <kernel.h>
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset){
    sceKernelMapDirectMemory(&addr, length, prot, flags, offset, 0);
    return addr;
}

int munmap(void* addr, size_t length){
    return sceKernelMunmap(addr, length);
}

#endif