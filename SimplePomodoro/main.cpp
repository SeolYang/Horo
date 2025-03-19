#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <format>

int main()
{
    /* 에러 별도 처리 X */
    constexpr std::string_view kConfigFilePath = "config.cfg";
    const auto minToMs = [](size_t& targetTime)
    {
        targetTime *= (60Ui64 * 1000Ui64);
    };

    const bool bConfigFileExists = std::filesystem::exists(kConfigFilePath);
    while (true)
    {
        size_t concentrateTime = 0;
        size_t reviewTime = 0;
        size_t shortDefocusTime = 0;
        size_t longDefocusTime = 0;
        size_t numSets = 0;
        size_t numRepeats = 0;

        std::cout << "1 세트 = 집중 + 복습 + 짧은 휴식" << std::endl;
        std::cout << "1 반복 = N 세트 + 긴 휴식(마지막 세트 짧은 휴식 시간 제외)" << std::endl;

        bool bShouldResetConfigFile = true;
        if (bConfigFileExists)
        {
            std::cout << "설정 파일 존재. 재설정을 위해선 'r' 입력. 그 외엔 설정 파일을 사용해 그대로 진행." << std::endl;
            std::string resetCofigInput;
            std::cin >> resetCofigInput;
            if (resetCofigInput.length() == 1 && (resetCofigInput[0] == 'r' || resetCofigInput[0] == 'R'))
            {
                bShouldResetConfigFile = true;
            }
            else
            {
                bShouldResetConfigFile = false;
            }
        }

        if (!bShouldResetConfigFile)
        {
            std::ifstream configInputStream{
                kConfigFilePath.data(),
                std::ios_base::in | std::ios_base::binary
            };

            if (configInputStream.is_open())
            {
                configInputStream.read((char*)&concentrateTime, sizeof(concentrateTime));
                configInputStream.read((char*)&reviewTime, sizeof(reviewTime));
                configInputStream.read((char*)&shortDefocusTime, sizeof(shortDefocusTime));
                configInputStream.read((char*)&longDefocusTime, sizeof(longDefocusTime));
                configInputStream.read((char*)&numSets, sizeof(numSets));

                std::cout << "설정 파일 읽어오기 성공." << std::endl;
                std::cout << "집중 할 시간(min): " << concentrateTime << std::endl;
                std::cout << "복습 시간(min): " << reviewTime << std::endl;
                std::cout << "짧은 휴식 시간(min): " << shortDefocusTime << std::endl;
                std::cout << "긴 휴식 시간(min): " << longDefocusTime << std::endl;
                std::cout << "반복 당 세트 수: " << numSets << std::endl;
            }
            else
            {
                std::cout << "설정 파일 읽어오기 실패. 재설정 필요!" << std::endl;
                bShouldResetConfigFile = true;
            }
            // Close
        }

        if (bShouldResetConfigFile)
        {
            std::cout << "집중 할 시간(min): ";
            std::cin >> concentrateTime;
            std::cout << "복습 시간(min): ";
            std::cin >> reviewTime;
            std::cout << "짧은 휴식 시간(min): ";
            std::cin >> shortDefocusTime;
            std::cout << "긴 휴식 시간(min): ";
            std::cin >> longDefocusTime;
            std::cout << "반복 당 세트 수: ";
            std::cin >> numSets;

            std::ofstream configOutputStream{
                kConfigFilePath.data(),
                std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
            };
            if (configOutputStream.is_open())
            {
                configOutputStream.write((char*)&concentrateTime, sizeof(concentrateTime));
                configOutputStream.write((char*)&reviewTime, sizeof(reviewTime));
                configOutputStream.write((char*)&shortDefocusTime, sizeof(shortDefocusTime));
                configOutputStream.write((char*)&longDefocusTime, sizeof(longDefocusTime));
                configOutputStream.write((char*)&numSets, sizeof(numSets));
            }
            // Flush & Close
        }

        std::cout << "반복 횟수: ";
        std::cin >> numRepeats;

        const size_t totalTime = ((concentrateTime + reviewTime) * numSets * numRepeats) +
                                 (shortDefocusTime * (numSets - 1) * numRepeats) +
                                 (longDefocusTime * numRepeats);
        std::cout << "예상 소요 시간: " << (totalTime / 60) << "시간 " << (totalTime % 60) << "분" << std::endl;

        minToMs(concentrateTime);
        minToMs(reviewTime);
        minToMs(shortDefocusTime);
        minToMs(longDefocusTime);

        size_t elapsedTime = 0;
        size_t repeatIdx = 0;
        while (repeatIdx < numRepeats)
        {
            const std::string perRepeatMessage = std::format("반복({}/{})", (repeatIdx + 1), numRepeats);
            size_t setIdx = 0;
            while (setIdx < numSets)
            {
                const std::string perSetMessage = std::format("{}.세트({}/{})", perRepeatMessage, (setIdx + 1), numSets);
                std::cout << std::format("{}.집중구간 시작!", perSetMessage) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds{ concentrateTime });
                std::cout << std::format("{}.집중구간 완료!", perSetMessage) << std::endl;
                PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);

                std::cout << std::format("{}..리뷰구간 시작!", perSetMessage) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds{ reviewTime });
                PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
                std::cout << std::format("{}..리뷰구간 완료!", perSetMessage) << std::endl;

                if (setIdx < (numSets - 1))
                {
                    std::cout << std::format("{}..휴식구간 시작!", perSetMessage) << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds{ shortDefocusTime });
                    PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
                    std::cout << std::format("{}..휴식구간 완료!", perSetMessage) << std::endl;
                }

                ++setIdx;
            }

            std::cout << std::format("{}.휴식구간 시작!", perRepeatMessage) << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds{ longDefocusTime });
            PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
            std::cout << std::format("{}.휴식구간 완료!", perRepeatMessage) << std::endl;
            ++repeatIdx;
        }

        std::string anyInput;
        std::cout << "재시작 하기 위해선 아무키 입력: ";
        std::cin >> anyInput;
    }
}