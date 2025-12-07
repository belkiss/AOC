#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#define ENABLE_LOG 1
#if ENABLE_LOG
    #define LogDebug(inFormat, ...) Debug(__FILE__, __LINE__, inFormat, ##__VA_ARGS__)
    #define LogInfo(inFormat, ...) Info(__FILE__, __LINE__, inFormat, ##__VA_ARGS__)
    #define LogError(inFormat, ...) Error(__FILE__, __LINE__, inFormat, ##__VA_ARGS__)
#else
    #define LogDebug(inFormat, ...) do { } while(0)
    #define LogInfo(inFormat, ...) do { } while(0)
    #define LogError(inFormat, ...) do { } while(0)
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
__attribute__((__format__ (__printf__, 3, 4)))
static void Debug(const char* inFile, const int inLine, const char* inFormat, ...)
{
    printf("%s(%3d): \033[34mDEBUG\033[0m: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
__attribute__((__format__ (__printf__, 3, 4)))
static void Info(const char* inFile, const int inLine, const char* inFormat, ...)
{
    printf("%s(%3d): \033[33mINFO\033[0m: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
__attribute__((__format__ (__printf__, 3, 4)))
static void Error(const char* inFile, const int inLine, const char* inFormat, ...)
{
    printf("%s(%3d): \033[31mERROR\033[0m: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class FreshIngredients
{
public:
    void addRange(uint64_t start, uint64_t end)
    {
        m_ranges.emplace_back(start, end);
    }

    bool isFresh(uint64_t id) const
    {
        for (const auto& range : m_ranges)
        {
            if (id >= range.first && id <= range.second)
                return true;
        }
        
        return false;
    }

    uint64_t numFreshRanges() const
    {
        std::vector<std::pair<uint64_t,uint64_t>> rangesNoOverlaps;
        auto sorted = m_ranges;
        std::sort(sorted.begin(), sorted.end(), [](auto a, auto b) { return a.first < b.first; });

        for (auto& range : sorted)
        {
            if (!rangesNoOverlaps.empty())
            {
                auto& previous = rangesNoOverlaps.back();
                if (range.first <= previous.second && range.first >= previous.first)
                {
                    if (range.second <= previous.second)
                    {
                        // this range is included in the previous, skipping
                        continue;
                    }
                    else
                    {
                        // this range upperbound is larger than previous, updating
                        previous.second = range.second;
                        continue;
                    }
                }
            }
            rangesNoOverlaps.emplace_back(range);
        }
        
        uint64_t result = 0;
        for (auto& range : rangesNoOverlaps)
        {
            result += range.second - range.first + 1;
        }

        return result;
    }

private:
    std::vector<std::pair<uint64_t,uint64_t>> m_ranges;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main()
{
    const bool useSmallInput = false;
    constexpr const char* fileName = useSmallInput ? "smallInput" : "input";
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogError("Couldn't find file '%s'", fileName);
        return 1;
    }

    uint64_t numFresh = 0;
    FreshIngredients ingredients;
    bool secondPart = false;
    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
            break;

        if (currentChar == '\r')
            continue;

        if (currentChar == '\n')
        {
            secondPart = true;
            continue;
        }

        auto readNumber = [&] {
            uint64_t number = 0;
            while (std::isdigit(currentChar))
            {
                int asInt = currentChar - '0';
                constexpr auto max = std::numeric_limits<uint64_t>::max();
                if (number > (max / 10))
                {
                    LogError("Number is too big to fit in the type");
                    exit(-1);
                }
                number = number * 10 + asInt;
                currentChar = file.get();
            }
            return number;
        };
        
        if (!secondPart)
        {
            uint64_t start = readNumber();
            if (currentChar == '-')
            {
                currentChar = file.get();
            }
            uint64_t end = readNumber();
            ingredients.addRange(start, end);
        }
        else
        {
            uint64_t id = readNumber();
            if (ingredients.isFresh(id))
                ++numFresh;
        }
    }
    while (file);

    uint64_t result = ingredients.numFreshRanges();
    
    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result numFresh ==> " << numFresh << std::endl;
    std::cout << "Result numFreshRanges ==> " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
