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
    printf("%s(%d): \033[34mDEBUG\033[0m: ", inFile, inLine);
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
    printf("%s(%d): \033[33mINFO\033[0m: ", inFile, inLine);
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
    printf("%s(%d): \033[31mERROR\033[0m: ", inFile, inLine);
    va_list args;
    va_start(args, inFormat);
    vprintf(inFormat, args);
    va_end(args);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
enum class States
{
    Invalid = -1,

    LetterM,
    LetterU,
    LetterL,
    LetterD,
    LetterO,
    LetterN,
    Quote,
    LetterT,
    Digit,
    Comma,
    OpenParenthesis,
    CloseParenthesis,
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static States CharToState(char inChar)
{
    switch (inChar)
    {
        case 'm': return States::LetterM;
        case 'u': return States::LetterU;
        case 'l': return States::LetterL;
        case 'd': return States::LetterD;
        case 'o': return States::LetterO;
        case 'n': return States::LetterN;
        case 't': return States::LetterT;
        case ',': return States::Comma;
        case '(': return States::OpenParenthesis;
        case ')': return States::CloseParenthesis;
        case '\'': return States::Quote;
    }

    if (std::isdigit(inChar))
        return States::Digit;

    return States::Invalid;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class StateMachine
{
    struct Node
    {
        States m_state = States::Invalid;
        int m_maxOccurrences = 1;
        std::vector<Node*> m_nextNodes;

        Node() = default;

        Node(States inState, int inMaxOccurrences)
            : m_state(inState)
            , m_maxOccurrences(inMaxOccurrences)
        {
        }

        ~Node()
        {
            for (Node* node : m_nextNodes)
            {
                delete node;
            }
        }

        Node* AddNextNode(States inState, int inMaxOccurrences)
        {
            Node* nextNode = new Node{inState, inMaxOccurrences};
            m_nextNodes.push_back(nextNode);
            return nextNode;
        }
    };

public:
    StateMachine()
        : m_rootNode(new Node())
    {
        m_rootNode
            ->AddNextNode(States::LetterM, 1)
            ->AddNextNode(States::LetterU, 1)
            ->AddNextNode(States::LetterL, 1)
            ->AddNextNode(States::OpenParenthesis, 1)
            ->AddNextNode(States::Digit, 3)
            ->AddNextNode(States::Comma, 1)
            ->AddNextNode(States::Digit, 3)
            ->AddNextNode(States::CloseParenthesis, 1);

        Node* letterONode = m_rootNode
            ->AddNextNode(States::LetterD, 1)
            ->AddNextNode(States::LetterO, 1);

        // do()
        letterONode
            ->AddNextNode(States::OpenParenthesis, 1)
            ->AddNextNode(States::CloseParenthesis, 1);

        // don't()
        letterONode
            ->AddNextNode(States::LetterN, 1)
            ->AddNextNode(States::Quote, 1)
            ->AddNextNode(States::LetterT, 1)
            ->AddNextNode(States::OpenParenthesis, 1)
            ->AddNextNode(States::CloseParenthesis, 1);

        m_currentNode = m_rootNode;
    }

    ~StateMachine()
    {
        delete m_rootNode;
    }

    bool Accept(States inState, bool& outEndState)
    {
        if (inState != States::Invalid)
        {
            if (m_currentNode != m_rootNode && inState == m_currentNode->m_state && ++m_nbTimesCurrentNode <= m_currentNode->m_maxOccurrences)
            {
                return true;
            }
            else
            {
                for (Node* nextNode : m_currentNode->m_nextNodes)
                {
                    if (nextNode->m_state == inState)
                    {
                        if (nextNode->m_nextNodes.empty())
                        {
                            outEndState = true;

                            m_currentNode = m_rootNode;
                            m_nbTimesCurrentNode = 0;
                        }
                        else
                        {
                            m_currentNode = nextNode;
                            m_nbTimesCurrentNode = 1;
                        }

                        return true;
                    }
                }
            }
        }
        m_currentNode = m_rootNode;
        m_nbTimesCurrentNode = 0;

        return false;
    }

    int m_nbTimesCurrentNode = 0;
    Node* m_currentNode = nullptr;
    Node* const m_rootNode = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main()
{
    const bool useSmallInput = false;
    const bool useSmallInputPart2 = false;
    constexpr const char* fileName = useSmallInput ? useSmallInputPart2 ? "smallInputPart2" : "smallInput" : "input";
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogError("Couldn't find file '%s'", fileName);
        return 1;
    }

    StateMachine s;

    long long int result = 0;

    bool foundComma = false;
    int firstDigit = 0;
    int secondDigit = 0;
    int nbAccept = 0;

    bool mulEnabled = true;
    do
    {
        const int currentChar = file.get();
        if (currentChar == EOF)
            break;

        const States currentState = CharToState(currentChar);

        bool endState = false;
        if (s.Accept(currentState, endState))
        {
            ++nbAccept;
            if (currentState == States::Digit)
            {
                int asInt = currentChar - '0';
                int& digit = foundComma ? secondDigit : firstDigit;
                digit = digit * 10 + asInt;
                LogDebug("Found %d and changed to %d", asInt, digit);
            }
            else if (currentState == States::Comma)
            {
                foundComma = true;
            }

            if (endState)
            {
                if (nbAccept == 7) // don't()
                {
                    LogInfo("Accept don't()");
                    mulEnabled = false;
                }
                else if (nbAccept == 4) // do()
                {
                    LogInfo("Accept do()");
                    mulEnabled = true;
                }
                else if (foundComma && mulEnabled)
                {
                    LogInfo("Accept mul(%d,%d) => %d", firstDigit, secondDigit, firstDigit * secondDigit);
                    result += firstDigit * secondDigit;
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            LogError("Refuse invalid char %c", currentChar);
        }

        foundComma = false;
        firstDigit = 0;
        secondDigit = 0;
        nbAccept = 0;
    }
    while (file);

    std::cout << std::setfill('#') << std::setw(100) << "\n";
    std::cout << "Result == " << result << std::endl;

    return EXIT_SUCCESS;
}

#undef LogError
#undef LogInfo
#undef LogDebug

#undef ENABLE_LOG
