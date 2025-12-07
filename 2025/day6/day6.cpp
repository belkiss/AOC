#include <algorithm>
#include <cassert>
#include <cmath>
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
enum class Operations : uint8_t
{
    Add = '+',
    Multiply = '*'
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class Problems
{
public:
    void addProblems(const std::vector<std::string>& problems, size_t maxLength)
    {
        for (int i = (int)maxLength - 1; i >= 0; --i)
        {
            bool foundDigit = false;
            uint64_t number = 0;
            for (auto it = problems.cbegin(); it != problems.cend(); ++it)
            {
                const std::string& problem = *it;
                const char currentChar = (size_t)i < problem.size() ? problem[i] : 0;
                if (std::isdigit(currentChar))
                {
                    foundDigit = true;
                    int asInt = currentChar - '0';
                    constexpr auto max = std::numeric_limits<uint64_t>::max();
                    if (number > (max / 10))
                    {
                        LogError("Number is too big to fit in the type");
                        exit(-1);
                    }
                    number = number * 10 + asInt;
                }
            }
            if (!foundDigit || m_inputs.empty())
                m_inputs.emplace_back();
            if (foundDigit)
            {
                m_inputs.back().emplace_back(number);
                number = 0;
            }
        }

        std::reverse(m_operations.begin(), m_operations.end());
    }

    void addOperation(Operations operation)
    {
        m_operations.emplace_back(operation);
    }
    
    uint64_t getResult() const
    {
        uint64_t total = 0;
        for (size_t i = 0, e = m_operations.size(); i < e; ++i)
        {
            uint64_t result = 0;
            switch (m_operations[i])
            {
                case Operations::Add:
                    for (const auto& inputValues : m_inputs[i])
                    {
                        result += inputValues;
                    }
                    break;
                case Operations::Multiply:
                    result = 1;
                    for (const auto& inputValues : m_inputs[i])
                    {
                        result *= inputValues;
                    }
                    break;
            }
            total += result;
        }
        return total;
    }

private:
    std::vector<std::vector<uint64_t>> m_inputs;
    std::vector<Operations> m_operations;
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

    Problems problems;
    size_t maxLength = 0;
    bool foundOperations = false;
    std::vector<std::string> lines;
    std::string line;
    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
            break;

        if (currentChar == '\r')
            continue;
        
        if (currentChar == '\n')
        {
            if (!line.empty())
            {
                maxLength = std::max(maxLength, line.length());
                lines.emplace_back(std::move(line));
                line.clear();
            }
            continue;
        }
        
        if (currentChar == (uint8_t)Operations::Add || currentChar == (uint8_t)Operations::Multiply)
        {
            foundOperations = true;
            problems.addOperation(static_cast<Operations>(currentChar));
            continue;
        }

        if (!foundOperations)
            line.push_back(currentChar);
    }
    while (file);
    
    problems.addProblems(lines, maxLength);

    const uint64_t result = problems.getResult();
    
    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result ==> " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
