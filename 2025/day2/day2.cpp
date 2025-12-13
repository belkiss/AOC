#include <cstdarg>
#include <cstdint>
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
class Range
{
public:
    Range(uint64_t inStart, uint64_t inEnd)
        : m_start(inStart)
        , m_end(inEnd)
    {
    }

    uint64_t getInvalidIds() const
    {
        LogDebug("Range %llu-%llu", m_start, m_end);

        uint64_t result = 0;
        for (uint64_t i = m_start; i <= m_end; ++i)
        {
            if (isInvalid(i))
            {
                LogDebug("%llu is invalid", i);
                result += i;
            }
        }
        return result;
    }

private:
    static bool isInvalid(uint64_t inNumber)
    {
        if (inNumber == 0)
            return false;

        std::string asString = std::to_string(inNumber);
        for (int i = 1, e = asString.length(); (e / i) >= 2; ++i)
        {
            std::string seq = asString.substr(0, i);
            bool repeatingSeq = true;
            for (int testIndex = i; testIndex < e; testIndex += i)
            {
                for (size_t j = 0; j < seq.length(); ++j)
                {
                    if (seq[j] != asString[testIndex + j])
                    {
                        repeatingSeq = false;
                        break;
                    }
                }

                if (!repeatingSeq)
                {
                    break;
                }
            }
            if (repeatingSeq)
            {
                return true;
            }
        }

        return false;
    }

    static bool isInvalidTwice(uint64_t inNumber)
    {
        if (inNumber == 0)
            return false;

        std::string asString = std::to_string(inNumber);
        if (asString.length() % 2)
            return false;

        const int j = asString.length() / 2;
        for (int i = 0; i < j; ++i)
        {
            if (asString[i] != asString[i + j])
                return false;
        }

        return true;
    }

private:
    uint64_t m_start;
    uint64_t m_end;
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

    std::vector<Range> ranges;
    do
    {
        int currentChar = file.get();
        while (std::isspace(currentChar))
            currentChar = file.get();
        if (currentChar == EOF)
            break;

        auto readNumber = [&] {
            uint64_t number = 0;
            while (std::isdigit(currentChar))
            {
                int asInt = currentChar - '0';
                number = number * 10 + asInt;
                currentChar = file.get();
            }
            return number;
        };

        uint64_t start = readNumber();
        if (currentChar == '-')
            currentChar = file.get();
        uint64_t end = readNumber();

        ranges.emplace_back(start, end);
    }
    while (file);

    uint64_t result = 0;
    for (const auto& range : ranges)
    {
        result += range.getInvalidIds();
    }

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result ==> " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
