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
#include <map>
#include <set>
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
__attribute__((__format__(__printf__, 3, 4)))
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
__attribute__((__format__(__printf__, 3, 4)))
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
__attribute__((__format__(__printf__, 3, 4)))
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
enum class Item
{
    Empty,
    Start,
    Beam,
    Splitter
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class TachyonManifold
{
public:
    void addStart(uint32_t index, bool newLine)
    {
        if (newLine)
            m_items.emplace_back();
        if (!m_items.back().emplace(index, Item::Start).second)
            LogError("Couldn't insert at index %u", index);
    }

    void addSplitter(uint32_t index, bool newLine)
    {
        if (newLine)
            m_items.emplace_back();
        if (!m_items.back().emplace(index, Item::Splitter).second)
            LogError("Couldn't insert at index %u", index);
    }

    void addSpaces()
    {
        m_items.emplace_back();
    }

    void print(uint32_t maxLength) const
    {
        for (const auto& items : m_items)
        {
            std::string s;
            int printed = 0;
            for (const auto& item : items)
            {
                for (uint32_t i = printed; i < item.first; ++i)
                    s += '.';
                switch (item.second)
                {
                case Item::Empty: assert(false); break;
                case Item::Start: s += 'S'; break;
                case Item::Beam: s += '|'; break;
                case Item::Splitter: s += '^'; break;
                }
                printed = item.first + 1;
            }
            for (uint32_t i = printed; i < maxLength; ++i)
                s += '.';
            LogDebug("%s", s.c_str());
        }
    }

    std::pair<uint64_t, uint64_t> getNumBeams(uint32_t maxLength)
    {
        print(maxLength);

        uint64_t result1 = 0;

        bool foundStart = false;
        uint32_t startIndex = 0;
        for (size_t i = 0, e = m_items.size(); i < e; ++i)
        {
            const auto& items = m_items[i];
            auto* next = (i + 1 < e) ? &m_items[i + 1] : nullptr;

            auto continueBeam = [next, maxLength](uint32_t inIndex) {
                if (!next)
                    return;

                if (next->find(inIndex) == next->end())
                {
                    next->emplace(inIndex, Item::Beam);
                }
                else
                {
                    auto& item = next->at(inIndex);
                    if (item == Item::Beam)
                    {
                        // do nothing
                    }
                    else if (item == Item::Empty)
                    {
                        // update
                        item = Item::Beam;
                    }
                    else if (item == Item::Splitter)
                    {
                        // check left and right
                        if (inIndex > 0)
                        {
                            if (next->find(inIndex - 1) == next->end())
                                next->emplace(inIndex - 1, Item::Beam);
                            else
                                next->at(inIndex - 1) = Item::Beam;
                        }

                        if (inIndex + 1 < maxLength)
                        {
                            if (next->find(inIndex + 1) == next->end())
                                next->emplace(inIndex + 1, Item::Beam);
                            else
                                next->at(inIndex + 1) = Item::Beam;
                        }
                    }
                }
            };

            for (const auto& item : items)
            {
                if (i == 0)
                {
                    if (item.second == Item::Start)
                    {
                        foundStart = true;
                        startIndex = item.first;
                        continueBeam(item.first);
                    }
                }
                else
                {
                    if (item.second == Item::Beam)
                    {
                        continueBeam(item.first);
                    }

                    if (item.second == Item::Splitter)
                    {
                        const auto& previous = m_items[i - 1];
                        if (previous.find(item.first) != previous.end() && previous.at(item.first) == Item::Beam)
                        {
                            ++result1;
                        }
                    }
                }
            }

            if (!foundStart)
            {
                LogError("Couldn't find the start of the manifold, exiting");
                exit(-1);
            }

            if (i + 1 == e)
            {
                break;
            }
        }

        print(maxLength);

        uint64_t result2 = numTimelines(1, startIndex, maxLength);

        return std::make_pair(result1, result2);
    }

    uint64_t numTimelines(const uint32_t level, const uint32_t index, const uint32_t maxLength)
    {
        const size_t e = m_items.size();
        for (size_t i = e; i > 1; --i)
        {
            const uint32_t level = i - 1;

            const auto& items = m_items[level];
            auto* next = level + 1 < e ? &m_items[level + 1] : nullptr;
            for (const auto& item : items)
            {
                uint64_t value = 0;
                if (item.second == Item::Beam)
                {
                    if (next)
                    {
                        const auto found = next->find(item.first);
                        if (found != next->end())
                        {
                            if (found->second == Item::Beam)
                            {
                                value += m_cachedResults.at(std::make_pair(level + 1, item.first));
                            }
                            else if (found->second == Item::Splitter)
                            {
                                // left
                                if (item.first > 0)
                                    value += m_cachedResults.at(std::make_pair(level + 1, item.first - 1));

                                // right
                                if (item.first + 1 < maxLength)
                                    value += m_cachedResults.at(std::make_pair(level + 1, item.first + 1));
                            }
                        }
                    }
                    else
                    {
                        value = 1;
                    }
                }
                if (value != 0)
                {
                    m_cachedResults[std::make_pair(level, item.first)] = value;
                }
            }
        }

        auto it = m_cachedResults.find(std::make_pair(level, index));
        if (it != m_cachedResults.end())
        {
            return it->second;
        }

        return 0;
    }

private:
    std::vector<std::map<uint32_t, Item>> m_items;
    std::map<std::pair<uint32_t, uint32_t>, uint64_t> m_cachedResults;
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

    TachyonManifold manifold;
    bool newLine = true;
    uint32_t charNumber = 0;
    uint32_t maxLength = 0;
    do
    {
        auto getChar = [&] {
            ++charNumber;
            return file.get();
        };

        int currentChar = getChar();
        if (currentChar == EOF)
            break;

        if (currentChar == '\r')
            continue;

        if (currentChar == '\n')
        {
            maxLength = std::max(maxLength, charNumber - 1);
            newLine = true;
            charNumber = 0;
            continue;
        }

        if (currentChar == 'S')
            manifold.addStart(charNumber - 1, newLine);
        else if (currentChar == '^')
            manifold.addSplitter(charNumber - 1, newLine);
        else if (newLine && currentChar == '.')
            manifold.addSpaces();
        newLine = false;
    }
    while (file);

    const auto result = manifold.getNumBeams(maxLength);

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Results ==> " << result.first << ", " << result.second << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
