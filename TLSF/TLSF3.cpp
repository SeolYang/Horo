#include "TLSF3.h"

namespace gen3
{
    constexpr inline size_t InvalidTLSFLevelIdx = (size_t)-1;

    struct TLSFBlock
    {
        size_t Offset = 0;
        size_t Size : 63 = 0;
        size_t bIsUsed : 1 = false;

        TLSFBlock* PrevPhysicalBlock = nullptr;
        TLSFBlock* NextPhysicalBlock = nullptr;
        TLSFBlock* PrevFreeBlock = nullptr;
        TLSFBlock* NextFreeBlock = nullptr;
    };

    struct TLSFMappedLevels
    {
        size_t FirstLevel = 0;
        size_t SecondLevel = 0;
    };

    static size_t CalculateSignificantBitIndex(const size_t val)
    {
        return std::min<uint8_t>(64Ui8 - (uint8_t)std::countl_zero(val) - 1Ui8, 63Ui8);
    }

    static TLSFMappedLevels MapLevels(const size_t size, const size_t secondLevelParam)
    {
        const size_t significantBitIdx = CalculateSignificantBitIndex(size);
        return TLSFMappedLevels{ .FirstLevel = significantBitIdx, .SecondLevel = ((size ^ (1Ui64 << significantBitIdx))) >> (significantBitIdx - secondLevelParam) };
    }

    TLSFAllocator::TLSFAllocator(const size_t memoryPoolSize, const size_t secondLevelParam /*= 4*/)
        : memoryPoolSize(memoryPoolSize),
          firstLevelParam(CalculateSignificantBitIndex(memoryPoolSize)),
          secondLevelParam(secondLevelParam),
          numSubdivisions(1Ui64 << secondLevelParam)
    {
        secondLevelBitmaps.resize(firstLevelParam + 1Ui64);
        freeLists.resize(firstLevelParam * numSubdivisions + 1Ui64);

        TLSFBlock* newBlock = new TLSFBlock;
        newBlock->Offset = 0Ui64;
        newBlock->Size = memoryPoolSize;
        Insert(newBlock);
    }

    TLSFAllocator::~TLSFAllocator()
    {
        // Physical block 기준으로 해제하면 모두 해제 될려나?
        for (TLSFBlock* freeBlock : freeLists)
        {
            if (freeBlock != nullptr)
            {
                TLSFBlock* nextFreeBlock = freeBlock->NextFreeBlock;
                while (nextFreeBlock != nullptr)
                {
                    TLSFBlock* tempBlockPtr = nextFreeBlock;
                    nextFreeBlock = nextFreeBlock->NextFreeBlock;
                    delete tempBlockPtr;
                }

                delete freeBlock;
            }
        }
    }

    TLSFAllocation TLSFAllocator::Allocate(const size_t allocSize)
    {
        const TLSFMappedLevels initialLevels = MapLevels(allocSize, secondLevelParam);
        size_t foundFirstLevelIdx = InvalidTLSFLevelIdx;
        for (size_t targetFirstLevelIdx = initialLevels.FirstLevel; targetFirstLevelIdx <= firstLevelParam; ++targetFirstLevelIdx)
        {
            if (firstLevelBitmap & (1Ui64 << targetFirstLevelIdx))
            {
                foundFirstLevelIdx = targetFirstLevelIdx;
            }
        }

        // Case 1: 요구된 크기의 클래스에 속하는 블럭이 없는 경우
        if (foundFirstLevelIdx == InvalidTLSFLevelIdx)
        {
            return TLSFAllocation{};
        }

        size_t& secondLevelBitmap = secondLevelBitmaps[foundFirstLevelIdx];
        size_t foundSecondLevelIdx = InvalidTLSFLevelIdx;
        for (size_t targetSecondLevelIdx = foundFirstLevelIdx == initialLevels.FirstLevel ? initialLevels.SecondLevel : 0Ui64;
             targetSecondLevelIdx <= numSubdivisions; ++targetSecondLevelIdx)
        {
            if (secondLevelBitmap & (1Ui64 << targetSecondLevelIdx))
            {
                foundSecondLevelIdx = targetSecondLevelIdx;
            }
        }

        // Case 2: 요구된 크기의 클래스에 속하는 가용 블럭은 존재하지만, 세부 클래스(2nd level)에 속하는 가용 블럭은 없는 경우
        if (foundSecondLevelIdx == InvalidTLSFLevelIdx)
        {
            return TLSFAllocation{};
        }

        TLSFBlock* extractedBlock = ExtractHead(foundFirstLevelIdx, foundSecondLevelIdx);

        if (allocSize < extractedBlock->Size)
        {
            TLSFBlock* splittedBlock = new TLSFBlock;
            splittedBlock->Offset = extractedBlock->Offset + allocSize;
            splittedBlock->Size = extractedBlock->Size - allocSize;
            splittedBlock->PrevPhysicalBlock = extractedBlock;

            extractedBlock->Size = allocSize;
            extractedBlock->NextPhysicalBlock = splittedBlock;
            Insert(splittedBlock);
        }
        // Case 3: 요구된 크기의 클래스에 만족하는 블럭은 있지만, 요구된 크기에 정확히 맞는 블럭이 없는 경우
        // @note 이 경우, 첫 번째 head 노드가 크기에 맞지 않는 경우만 상정하기 때문에.. 필요하다면 해당 free list를 순회하여
        // 요구 크기에 맞는 블럭을 탐색 할 수는 있을 것 같음
        else
        {
            return TLSFAllocation{};
        }

        extractedBlock->bIsUsed = true;
        return TLSFAllocation{ .Block = extractedBlock, .Offset = extractedBlock->Offset, .Size = extractedBlock->Size };
    }

    void TLSFAllocator::Deallocate(TLSFAllocation allocation)
    {
        // assert; allocation.Block != nullptr
        // assert; allocation.Block->bIsUsed
        const bool bPrevPhysicalBlockMergeable = allocation.Block->PrevPhysicalBlock != nullptr && !allocation.Block->PrevPhysicalBlock->bIsUsed;
        const bool bNextPhysicalBlockMergeable = allocation.Block->NextPhysicalBlock != nullptr && !allocation.Block->NextPhysicalBlock->bIsUsed;
        // Case 1: 양 쪽 모두 가용 블럭인 경우
        if (bPrevPhysicalBlockMergeable && bNextPhysicalBlockMergeable)
        {
            TLSFBlock* leftBlock = allocation.Block->PrevPhysicalBlock;
            TLSFBlock* midBlock = allocation.Block;
            TLSFBlock* rightBlock = allocation.Block->NextPhysicalBlock;

            Extract(leftBlock);
            Extract(rightBlock);

            leftBlock->Size += (midBlock->Size + rightBlock->Size);
            leftBlock->NextPhysicalBlock = rightBlock->NextPhysicalBlock;
            if (rightBlock->NextPhysicalBlock != nullptr)
            {
                // assert; rightBlock == rightBlock->NextPhysicalBlock->PrevPhysicalBlock
                rightBlock->NextPhysicalBlock->PrevPhysicalBlock = leftBlock;
            }

            delete midBlock;
            delete rightBlock;
            Insert(leftBlock);
        }
        else if (bPrevPhysicalBlockMergeable || bNextPhysicalBlockMergeable)
        {
            TLSFBlock* extractTargetBlock = nullptr;
            TLSFBlock* mergeBaseBlock = nullptr;
            TLSFBlock* mergeTargetBlock = nullptr;

            // Case 2: 앞 쪽 블럭만 가용 블럭인 경우
            if (bPrevPhysicalBlockMergeable)
            {
                extractTargetBlock = allocation.Block->PrevPhysicalBlock;
                mergeBaseBlock = allocation.Block->PrevPhysicalBlock;
                mergeTargetBlock = allocation.Block;
            }

            // Case 3: 뒷 쪽 블럭만 가용 블럭인 경우
            else if (bNextPhysicalBlockMergeable)
            {
                extractTargetBlock = allocation.Block->NextPhysicalBlock;
                mergeBaseBlock = allocation.Block;
                mergeTargetBlock = allocation.Block->NextPhysicalBlock;
            }

            Extract(extractTargetBlock);

            // assert; mergeBaseBlock != nullptr; mergeTargetBlock != nullptr
            // assert; mergeBaseBlock->Offset < mergeTargetBlock->Offset
            mergeBaseBlock->Size += mergeTargetBlock->Size;
            mergeBaseBlock->NextPhysicalBlock = mergeTargetBlock->NextPhysicalBlock;
            if (mergeTargetBlock->NextPhysicalBlock != nullptr)
            {
                mergeTargetBlock->NextPhysicalBlock->PrevPhysicalBlock = mergeBaseBlock;
            }
            mergeBaseBlock->bIsUsed = false;
            Insert(mergeBaseBlock);

            delete mergeTargetBlock;
            mergeTargetBlock = nullptr;
        }
        // Case 4: 양 쪽 모두 가용 블럭이 아닌 경우
        else
        {
            allocation.Block->bIsUsed = false;
            Insert(allocation.Block);
        }
    }

    void TLSFAllocator::Insert(TLSFBlock* block)
    {
        // assert(block != nullptr);
        // assert(block->Size > 0);
        // assert; block->NextFreeBlock == nullptr && block->PrevFreeBlock == nullptr
        const TLSFMappedLevels levels = MapLevels(block->Size, secondLevelParam);
        // assert; first level <= firstLevelParam
        firstLevelBitmap |= 1Ui64 << levels.FirstLevel;
        // assert; second level <= numSubdivisions
        secondLevelBitmaps[levels.FirstLevel] |= 1Ui64 << levels.SecondLevel;

        const size_t freeListIdx = levels.FirstLevel * numSubdivisions + levels.SecondLevel;
        if (freeLists[freeListIdx] != nullptr)
        {
            // assert; freeLists[freeListIdx]->PrevFreeBlock == nullptr
            freeLists[freeListIdx]->PrevFreeBlock = block;
        }
        freeLists[freeListIdx] = block;
    }

    TLSFBlock* TLSFAllocator::ExtractHead(const size_t firstLevelIdx, const size_t secondLevelIdx)
    {
        const uint64_t firstLevelBitmapMask = 1Ui64 << firstLevelIdx;
        if (!(firstLevelBitmap & firstLevelBitmapMask))
        {
            return nullptr;
        }

        const uint64_t secondLevelBitmapMask = 1Ui64 << secondLevelIdx;
        if (!(secondLevelBitmaps[firstLevelIdx] & secondLevelBitmapMask))
        {
            return nullptr;
        }

        const size_t freeListIdx = firstLevelIdx * numSubdivisions + secondLevelIdx;
        // assert; freeLists[freeListIdx] != nullptr
        if (freeLists[freeListIdx] == nullptr)
        {
            return nullptr;
        }

        TLSFBlock* extractedBlock = freeLists[freeListIdx];
        if (extractedBlock->NextFreeBlock == nullptr)
        {
            secondLevelBitmaps[firstLevelIdx] ^= secondLevelBitmapMask;
            if (secondLevelBitmaps[firstLevelIdx] == 0)
            {
                firstLevelBitmap ^= firstLevelBitmapMask;
            }
        }
        else
        {
            extractedBlock->NextFreeBlock->PrevFreeBlock = nullptr;
        }

        freeLists[freeListIdx] = extractedBlock->NextFreeBlock;
        return extractedBlock;
    }

    void TLSFAllocator::Extract(TLSFBlock* block)
    {
        // assert; block != nullptr
        // assert; block->Size > 0
        const TLSFMappedLevels levels = MapLevels(block->Size, secondLevelParam);
        const size_t freeListIdx = levels.FirstLevel * numSubdivisions + levels.SecondLevel;
        if (block == freeLists[freeListIdx])
        {
            // assert; block->PrevFreeBlock == nullptr
            if (block->NextFreeBlock == nullptr)
            {
                secondLevelBitmaps[levels.FirstLevel] ^= (1Ui64 << levels.SecondLevel);
                if (secondLevelBitmaps[levels.FirstLevel] == 0)
                {
                    firstLevelBitmap ^= (1Ui64 << levels.FirstLevel);
                }
                freeLists[freeListIdx] = nullptr;
            }
            else
            {
                block->NextFreeBlock->PrevFreeBlock = nullptr;
                freeLists[freeListIdx] = block->NextFreeBlock;
            }
        }
        else
        {
            if (block->NextFreeBlock != nullptr)
            {
                block->NextFreeBlock->PrevFreeBlock = block->PrevFreeBlock;
            }
            if (block->PrevFreeBlock != nullptr)
            {
                block->PrevFreeBlock->NextFreeBlock = block->NextFreeBlock;
            }
        }
    }
} // namespace gen3