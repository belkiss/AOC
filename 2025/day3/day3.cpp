#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// clang-format off
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
static void Error(const char* inFile, const int inLine, const char* inFormat, ...)
{
    printf("%s(%3d): \033[31mERROR\033[0m: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}
// clang-format on

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BatteryBank
{
public:
    void addBattery(uint8_t batteryJoltage) { m_batteries.emplace_back(batteryJoltage); }
    uint64_t computeLargestJoltage2() const
    {
        int indexLarge1 = 0;
        int indexLarge2 = 1;
        auto computeCandidate = [this](int index1, int index2) -> uint64_t {
            return m_batteries[index1] * 10ull + m_batteries[index2];
        };
        uint64_t candidate = computeCandidate(indexLarge1, indexLarge2);

        for (int i = 1, e = m_batteries.size(); i < e; ++i)
        {
            uint8_t current = m_batteries[i];
            LogDebug("battery %u, %d,%d => %u%u", current, indexLarge1, indexLarge2, m_batteries[indexLarge1],
                m_batteries[indexLarge2]);
            if (current > m_batteries[indexLarge1] && (i + 1) < e)
            {
                indexLarge1 = i;
                indexLarge2 = i + 1;
            }
            else if (current > m_batteries[indexLarge2] && computeCandidate(indexLarge1, i) > candidate)
            {
                indexLarge2 = i;
            }
            candidate = computeCandidate(indexLarge1, indexLarge2);
        }
        return candidate;
    }

    uint64_t computeLargestJoltage(int numToConsider) const
    {
        std::vector<uint8_t> indexes;
        indexes.resize(numToConsider);

        const int e = m_batteries.size();
        for (int index = 0; index < numToConsider; ++index)
        {
            indexes[index] = index == 0 ? 0 : (indexes[index - 1] + 1);
            for (int i = indexes[index] + 1; i < (e - (numToConsider - 1 - index)); ++i)
            {
                uint8_t current = m_batteries[i];
                if (current > m_batteries[indexes[index]])
                    indexes[index] = i;
            }
        }

        uint64_t result = 0;
        for (int i = 0; i < numToConsider; ++i)
        {
            result *= 10;
            result += m_batteries[indexes[i]];
        }

        return result;
    }
    size_t numBatteries() const { return m_batteries.size(); }
    std::string toString() const
    {
        std::string result;
        for (uint8_t battery : m_batteries)
            result += char(battery + '0');
        return result;
    }

private:
    std::vector<uint8_t> m_batteries;
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

    std::vector<BatteryBank> banks;
    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
            break;

        if (currentChar == '\r')
            continue;

        if (currentChar == '\n')
        {
            LogDebug("Finished bank, %zu batteries", banks.back().numBatteries());
            banks.emplace_back();
            continue;
        }

        if (std::isdigit(currentChar))
        {
            if (banks.empty())
                banks.emplace_back();
            uint8_t asInt = currentChar - '0';
            banks.back().addBattery(asInt);
        }
        else
        {
            LogError("Found invalid char '%d'", currentChar);
        }
    }
    while (file);

    uint64_t result = 0;
    for (const auto& bank : banks)
    {
        if (bank.numBatteries() < 2)
            continue;

        uint64_t largest = bank.computeLargestJoltage(12);
        LogDebug("bank %s, %zu", bank.toString().c_str(), largest);
        result += largest;
    }

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result ==> " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
