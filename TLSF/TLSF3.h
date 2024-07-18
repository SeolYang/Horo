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
        // Free list에서 제거, fli 그리고 sli 계산 후 bitmap 업데이트 및 만약 블럭이 해당 레벨에서의 헤드 노드인 경우 freeLists 요소를 null로
        void Extract(TLSFBlock* block);

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