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
class JunctionBoxes
{
    struct Circuit;
    struct JunctionBox
    {
        uint32_t m_x = 0;
        uint32_t m_y = 0;
        uint32_t m_z = 0;
        Circuit* m_circuit = nullptr;

        double squaredDistance(const JunctionBox& another) const
        {
            const double result = pow((double)another.m_z - m_z, 2) + pow((double)another.m_y - m_y, 2) +
                                  pow((double)another.m_x - m_x, 2);
            return result;
        }
    };

    struct Circuit
    {
        std::vector<JunctionBox*> m_connectedBoxes;
    };

    struct DistanceData
    {
        DistanceData(double d, JunctionBox* f, JunctionBox* s)
            : m_distance(d)
            , m_first(f)
            , m_second(s)
        {
        }

        double m_distance = 0;
        JunctionBox* m_first = nullptr;
        JunctionBox* m_second = nullptr;
    };

public:
    void addBox(uint32_t x, uint32_t y, uint32_t z) { m_boxes.emplace_back(x, y, z); }

    std::pair<uint64_t, uint64_t> connect(uint32_t numPairsToConnect) const
    {
        uint64_t result1 = getLargestCircuits(numPairsToConnect);
        uint64_t result2 = getSingleCircuit();
        return std::make_pair(result1, result2);
    }

    uint64_t getLargestCircuits(uint32_t numPairsToConnect) const
    {
        std::vector<JunctionBox> boxes = m_boxes;
        std::vector<DistanceData> distances = getShortestSortedDistances(numPairsToConnect, boxes);

        std::vector<Circuit*> circuits;
        for (auto& distance : distances)
        {
            if (distance.m_first->m_circuit == nullptr && distance.m_second->m_circuit == nullptr)
            {
                circuits.emplace_back(new Circuit);
                Circuit* circuit = circuits.back();
                distance.m_first->m_circuit = circuit;
                distance.m_second->m_circuit = circuit;
                circuit->m_connectedBoxes.emplace_back(distance.m_first);
                circuit->m_connectedBoxes.emplace_back(distance.m_second);
            }
            else if (distance.m_first->m_circuit == nullptr && distance.m_second->m_circuit != nullptr)
            {
                distance.m_first->m_circuit = distance.m_second->m_circuit;
                distance.m_second->m_circuit->m_connectedBoxes.emplace_back(distance.m_first);
            }
            else if (distance.m_first->m_circuit != nullptr && distance.m_second->m_circuit == nullptr)
            {
                distance.m_second->m_circuit = distance.m_first->m_circuit;
                distance.m_first->m_circuit->m_connectedBoxes.emplace_back(distance.m_second);
            }
            else if (distance.m_first->m_circuit != distance.m_second->m_circuit)
            {
                // both were part of circuits, but not the same, we need to merge
                // we choose the one with the smallest amount of connections, to minimize needed updates
                Circuit* sourceCircuit = (Circuit*)distance.m_first->m_circuit;
                Circuit* targetCircuit = (Circuit*)distance.m_second->m_circuit;
                if (sourceCircuit->m_connectedBoxes.size() > targetCircuit->m_connectedBoxes.size())
                {
                    sourceCircuit = (Circuit*)distance.m_second->m_circuit;
                    targetCircuit = (Circuit*)distance.m_first->m_circuit;
                }

                for (auto* box : sourceCircuit->m_connectedBoxes)
                {
                    box->m_circuit = targetCircuit;
                    targetCircuit->m_connectedBoxes.push_back(box);
                }
                auto found = std::find(circuits.begin(), circuits.end(), sourceCircuit);
                if (found != circuits.end())
                {
                    circuits.erase(found);
                    delete sourceCircuit;
                }
                else
                {
                    LogError("Couldn't find the circuit in the vector!");
                    exit(-1);
                }
            }
            else
            {
                // Already part of the same circuit
            }
        }

        uint64_t result = 0;
        std::sort(circuits.begin(), circuits.end(),
            [](const auto& a, const auto& b) { return a->m_connectedBoxes.size() > b->m_connectedBoxes.size(); });

        for (size_t i = 0; i < circuits.size() && i < 3; ++i)
        {
            if (result == 0)
                result = 1;
            const auto& circuit = circuits[i];
            result *= circuit->m_connectedBoxes.size();
        }

        for (const auto& circuit : circuits)
            delete circuit;

        return result;
    }

    uint64_t getSingleCircuit() const
    {
        uint64_t result = 0;

        std::vector<JunctionBox> boxes = m_boxes;
        std::vector<DistanceData> distances = getAllSortedDistances(boxes);

        std::vector<Circuit*> circuits;
        for (auto& distance : distances)
        {
            if (distance.m_first->m_circuit == nullptr && distance.m_second->m_circuit == nullptr)
            {
                circuits.emplace_back(new Circuit);
                Circuit* circuit = circuits.back();
                distance.m_first->m_circuit = circuit;
                distance.m_second->m_circuit = circuit;
                circuit->m_connectedBoxes.emplace_back(distance.m_first);
                circuit->m_connectedBoxes.emplace_back(distance.m_second);
            }
            else if (distance.m_first->m_circuit == nullptr && distance.m_second->m_circuit != nullptr)
            {
                distance.m_first->m_circuit = distance.m_second->m_circuit;
                distance.m_second->m_circuit->m_connectedBoxes.emplace_back(distance.m_first);
            }
            else if (distance.m_first->m_circuit != nullptr && distance.m_second->m_circuit == nullptr)
            {
                distance.m_second->m_circuit = distance.m_first->m_circuit;
                distance.m_first->m_circuit->m_connectedBoxes.emplace_back(distance.m_second);
            }
            else if (distance.m_first->m_circuit != distance.m_second->m_circuit)
            {
                // both were part of circuits, but not the same, we need to merge

                // we choose the one with the smallest amount of connections, to minimize needed updates
                Circuit* sourceCircuit = distance.m_first->m_circuit;
                Circuit* targetCircuit = distance.m_second->m_circuit;

                for (auto* box : sourceCircuit->m_connectedBoxes)
                {
                    box->m_circuit = targetCircuit;
                    targetCircuit->m_connectedBoxes.push_back(box);
                }
                auto found = std::find(circuits.begin(), circuits.end(), sourceCircuit);
                if (found != circuits.end())
                {
                    circuits.erase(found);
                    delete sourceCircuit;
                }
                else
                {
                    LogError("Couldn't find the circuit in the vector!");
                    exit(-1);
                }
            }
            else
            {
                // Already part of the same circuit
            }

            if (circuits.size() == 1 && circuits.front()->m_connectedBoxes.size() == m_boxes.size())
            {
                result = (uint64_t)distance.m_first->m_x * (uint64_t)distance.m_second->m_x;
                break;
            }
        }

        for (const auto& circuit : circuits)
            delete circuit;

        return result;
    }

private:
    std::vector<DistanceData> getAllSortedDistances(std::vector<JunctionBox>& boxes) const
    {
        std::vector<DistanceData> distances;
        for (size_t i = 0, e = boxes.size(); i < e; ++i)
        {
            JunctionBox& firstBox = boxes[i];
            for (size_t j = i + 1; j < e; ++j)
            {
                JunctionBox& secondBox = boxes[j];
                distances.emplace_back(firstBox.squaredDistance(secondBox), &firstBox, &secondBox);
            }
        }

        // sort before returning
        std::sort(distances.begin(), distances.end(),
            [](const auto& a, const auto& b) { return a.m_distance < b.m_distance; });

        return distances;
    }

    std::vector<DistanceData> getShortestSortedDistances(
        uint32_t numPairsToConnect, std::vector<JunctionBox>& boxes) const
    {
        std::vector<DistanceData> distances;
        for (size_t i = 0, e = boxes.size(); i < e; ++i)
        {
            JunctionBox& firstBox = boxes[i];
            for (size_t j = i + 1; j < e; ++j)
            {
                JunctionBox& secondBox = boxes[j];

                const double distance = firstBox.squaredDistance(secondBox);

                bool add = true;
                add = distances.size() < numPairsToConnect;
                if (!add)
                {
                    auto it = std::lower_bound(distances.begin(), distances.end(), distance,
                        [](const DistanceData& data, double value) { return data.m_distance < value; });
                    if (it != distances.end())
                    {
                        const auto index = std::distance(distances.begin(), it);
                        if (index < numPairsToConnect)
                        {
                            auto itToRemove =
                                distances.begin() + std::max<int>(static_cast<int>(numPairsToConnect) - 1, index);
                            distances.erase(itToRemove);
                            add = true;
                        }
                    }
                    else
                    {
                        // value is above max of the ones with consider, skipping
                    }
                }

                if (add)
                {
                    distances.emplace_back(distance, &firstBox, &secondBox);
                    if (distances.size() > 1)
                    {
                        std::sort(distances.begin(), distances.end(),
                            [](const auto& a, const auto& b) { return a.m_distance < b.m_distance; });
                    }
                }
            }
        }
        return distances;
    }

private:
    std::vector<JunctionBox> m_boxes;
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

    JunctionBoxes boxes;
    do
    {
        int currentChar = file.get();
        if (currentChar == EOF)
            break;

        if (currentChar == '\r')
            continue;

        if (currentChar == '\n')
            continue;

        auto readNumber = [&] {
            constexpr auto max = std::numeric_limits<uint32_t>::max();
            uint32_t number = 0;
            while (std::isdigit(currentChar))
            {
                int asInt = currentChar - '0';
                if (number > (max / 10))
                {
                    LogError("Number %u is too big to fit in the type", number);
                    exit(-1);
                }
                number = number * 10 + asInt;

                if (!std::isdigit(file.peek()))
                    break;

                currentChar = file.get();
            }
            return number;
        };
        auto readSeparator = [&] {
            currentChar = file.get();
            if (currentChar != ',')
            {
                LogError("Unexpected format: number was followed by %c instead of a comma (,)", currentChar);
                exit(-1);
            }
            currentChar = file.get();
        };

        auto x = readNumber();
        readSeparator();
        auto y = readNumber();
        readSeparator();
        auto z = readNumber();
        boxes.addBox(x, y, z);
    }
    while (file);

    const auto result = boxes.connect(useSmallInput ? 10 : 1000);

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Results ==> " << result.first << ", " << result.second << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
