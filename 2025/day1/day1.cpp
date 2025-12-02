#include <cstdarg>
#include <fstream>
#include <iomanip>
#include <iostream>
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
enum class Direction
{
    Invalid,
    Left,
    Right
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static Direction CharToDirection(char inChar)
{
    switch (inChar)
    {
        case 'l': 
        case 'L': 
            return Direction::Left;
        case 'r':
        case 'R':
            return Direction::Right;
    }

    return Direction::Invalid;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class Dial
{
public:
    Dial(int initValue, int dialSize)
        : m_initValue(initValue)
        , m_dialSize(dialSize)
        , m_current(m_initValue)
        , m_resultPart1(0)
        , m_resultPart2(0) { }

    void rotate(Direction direction, int rotations)
    {
        if (rotations == 0)
            return;
            
        const int sign = direction == Direction::Right ? 1 : -1;
        const int total = (m_current + (sign * rotations));
        m_resultPart2 += total <= 0 ? (-total / m_dialSize) + (m_current != 0 ? 1 : 0) : (total / m_dialSize);
        int candidate = total % m_dialSize;
        m_current = candidate < 0 ? m_dialSize + candidate : candidate;
        if (m_current == 0)
             ++m_resultPart1;
        LogDebug("  now current: %d, result: %d", m_current, result());
    }

    int result() const { return m_resultPart2; }

private:
    const int m_initValue;
    const int m_dialSize;
    int m_current;
    int m_resultPart1;
    int m_resultPart2;
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

    Dial d(50, 100);

    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
            break;

        const Direction direction = CharToDirection(currentChar);
        if (direction == Direction::Invalid)
        {
            LogError("Invalid direction read %c", currentChar);
            return 1;
        }
        int rotations = 0;
        currentChar = file.get();
        while (std::isdigit(currentChar))
        {
            int asInt = currentChar - '0';
            rotations = rotations * 10 + asInt;
            currentChar = file.get();
        }
        LogDebug("Read %s%d", direction == Direction::Left ? "L" : "R", rotations);
        d.rotate(direction, rotations);
    }
    while (file);

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result ==> " << d.result() << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
