#include <catch2/catch_test_macros.hpp>
#include "TLSF3.h"

TEST_CASE("TLSF")
{
    gen3::TLSFAllocator allocator{ 2048 };
    gen3::TLSFAllocation allocation = allocator.Allocate(32);
    allocator.Deallocate(allocation);
}