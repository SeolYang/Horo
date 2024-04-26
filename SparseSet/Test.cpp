#include <catch2/catch_test_macros.hpp>
#include "SparseSet.h"

TEST_CASE("SparseSet")
{
    SparseSet<size_t> sparseSet{};
    constexpr size_t kNumElements = 1000;
    std::vector<size_t> sparseIndices;
    sparseIndices.reserve(kNumElements);
    for (const size_t element : std::views::iota(0Ui64, kNumElements))
    {
        sparseIndices.emplace_back(sparseSet.Emplace(element));
    }

    REQUIRE(sparseSet.GetSize() == kNumElements);
    REQUIRE(sparseSet.Get(sparseIndices[300]) == 300);
    sparseSet.Erase(sparseIndices[299]);
    REQUIRE(sparseSet.GetSize() == kNumElements - 1);
    REQUIRE(sparseSet.TryGet(sparseIndices[299]) == nullptr);
    REQUIRE(sparseSet.Get(sparseIndices[300]) == 300);
}