#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <execution>
#include <string>
#include <cassert>

class Solution
{
public:
    static void Solve()
    {
        size_t numBits = 0;
        std::cin >> numBits;
        std::string bitset{};
        bitset.reserve(numBits);
        std::cin >> bitset;

        size_t numOps = 0;
        for (size_t pos = bitset.size() - 1; pos > 0; --pos)
        {
            const char currBit = bitset[pos];
            // 만약 pos에서 bit 값이 '1'인 경우 홀수
            if (currBit == '1')
            {
                ++numOps;
                while (pos > 1)
                {
                    --pos;
                    if (bitset[pos] == '0')
                    {
                        bitset[pos] = '1';
                        ++pos;
                        break;
                    }
                }
            }
        }

        std::cout << numOps;
    }
};
