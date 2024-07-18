#include <catch2/catch_session.hpp>
#include "TLSF3.h"

int main(int argc, char* argv[])
{
    gen3::TLSFAllocator allocator{ 2048 };
    gen3::TLSFAllocation alloc0 = allocator.Allocate(32);
    gen3::TLSFAllocation alloc1 = allocator.Allocate(128);
    gen3::TLSFAllocation alloc2 = allocator.Allocate(256);
    allocator.Deallocate(alloc1);
    allocator.Deallocate(alloc2);
    allocator.Deallocate(alloc0);
    return 0;

    //int result = Catch::Session().run(argc, argv);
    //system("PAUSE");
    //return result;
}