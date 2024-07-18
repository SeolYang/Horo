#pragma once
#include <cstdint>
#include <vector>
#include <bit>
#include <cmath>

namespace gen3
{
    struct TLSFBlock;
    struct TLSFAllocation
    {
        TLSFBlock* Block = nullptr;
        size_t Offset = 0;
        size_t Size = 0;
    };

    class TLSFAllocator
    {
    public:
        TLSFAllocator(const size_t memoryPoolSize, const size_t secondLevelParam = 4);
        ~TLSFAllocator();

        TLSFAllocation Allocate(const size_t size);
        void Deallocate(TLSFAllocation allocation);

    private:
        void Insert(TLSFBlock* block);
        TLSFBlock* ExtractHead(const size_t firstLevelIdx, const size_t secondLevelIdx);
        void Extract(TLSFBlock* block);


        void Merge(TLSFBlock* baseBlock, TLSFBlock* targetBlock);

    private:
        size_t memoryPoolSize = 0;

        size_t firstLevelParam = 0;
        uint64_t firstLevelBitmap = 0;

        size_t secondLevelParam = 0;
        size_t numSubdivisions = 0;
        std::vector<uint64_t> secondLevelBitmaps;
        std::vector<TLSFBlock*> freeLists;
    };
} // namespace gen3