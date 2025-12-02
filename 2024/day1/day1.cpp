#include <algorithm>
#include <charconv>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#define LogError(...) Error(__FILE__, __LINE__, __VA_ARGS__)

static void Error(const char* inFile, const int inLine, const char* inErrorMessage)
{
    std::cout << inFile << "(" << inLine << ") ERROR: " << inErrorMessage << std::endl;
}

int main()
{
    constexpr const char* fileName = "input";
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogError("Couldn't find file");
        return 1;
    }

    std::vector<int> list1;
    std::vector<int> list2;
    
    std::string line;
    do
    {
        std::getline(file, line);
        int first = 0, second = 0;

        size_t startSecond = 0;

        // first
        {
            size_t start = 0;
            while (start < line.size() && line[start] == ' ')
                ++start;
            size_t end = start;
            while (end < line.size() && line[end] != ' ')
                ++end;
            std::from_chars_result result = std::from_chars(line.data() + start, line.data() + end, first);
            if (result.ec != std::errc())
            {
                LogError("Couldn't parse line");
                return 1;
            }

            startSecond = end + 1;
        }

        // second
        {
            size_t start = startSecond;
            while (start < line.size() && line[start] == ' ')
                ++start;
            size_t end = start;
            while (end < line.size() && line[end] != ' ')
                ++end;
            std::from_chars_result result = std::from_chars(line.data() + start, line.data() + end, second);
            if (result.ec != std::errc())
            {
                LogError("Couldn't parse line");
                return 1;
            }
        }
        list1.push_back(first);
        list2.push_back(second);
    }
    while (file && file.peek() != EOF);

    if (list1.size() != list2.size())
    {
        LogError("Lists are not of the same size!");
        return 1;
    }

    std::sort(list1.begin(), list1.end());
    std::sort(list2.begin(), list2.end());


    // part1 result
    {
        int sumDistances = 0;
        for (size_t i = 0; i < list1.size(); ++i)
        {
            int left = list1[i];
            int right = list2[i];
            int distance = left > right ? left - right : right - left;
            sumDistances += distance;
        }

        std::cout << "sumDistances => " << sumDistances << std::endl;
    }

    // part2
    {
        int similarityScore = 0;
        size_t previousRightIndex = 0;
        for (size_t i = 0; i < list1.size(); ++i)
        {
            int left = list1[i];
            size_t rightIndex = previousRightIndex;
            do
            {
                int right = list2[rightIndex];

                if (right < left)
                {
                    ++rightIndex;
                }
                else if (right == left)
                {
                    ++rightIndex;
                    similarityScore += left;
                }
                else
                {
                    break;
                }
            }
            while (rightIndex < list2.size());

            if ((i + 1) < list1.size() && list1[i + 1] > left)
            {
                previousRightIndex = rightIndex;
            }
        }

        std::cout << "similarityScore => " << similarityScore << std::endl;
    }

    return EXIT_SUCCESS;
}
