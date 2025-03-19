#pragma once
// https://www.acmicpc.net/problem/10809

#include <iostream>
#include <string>
#include <array>
#include <cassert>

class Solution
{
public:
    static void Solve()
    {
        constexpr char InvalidCounter = -1;
        std::string input;
        std::cin >> input;
        constexpr char MinChar = 'a';
        constexpr char MaxChar = 'z';
        constexpr char NumChars = MaxChar - MinChar + 1;
        std::array<int, NumChars> counts{};
        std::fill(counts.begin(), counts.end(), InvalidCounter);

        size_t offset = 0;
        for (const char targetChar : input)
        {
            assert(targetChar >= MinChar && targetChar <= MaxChar);
            const size_t idxOfTargetChar = (size_t)targetChar - MinChar;
            assert(idxOfTargetChar >= 0 && idxOfTargetChar < NumChars);
            
            const bool bIsFirstTimeFound = counts[idxOfTargetChar] == InvalidCounter;
            if (bIsFirstTimeFound)
            {
                counts[idxOfTargetChar] = offset;
            }
            ++offset;
        }

        for (int count : counts)
        {
            std::cout << count << ' ';
        }
    }
};
