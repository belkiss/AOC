#include <charconv>
#include <cstdarg>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#define LogError(inFormat, ...) Error(__FILE__, __LINE__, inFormat, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void Error(const char* inFile, const int inLine, const char* inFormat, ...)
{
    printf("%s(%d): ERROR: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static std::string toString(const std::vector<int>& inValues, int wrapIndex = -1)
{
    std::string result;
    for (int i = 0, e = inValues.size(); i < e; ++i)
    {
        if (wrapIndex == i)
            result.push_back('[');
        result.append(std::to_string(inValues[i]));
        if (wrapIndex == i)
            result.push_back(']');
        if (i + 1 < e)
            result.push_back(' ');
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static bool isSafeReport(const std::vector<int>& inValues, bool inLog, bool inLogError)
{
    bool safeReport = true;

    int i = 0;
    bool increasing = false;
    int previousValue = 0;

    for (const int value : inValues)
    {
        if (++i == 1)
        {
            // do nothing except storing value at index 0
            previousValue = value;
            continue;
        }

        if (i == 2) // figure out if we increase or decrease at index 2
        {
            increasing = value > previousValue;
        }

        // levels are either all increasing or all decreasing.
        // two adjacent levels differ by at least one and at most three
        const int diff = increasing ? (value - previousValue) : (previousValue - value);
        if (diff < 1 || diff > 3)
        {
            safeReport = false;

            if (inLogError)
            {
                if (diff < 0)
                {
                    LogError("Current report '%s' is not safe because not all are %s!", toString(inValues).c_str(), increasing ? "increasing" : "decreasing");
                }
                else if (diff < 1)
                {
                    LogError("Current report '%s' is not safe because not all adjacent differ by at least 1: %d != %d", toString(inValues).c_str(), previousValue, value);
                }
                else if (diff > 3)
                {
                    LogError("Current report '%s' is not safe because not all adjacent differ by at most 3: %d != %d", toString(inValues).c_str(), previousValue, value);
                }
            }
            break;
        }

        previousValue = value;
    }
    if (safeReport && inLog)
    {
        std::cout << toString(inValues).c_str() << " => " << (increasing ? "increasing safe" : "decreasing safe") << std::endl;
    }
    return safeReport;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool isSafeReportWithTolerance(const std::vector<int>& inValues)
{
    for (int i = 0, e = inValues.size(); i < e; ++i)
    {
        std::vector<int> copy = inValues;
        copy.erase(copy.begin() + i);
        if (isSafeReport(copy, false, false))
        {
            std::cout << toString(inValues, i).c_str() << " is now safe after removal of " << i << std::endl;
            return true;
        }
    }

    LogError("Current report '%s' is not safe regardless of level removed!", toString(inValues).c_str());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main()
{
    const bool useSmallInput = false;
    constexpr const char* fileName = useSmallInput ? "smallInput" : "input";
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogError("Couldn't find file");
        return 1;
    }

    unsigned int safeReports = 0;
    unsigned int safeReportsWithTolerance = 0;

    std::string line;
    do
    {
        std::getline(file, line);
        auto splitView = std::views::split(line, ' ')
            | std::views::transform([](const auto& subrange) {
                int value = 0;
                std::from_chars_result result = std::from_chars(subrange.data(), subrange.data() + subrange.size(), value);
                if (result.ec != std::errc())
                {
                    LogError("Couldn't parse line");
                    return -1;
                }
                return value;
            });

        std::vector<int> values{splitView.begin(), splitView.end()};
        if (isSafeReport(values, true, false))
        {
            ++safeReports;
            ++safeReportsWithTolerance;
        }
        else if (isSafeReportWithTolerance(values))
        {
            ++safeReportsWithTolerance;
        }
    }
    while (file && file.peek() != EOF);

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Found " << safeReports << " safe report" << (safeReports == 1 ? "" : "s") << std::endl;
    std::cout << "Found " << safeReportsWithTolerance << " safe report" << (safeReportsWithTolerance == 1 ? " " : "s") << " with tolerance" << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
