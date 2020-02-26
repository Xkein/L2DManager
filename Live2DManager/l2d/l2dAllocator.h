#pragma once

#include <CubismFramework.hpp>
#include <ICubismAllocator.hpp>


class Live2DAllocator : public Csm::ICubismAllocator
{
    void* Allocate(const Csm::csmSizeType size)
    {
        return malloc(size);
    }

    void Deallocate(void* memory)
    {
        free(memory);
    }
    void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment)
    {
        size_t offset, shift, alignedAddress;
        void* allocation;
        void** preamble;

        offset = alignment - 1 + sizeof(void*);

        allocation = Allocate(size + static_cast<Csm::csmUint32>(offset));

        alignedAddress = reinterpret_cast<size_t>(allocation) + sizeof(void*);

        shift = alignedAddress % alignment;

        if (shift)
        {
            alignedAddress += (alignment - shift);
        }

        preamble = reinterpret_cast<void**>(alignedAddress);
        preamble[-1] = allocation;

        return reinterpret_cast<void*>(alignedAddress);
    }

    void DeallocateAligned(void* alignedMemory)
    {
        void** preamble;

        preamble = static_cast<void**>(alignedMemory);

        Deallocate(preamble[-1]);
    }
};
