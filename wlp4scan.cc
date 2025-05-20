#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
const std::string ALPHABET = ".ALPHABET";
const std::string STATES = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT = ".INPUT";
const std::string EMPTY = ".EMPTY";

unsigned char wlp4_dfa[] = {
    0x2e, 0x41, 0x4c, 0x50, 0x48, 0x41, 0x42, 0x45, 0x54, 0x0a, 0x61, 0x2d,
    0x7a, 0x20, 0x41, 0x2d, 0x5a, 0x20, 0x30, 0x2d, 0x39, 0x0a, 0x28, 0x20,
    0x29, 0x20, 0x7b, 0x20, 0x7d, 0x20, 0x5b, 0x20, 0x5d, 0x0a, 0x3d, 0x20,
    0x21, 0x20, 0x3c, 0x20, 0x3e, 0x20, 0x2b, 0x20, 0x2d, 0x20, 0x2a, 0x20,
    0x2f, 0x20, 0x25, 0x20, 0x26, 0x0a, 0x2c, 0x20, 0x3b, 0x0a, 0x0a, 0x2e,
    0x53, 0x54, 0x41, 0x54, 0x45, 0x53, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x0a, 0x65, 0x78, 0x0a, 0x69, 0x64, 0x21, 0x0a, 0x30, 0x21, 0x0a, 0x6e,
    0x75, 0x6d, 0x21, 0x0a, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x21, 0x0a, 0x73,
    0x65, 0x6d, 0x69, 0x21, 0x0a, 0x6c, 0x70, 0x61, 0x72, 0x65, 0x6e, 0x21,
    0x0a, 0x72, 0x70, 0x61, 0x72, 0x65, 0x6e, 0x21, 0x0a, 0x6c, 0x62, 0x72,
    0x61, 0x63, 0x65, 0x21, 0x0a, 0x72, 0x62, 0x72, 0x61, 0x63, 0x65, 0x21,
    0x0a, 0x72, 0x65, 0x74, 0x75, 0x72, 0x6e, 0x21, 0x0a, 0x69, 0x66, 0x21,
    0x0a, 0x65, 0x6c, 0x73, 0x65, 0x21, 0x0a, 0x77, 0x68, 0x69, 0x6c, 0x65,
    0x21, 0x0a, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x6c, 0x6e, 0x21, 0x0a, 0x70,
    0x75, 0x74, 0x63, 0x68, 0x61, 0x72, 0x21, 0x0a, 0x67, 0x65, 0x74, 0x63,
    0x68, 0x61, 0x72, 0x21, 0x0a, 0x77, 0x61, 0x69, 0x6e, 0x21, 0x0a, 0x62,
    0x65, 0x63, 0x6f, 0x6d, 0x65, 0x73, 0x21, 0x0a, 0x69, 0x6e, 0x74, 0x21,
    0x0a, 0x65, 0x71, 0x21, 0x0a, 0x6e, 0x65, 0x21, 0x0a, 0x6c, 0x74, 0x21,
    0x0a, 0x67, 0x74, 0x21, 0x0a, 0x6c, 0x65, 0x21, 0x0a, 0x67, 0x65, 0x21,
    0x0a, 0x70, 0x6c, 0x75, 0x73, 0x21, 0x0a, 0x6d, 0x69, 0x6e, 0x75, 0x73,
    0x21, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x21, 0x0a, 0x73, 0x6c, 0x61, 0x73,
    0x68, 0x21, 0x0a, 0x70, 0x63, 0x74, 0x21, 0x0a, 0x6e, 0x65, 0x77, 0x21,
    0x0a, 0x64, 0x65, 0x6c, 0x65, 0x74, 0x65, 0x21, 0x0a, 0x6c, 0x62, 0x72,
    0x61, 0x63, 0x6b, 0x21, 0x0a, 0x72, 0x62, 0x72, 0x61, 0x63, 0x6b, 0x21,
    0x0a, 0x61, 0x6d, 0x70, 0x21, 0x0a, 0x6e, 0x75, 0x6c, 0x6c, 0x21, 0x0a,
    0x0a, 0x2e, 0x54, 0x52, 0x41, 0x4e, 0x53, 0x49, 0x54, 0x49, 0x4f, 0x4e,
    0x53, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x28, 0x20, 0x6c, 0x70,
    0x61, 0x72, 0x65, 0x6e, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x29,
    0x20, 0x72, 0x70, 0x61, 0x72, 0x65, 0x6e, 0x0a, 0x73, 0x74, 0x61, 0x72,
    0x74, 0x20, 0x7b, 0x20, 0x6c, 0x62, 0x72, 0x61, 0x63, 0x65, 0x0a, 0x73,
    0x74, 0x61, 0x72, 0x74, 0x20, 0x7d, 0x20, 0x72, 0x62, 0x72, 0x61, 0x63,
    0x65, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x3d, 0x20, 0x62, 0x65,
    0x63, 0x6f, 0x6d, 0x65, 0x73, 0x0a, 0x62, 0x65, 0x63, 0x6f, 0x6d, 0x65,
    0x73, 0x20, 0x3d, 0x20, 0x65, 0x71, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x20, 0x21, 0x20, 0x65, 0x78, 0x0a, 0x65, 0x78, 0x20, 0x3d, 0x20, 0x6e,
    0x65, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x3c, 0x20, 0x6c, 0x74,
    0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x3e, 0x20, 0x67, 0x74, 0x0a,
    0x6c, 0x74, 0x20, 0x3d, 0x20, 0x6c, 0x65, 0x0a, 0x67, 0x74, 0x20, 0x3d,
    0x20, 0x67, 0x65, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x2b, 0x20,
    0x70, 0x6c, 0x75, 0x73, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x2d,
    0x20, 0x6d, 0x69, 0x6e, 0x75, 0x73, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x20, 0x2a, 0x20, 0x73, 0x74, 0x61, 0x72, 0x0a, 0x73, 0x74, 0x61, 0x72,
    0x74, 0x20, 0x2f, 0x20, 0x73, 0x6c, 0x61, 0x73, 0x68, 0x0a, 0x73, 0x74,
    0x61, 0x72, 0x74, 0x20, 0x25, 0x20, 0x70, 0x63, 0x74, 0x0a, 0x73, 0x74,
    0x61, 0x72, 0x74, 0x20, 0x2c, 0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x0a,
    0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x3b, 0x20, 0x73, 0x65, 0x6d, 0x69,
    0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x5b, 0x20, 0x6c, 0x62, 0x72,
    0x61, 0x63, 0x6b, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x5d, 0x20,
    0x72, 0x62, 0x72, 0x61, 0x63, 0x6b, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x20, 0x26, 0x20, 0x61, 0x6d, 0x70, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x20, 0x30, 0x20, 0x30, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x31,
    0x2d, 0x39, 0x20, 0x6e, 0x75, 0x6d, 0x0a, 0x6e, 0x75, 0x6d, 0x20, 0x30,
    0x2d, 0x39, 0x20, 0x6e, 0x75, 0x6d, 0x0a, 0x73, 0x74, 0x61, 0x72, 0x74,
    0x20, 0x61, 0x2d, 0x7a, 0x20, 0x41, 0x2d, 0x5a, 0x20, 0x69, 0x64, 0x0a,
    0x69, 0x64, 0x20, 0x61, 0x2d, 0x7a, 0x20, 0x41, 0x2d, 0x5a, 0x20, 0x30,
    0x2d, 0x39, 0x20, 0x69, 0x64, 0x0a};
unsigned int wlp4_dfa_len = 666;

bool isChar(std::string s)
{
    return s.length() == 1;
}
bool isRange(std::string s)
{
    return s.length() == 3 && s[1] == '-';
}
bool isInStates(std::vector<std::string> states, std::string state)
{
    if (std::find(states.begin(), states.end(), state) != states.end())
    {
        return true;
    }
    return false;
}
// when flag == true, then only peek
// when flag == false, then consume and modify lexeme and currState
bool transition(std::vector<std::pair<std::pair<std::string, std::string>, std::vector<char>>> tranPairs, std::string &currState,
                char c, std::string &lexeme, bool flag)
{
    for (unsigned int i = 0; i < tranPairs.size(); i++)
    {
        if (tranPairs[i].first.first == currState)
        {
            std::vector<char> tran = tranPairs[i].second;

            if (std::find(tran.begin(), tran.end(), c) != tran.end())
            {
                // consume
                if (!flag)
                {
                    currState = tranPairs[i].first.second;
                    lexeme += c;
                }
                return true;
            }
        }
    }
    // if peek, then nextChar has no valid transitions
    // if consume, then currChar has no valid transitions, running into error state
    return false;
}
std::string typeIdentifier(const std::string currState, const std::string s, bool &outOfRange)
{
    const int max = 2147483647;
    if (currState == "comma")
        return "COMMA";
    if (currState == "lparen")
        return "LPAREN";
    if (currState == "rparen")
        return "RPAREN";
    if (currState == "num" || currState == "0")
    {
        long long num = std::stoll(s);
        if (num > max)
        {
            std::cerr << "ERROR: numeric value is out of range" << '\n';
            outOfRange = true;
        }
        return "NUM";
    }
    if (currState == "semi")
        return "SEMI";
    if (currState == "lbrace")
        return "LBRACE";
    if (currState == "rbrace")
        return "RBRACE";
    if (currState == "becomes")
        return "BECOMES";
    if (currState == "eq")
        return "EQ";
    if (currState == "ne")
        return "NE";
    if (currState == "lt")
        return "LT";
    if (currState == "ge")
        return "GE";
    if (currState == "gt")
        return "GT";
    if (currState == "le")
        return "LE";
    if (currState == "plus")
        return "PLUS";
    if (currState == "minus")
        return "MINUS";
    if (currState == "star")
        return "STAR";
    if (currState == "slash")
        return "SLASH";
    if (currState == "pct")
        return "PCT";
    if (currState == "lbrack")
        return "LBRACK";
    if (currState == "rbrack")
        return "RBRACK";
    if (currState == "amp")
        return "AMP";
    if (currState == "id" && s == "return")
        return "RETURN";
    if (currState == "id" && s == "if")
        return "IF";
    if (currState == "id" && s == "else")
        return "ELSE";
    if (currState == "id" && s == "while")
        return "WHILE";
    if (currState == "id" && s == "println")
        return "PRINTLN";
    if (currState == "id" && s == "putchar")
        return "PUTCHAR";
    if (currState == "id" && s == "getchar")
        return "GETCHAR";
    if (currState == "id" && s == "wain")
        return "WAIN";
    if (currState == "id" && s == "int")
        return "INT";
    if (currState == "id" && s == "new")
        return "NEW";
    if (currState == "id" && s == "delete")
        return "DELETE";
    if (currState == "id" && s == "NULL")
        return "NULL";
    if (currState == "id")
    {
        return "ID";
    }
    return "";
}

// Locations in the program that you should modify to store the
// DFA information have been marked with four-slash comments:
//// (Four-slash comment)
int main()
{
    std::string foo((char *)wlp4_dfa, wlp4_dfa_len);
    std::stringstream in(foo);
    std::string s;
    std::string initial_state;
    std::vector<std::string> accepting_states;
    std::vector<std::pair<std::pair<std::string, std::string>, std::vector<char>>> tranPairs;

    std::getline(in, s); // Alphabet section (skip header)
    // Read characters or ranges separated by whitespace
    while (in >> s)
    {
        if (s == STATES)
        {
            break;
        }
        else
        {
            if (isChar(s))
            {
                //// Variable 's[0]' is an alphabet symbol
            }
            else if (isRange(s))
            {
                for (char c = s[0]; c <= s[2]; ++c)
                {
                    //// Variable 'c' is an alphabet symbol
                }
            }
        }
    }

    std::getline(in, s); // States section (skip header)
    // Read states separated by whitespace
    while (in >> s)
    {
        if (s == TRANSITIONS)
        {
            break;
        }
        else
        {
            static bool initial = true;
            bool accepting = false;
            if (s.back() == '!' && !isChar(s))
            {
                accepting = true;
                s.pop_back();
            }
            //// Variable 's' contains the name of a state
            if (initial)
            {
                //// The state is initial
                initial = false;
                initial_state = s;
            }
            if (accepting)
            {
                //// The state is accepting
                accepting_states.push_back(s);
            }
        }
    }

    std::getline(in, s); // Transitions section (skip header)
    // Read transitions line-by-line
    while (std::getline(in, s))
    {
        std::string line = s;
        if (s == INPUT)
        {
            // Note: Since we're reading line by line, once we encounter the
            // input header, we will already be on the line after the header
            break;
        }
        else
        {
            std::string fromState, symbols, toState;
            std::istringstream line(s);
            std::vector<std::string> lineVec;

            while (line >> s)
            {
                lineVec.push_back(s);
            }
            fromState = lineVec.front();
            toState = lineVec.back();
            for (unsigned int i = 1; i < lineVec.size() - 1; ++i)
            {
                std::string s = lineVec[i];
                if (isChar(s))
                {
                    symbols += s;
                }
                else if (isRange(s))
                {
                    for (char c = s[0]; c <= s[2]; ++c)
                    {
                        symbols += c;
                    }
                }
            }
            std::vector<char> tran;
            for (char c : symbols)
            {
                //// There is a transition from 'fromState' to 'toState' on 'c'
                tran.push_back(c);
            }
            std::pair<std::pair<std::string, std::string>, std::vector<char>> tranPair(std::pair<std::string, std::string>(fromState, toState), tran);

            tranPairs.push_back(tranPair);
        }
    }

    bool outOfRange = false;
    std::istream &wlp4Code = std::cin;
    // Input section (already skipped header)
    while (std::getline(wlp4Code, s))
    {
        //// Variable 's' contains an input string for the DFA
        std::string currState = initial_state;
        std::string type = "";
        std::string lexeme = "";
        unsigned int ind = 0;
        bool goNext = false;  // flag for whitespace at the end of string
        bool comment = false; // when read a comment, turns true util read a newline character

        // if input number is out of range, then break
        if (outOfRange)
        {
            break;
        }

        // Ignore the empty line
        if (s.empty() || std::all_of(s.begin(), s.end(), isspace))
        {
            continue;
        }

        for (; ind < s.size();)
        {
            char c = s[ind];
            // ignore comment
            if ((c == '/' && s[ind + 1] == '/') || comment)
            {
                if (c == '/' && s[ind + 1] == '/')
                    comment = true;
                if (c == '\n')
                    comment = false;
                ind++;
                continue;
            }
            // ignore whitespace
            if (c == ' ')
            {
                // if current state is accepting state, then print out the token
                // otherwise just ignore the whitespace
                if (isInStates(accepting_states, currState))
                {
                    type = typeIdentifier(currState, lexeme, outOfRange);
                    std::cout << type << " " << lexeme << '\n';
                    // resume
                    lexeme = "";
                    currState = initial_state;
                }
                ind++;
                if (ind == s.size())
                {
                    goNext = true;
                    break;
                }
                continue;
            }
            // transition(state, peek)
            bool hasNextTran = transition(tranPairs, currState, c, lexeme, true);

            // no valid next transitions
            if (!hasNextTran)
            {
                // check if currState is accepted
                if (isInStates(accepting_states, currState))
                {
                    type = typeIdentifier(currState, lexeme, outOfRange);
                    std::cout << type << " " << lexeme << '\n';
                    // resume
                    lexeme = "";
                    currState = initial_state;
                    continue;
                }
                // currState is not accepted, reject
                else
                {
                    std::cerr << "ERROR: rejected when peek due to transition from " << currState << " to an error state on character " << c << " in string " << s << '\n';
                    break;
                }
            }
            // has valid next transition
            else
            {
                // transition(state, consume)
                transition(tranPairs, currState, c, lexeme, false);
            }
            ind++;
        }
        // consumed all the characters
        if (ind == s.length() && isInStates(accepting_states, currState))
        {
            type = typeIdentifier(currState, lexeme, outOfRange);
            std::cout << type << " " << lexeme << '\n';
        }
        // currState is not accepted
        else if (ind == s.length() && !goNext && !comment)
        {
            std::cerr << "ERROR: transition from " << currState << " to an error state on character " << s[s.length() - 1] << " in string " << s << '\n';
            break;
        }
        // stopped before consuming all the characters (rejected when peek)
        else if (!goNext && !comment)
        {
            break;
        }
    }
}
