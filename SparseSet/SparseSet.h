#pragma once
#include <ranges>

template <typename Ty>
class SparseSet
{
private:
    struct Node
    {
        size_t SparseIdx;
        Ty Element;
    };

    constexpr static size_t InvalidIndex = 0xffffffffffffffffUi64;
    constexpr static size_t Tombstone = 0xffffffffffffffffUi64;
    constexpr static size_t InitialSparseSize = 4;

public:
    SparseSet() = default;
    SparseSet(const SparseSet&) = default;
    SparseSet(SparseSet&&) noexcept = default;
    ~SparseSet() = default;

    SparseSet& operator=(const SparseSet&) = default;
    SparseSet& operator=(SparseSet&&) noexcept = default;

    template <typename... Args>
    [[nodiscard]] size_t Emplace(Args&&... args)
    {
        if (freeSparseSlotIdx.empty() && !Grow())
        {
            return InvalidIndex;
        }

        const size_t newSparseSlotIdx = freeSparseSlotIdx.back();
        freeSparseSlotIdx.pop_back();
        assert(newSparseSlotIdx != InvalidIndex);
        const size_t newDenseSlotIdx = dense.size();
        sparse[newSparseSlotIdx] = newDenseSlotIdx;
        dense.emplace_back(Node{ .SparseIdx = newSparseSlotIdx, .Element = Ty{ std::forward<Args>(args)... } });

        return newSparseSlotIdx;
    }

    void Erase(const size_t sparseIdx)
    {
        if (sparseIdx >= sparse.size())
        {
            return;
        }

        const size_t denseSlotIdx = sparse[sparseIdx];
        if (denseSlotIdx == Tombstone || denseSlotIdx >= dense.size())
        {
            return;
        }

        if (dense.size() > 1)
        {
            sparse[dense.back().SparseIdx] = denseSlotIdx;
            std::swap(dense[denseSlotIdx], dense.back());
        }
        sparse[sparseIdx] = Tombstone;
        dense.pop_back();
    }

    Ty* TryGet(const size_t sparseIdx)
    {
        if (sparseIdx >= sparse.size())
        {
            return nullptr;
        }

        const size_t denseSlotIdx = sparse[sparseIdx];
        if (denseSlotIdx == Tombstone || denseSlotIdx >= dense.size())
        {
            return nullptr;
        }

        return &dense[denseSlotIdx].Element;
    }

    const Ty* TryGet(const size_t sparseIdx) const
    {
        if (sparseIdx >= sparse.size())
        {
            return nullptr;
        }

        const size_t denseSlotIdx = sparse[sparseIdx];
        if (denseSlotIdx == Tombstone || denseSlotIdx >= dense.size())
        {
            return nullptr;
        }

        return &dense[denseSlotIdx].Element;
    }

    Ty& Get(const size_t sparseIdx)
    {
        return *TryGet(sparseIdx);
    }

    const Ty& Get(const size_t sparseIdx) const
    {
        return *TryGet(sparseIdx);
    }

    [[nodiscard]] size_t GetSize() const noexcept { return dense.size(); }

private:
    [[nodiscard]] bool Grow()
    {
        const size_t oldSize = sparse.size();
        if (oldSize >= InvalidIndex)
        {
            return false;
        }

        const size_t newSize = std::min(sparse.empty() ? InitialSparseSize : oldSize * 2, InvalidIndex);
        freeSparseSlotIdx.reserve(newSize);
        sparse.reserve(newSize);

        for (const size_t newSlotIdx : (std::views::iota(oldSize, newSize) | std::views::reverse))
        {
            freeSparseSlotIdx.emplace_back(newSlotIdx);
            sparse.emplace_back(Tombstone);
        }

        return true;
    }

private:
    std::vector<size_t> freeSparseSlotIdx{};
    std::vector<size_t> sparse{};
    std::vector<Node> dense{};
};
