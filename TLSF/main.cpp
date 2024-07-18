#include <catch2/catch_session.hpp>
#include "TLSF3.h"

int main(int argc, char* argv[])
{
    gen3::TLSFAllocator allocator{ 2048 };
    gen3::TLSFAllocation allocation = allocator.Allocate(32);
    allocator.Deallocate(allocation);
    return 0;

    //int result = Catch::Session().run(argc, argv);
    //system("PAUSE");
    //return result;
}