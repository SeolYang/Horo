#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <execution>

class Solution
{
public:
    static void Solve()
    {
        size_t numCases = 0;
        std::cin >> numCases;

        std::vector<std::string> resultsList{};
        std::vector<size_t> scores{};
        resultsList.resize(numCases);
        scores.resize(numCases);
        while (numCases > 0)
        {
            --numCases;
            std::cin >> resultsList[numCases];
        }

        std::transform(std::execution::par, resultsList.begin(), resultsList.end(), scores.begin(),
                       [](const std::string& results)
                       {
                           size_t scoreSum = 0;
                           size_t streak = 0;
                           for (const char result : results)
                           {
                               const bool bIsCorrect = result == 'O';
                               if (bIsCorrect)
                               {
                                   ++streak;
                                   scoreSum += streak;
                               }
                               else
                               {
                                   streak = 0;
                               }
                           }

                           return scoreSum;
                       });

        for (auto scoreItr = scores.rbegin(); scoreItr != scores.rend(); ++scoreItr)
        {
            std::cout << (*scoreItr) << std::endl;
        }
    }
};
