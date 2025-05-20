#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include "wlp4data.h"
#include <unordered_map>
#include <algorithm>

class Node
{
public:
    std::string val;
    bool hasParent = false;
    std::vector<std::shared_ptr<Node>> children;
    Node() = default;
    Node(std::string val) : val(val) {}
};

void print(const std::deque<std::string> input)
{
    for (std::string word : input)
    {
        if (word == input.back())
        {
            std::cout << word << std::endl;
            continue;
        }
        std::cout << word << " ";
    }
}
void shift(int &pos, std::deque<std::string> &input)
{
    if (pos >= 0 && pos < input.size() - 1)
    {
        std::swap(input[pos], input[pos + 1]);
        pos++;
    }
    else
    {
        std::cout << "shift error, position: " << pos << " and size: " << input.size() << std::endl;
    }
    // print(input);
}
void reduce(int &pos, std::deque<std::string> &input, const std::pair<std::string, std::deque<std::string>> red)
{
    if (red.second[0] == ".EMPTY")
    {
        input.insert(input.begin() + pos, red.first);
        pos++;
    }
    else
    {
        std::deque<std::string> rhs;
        int i = pos - 1;
        for (; i >= 0; i--)
        {
            rhs.emplace_front(input[i]);
            if (rhs.size() == red.second.size())
                break;
        }
        input.erase(input.begin() + i, input.begin() + pos);
        input.insert(input.begin() + i, red.first);
        pos -= rhs.size() - 1;
    }
    // print(input);
}
void reverseDuplicateIndices(std::deque<std::shared_ptr<Node>> &dq)
{
    std::unordered_map<std::string, std::vector<int>> indexMap;
    for (int i = 0; i < dq.size(); i++)
    {
        indexMap[dq[i]->val].push_back(i);
    }
    for (auto &[element, indices] : indexMap)
    {
        if (indices.size() > 1)
        {
            std::vector<std::shared_ptr<Node>> tempElements;
            std::vector<int> reversedIndices = indices;
            std::reverse(reversedIndices.begin(), reversedIndices.end());
            for (int idx : indices)
            {
                tempElements.push_back(dq[idx]);
            }
            for (size_t i = 0; i < indices.size(); i++)
            {
                dq[reversedIndices[i]] = tempElements[i];
            }
        }
    }
}
std::vector<std::shared_ptr<Node>> slr1parsing(const std::deque<std::string> input, const std::vector<std::pair<std::string, std::deque<std::string>>> cfgs,
                                               const std::map<int, std::map<std::string, int>> transitions, const std::map<int, std::map<std::string, int>> reductions)
{
    int pos = 0;   // mark position
    int currState; // current state
    int success = 0;
    bool error = false;
    std::deque<std::string> output = input;
    output.emplace_front(".");
    std::vector<std::shared_ptr<Node>> symbolStack;
    std::deque<std::shared_ptr<Node>> subTreeStack;
    std::vector<int> stateStack;
    stateStack.emplace_back(0);
    // print(output);
    for (std::string symbol : input)
    {
        if (symbol != "BOF")
            success++;
        // Reduce until we are no longer in a reduce state
        while (reductions.find(stateStack.back()) != reductions.end() &&
               reductions.at(stateStack.back()).find(symbol) != reductions.at(stateStack.back()).end())
        {
            currState = stateStack.back();
            int rule = reductions.at(currState).at(symbol);
            std::string lhs = cfgs[rule].first;
            std::deque<std::string> rhs = cfgs[rule].second;
            int len = rhs.size();
            if (rhs[0] == ".EMPTY")
            {
                len = 0;
                std::shared_ptr<Node> empty = std::make_shared<Node>(".EMPTY");
                empty->hasParent = true;
                subTreeStack.emplace_back(empty);
            }
            for (int i = 0; i < len; i++)
            {
                stateStack.pop_back();
                for (int j = symbolStack.size() - 1; j >= 0; j--)
                {
                    if (!symbolStack.at(j)->hasParent && symbolStack.at(j)->val == rhs[i])
                    {
                        symbolStack.at(j)->hasParent = true;
                        subTreeStack.emplace_back(symbolStack.at(j));
                        symbolStack.erase(symbolStack.begin() + j);
                        break;
                    }
                }
            }
            std::shared_ptr<Node> root = std::make_shared<Node>(lhs);
            reverseDuplicateIndices(subTreeStack);
            for (auto n : subTreeStack)
            {
                root->children.emplace_back(n);
            }

            subTreeStack.clear();
            symbolStack.emplace_back(root);
            int newState = transitions.at(stateStack.back()).at(lhs);
            stateStack.emplace_back(newState);
            reduce(pos, output, cfgs[rule]);
        }
        // Once we can no longer reduce, shift a symbol from input
        if (transitions.find(stateStack.back()) != transitions.end() &&
            transitions.at(stateStack.back()).find(symbol) != transitions.at(stateStack.back()).end())
        {
            symbolStack.emplace_back(new Node(symbol));
            stateStack.emplace_back(transitions.at(stateStack.back()).at(symbol));
            shift(pos, output);
        }
        else
        {
            error = true;
            std::cerr << "ERROR at " << success << "\n";
            break;
        }
    }
    if (!error && reductions.at(stateStack.back()).find(".ACCEPT") != reductions.at(stateStack.back()).end())
    {
        std::shared_ptr<Node> root = std::make_shared<Node>(cfgs[0].first);
        for (std::string s : cfgs[0].second)
        {
            for (auto n : symbolStack)
            {
                if (!n->hasParent && n->val == s)
                    root->children.emplace_back(n);
            }
        }
        reduce(pos, output, cfgs[0]);
        symbolStack.emplace_back(root);
        return symbolStack;
    }
    return {};
}
void printNodes(std::shared_ptr<Node> currNode, std::map<std::string, std::deque<std::string>> &tokens,
                const std::vector<std::pair<std::string, std::deque<std::string>>> cfgs)
{
    if (currNode->val == ".EMPTY")
        return;
    // leaf node
    if (currNode->children.size() == 0)
    {
        std::cout << currNode->val << " " << tokens.at(currNode->val).at(0) << std::endl;
        tokens.at(currNode->val).pop_front();
        return;
    }
    // internal node
    std::deque<std::string> children;
    for (auto child : currNode->children)
    {
        children.emplace_back(child->val);
    }
    for (std::pair<std::string, std::deque<std::string>> cfg : cfgs)
    {
        if (cfg.first == currNode->val && cfg.second == children)
        {
            std::cout << cfg.first;
            for (std::string toSymbols : cfg.second)
            {
                std::cout << " " << toSymbols;
            }
            std::cout << std::endl;
            break;
        }
    }
    for (auto child : currNode->children)
    {
        printNodes(child, tokens, cfgs);
    }
}

int main()
{
    std::vector<std::pair<std::string, std::deque<std::string>>> cfgs; // cfgs: <startSymbol, toSymbols>
    std::map<int, std::map<std::string, int>> transitions;             // transition: <currState, <symbol, toState>>
    std::map<int, std::map<std::string, int>> reductions;              // reductions: <currState, <next symbols, rule>>
    std::map<std::string, std::deque<std::string>> tokens;
    std::string line, word;
    std::deque<std::string> input;

    // get CFGs
    std::istringstream stream(WLP4_CFG);
    while (std::getline(stream, line))
    {
        if (line == ".CFG")
            continue;
        std::istringstream stream(line);
        std::string lhs;
        std::deque<std::string> rhs;
        stream >> lhs; // get the nonterminal
        while (stream >> word)
        {
            rhs.push_back(word);
        } // get the right hand side
        cfgs.emplace_back(std::pair<std::string, std::deque<std::string>>(lhs, rhs));
    }
    // get transitions
    stream.clear();
    stream.str(WLP4_TRANSITIONS);
    while (std::getline(stream, line))
    {
        int fromState, toState;
        std::string symbol;
        stream >> fromState;
        stream >> symbol;
        stream >> toState;
        transitions[fromState][symbol] = toState;
    }
    // get reductioins
    stream.clear();
    stream.str(WLP4_REDUCTIONS);
    while (std::getline(stream, line))
    {
        int state, rule;
        std::string nextSymbol;
        stream >> state;
        stream >> rule;
        stream >> nextSymbol;
        reductions[state][nextSymbol] = rule;
    }
    // get input
    while (std::getline(std::cin, line))
    {
        if (line.empty())
            continue;
        std::string kind;
        std::string lexeme;
        std::istringstream stream(line);

        stream >> kind;
        input.push_back(kind);
        stream >> lexeme;
        tokens[kind].emplace_back(lexeme);
    }
    input.emplace_front("BOF");
    input.emplace_back("EOF");
    tokens["BOF"].emplace_back("BOF");
    tokens["EOF"].emplace_back("EOF");
    std::vector<std::shared_ptr<Node>> treeStack = slr1parsing(input, cfgs, transitions, reductions);

    if (!treeStack.empty() && treeStack.back() != nullptr)
    {
        printNodes(treeStack.back(), tokens, cfgs);
    }
}