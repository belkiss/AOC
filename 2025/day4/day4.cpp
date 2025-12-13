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
class RollsGrid
{
public:
    void setNumPerLine(uint32_t numPerLine) { m_numPerLine = numPerLine; }
    void setTotal(uint32_t total) { m_total = total; }

    void addRoll(uint32_t index) { m_sortedRolls.emplace_back(index); }

    uint32_t removeAccessible(uint32_t minSurrounding)
    {
        auto hasValueAtIndex = [this](int index) {
            if (index >= 0)
            {
                auto it = std::lower_bound(m_sortedRolls.begin(), m_sortedRolls.end(), index);
                if (it != m_sortedRolls.end() && *it == (uint32_t)index)
                    return true;
            }
            return false;
        };

        std::vector<uint32_t> toRemove;
        for (uint32_t i = 0; i < m_total; ++i)
        {
            if (!hasValueAtIndex(i))
                continue;

            const bool isOnLeftBorder = (i % m_numPerLine) == 0;
            const bool isOnRightBorder = !isOnLeftBorder && ((i + 1) % m_numPerLine) == 0;
            const bool isTopRow = i < m_numPerLine;
            const bool isBottom = i >= (m_total - m_numPerLine);
            const int nw = !isTopRow && !isOnLeftBorder ? (i - m_numPerLine - 1) : -1;
            const int n = !isTopRow ? (i - m_numPerLine) : -1;
            const int ne = !isTopRow && !isOnRightBorder ? (i - m_numPerLine + 1) : -1;
            const int w = !isOnLeftBorder ? (i - 1) : -1;
            const int e = !isOnRightBorder ? (i + 1) : -1;
            const int sw = !isBottom && !isOnLeftBorder ? (i + m_numPerLine - 1) : -1;
            const int s = !isBottom ? (i + m_numPerLine) : -1;
            const int se = !isBottom && !isOnRightBorder ? (i + m_numPerLine + 1) : -1;

            uint32_t nbSurrounding = 0;
            if (hasValueAtIndex(nw))
                ++nbSurrounding;
            if (hasValueAtIndex(n))
                ++nbSurrounding;
            if (hasValueAtIndex(ne))
                ++nbSurrounding;
            if (hasValueAtIndex(w))
                ++nbSurrounding;
            if (hasValueAtIndex(e))
                ++nbSurrounding;
            if (hasValueAtIndex(sw))
                ++nbSurrounding;
            if (hasValueAtIndex(s))
                ++nbSurrounding;
            if (hasValueAtIndex(se))
                ++nbSurrounding;

            if (nbSurrounding < minSurrounding)
                toRemove.push_back(i);
        }

        // TODO: optim: we don't really need to call lower_bound again here
        for (uint32_t indexToRemove : toRemove)
            m_sortedRolls.erase(std::lower_bound(m_sortedRolls.begin(), m_sortedRolls.end(), indexToRemove));

        LogDebug("Removed %zu", toRemove.size());
        return toRemove.size();
    }

private:
    std::vector<uint32_t> m_sortedRolls;
    uint32_t m_numPerLine;
    uint32_t m_total;
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

    RollsGrid grid;
    uint32_t index = 0;
    bool numPerLineSet = false;
    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
        {
            LogDebug("Total is %u", index);
            grid.setTotal(index);
            break;
        }

        if (currentChar == '\r')
            continue;

        if (currentChar == '\n')
        {
            if (!numPerLineSet)
            {
                numPerLineSet = true;
                grid.setNumPerLine(index);
                LogDebug("Width is %u", index);
            }
            continue;
        }

        if (currentChar == '@')
            grid.addRoll(index);
        ++index;
    }
    while (file);

    uint64_t result = 0;
    for (uint64_t removed = grid.removeAccessible(4); removed > 0; removed = grid.removeAccessible(4))
    {
        result += removed;
    }

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result ==> " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
