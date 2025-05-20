#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include <algorithm>

const std::string NUM_TYPE = "int";
const std::string NULL_TYPE = "int*";
const std::vector<std::string> OPERATIONS = {"PLUS", "MINUS", "STAR", "SLASH", "PCT", "AMP", "NEW", "GETCHAR"};

// A node stores a production or a token
class Node
{
public:
    std::string lhs;
    std::vector<std::string> rhs;
    // token: NUM or NULL, ID (has variable name) have types
    // lhs: expr, term, factor or lvalue have type
    std::string type = "";
    int childrenSize = 0;
    std::vector<std::shared_ptr<Node>> children;
    std::weak_ptr<Node> parent;
    bool isVisited = false;
    Node() = default;
    Node(std::string lhs, std::vector<std::string> rhs) : lhs(lhs), rhs(rhs) {}
};

class Procedure
{
public:
    std::string name;
    std::vector<std::string> signature;             // parameter list
    std::map<std::string, std::string> symbolTable; // local symbol table
    Procedure() = default;
    Procedure(std::string name) : name(name) {}
};

void printError(std::string s)
{
    std::cerr << "ERROR : " + s + "\n";
}

void printTree(std::shared_ptr<Node> node)
{
    if (node == nullptr)
        return;
    std::cout << node->lhs;
    for (auto s : node->rhs)
    {
        std::cout << " " << s;
    }
    if (node->type != "")
    {
        std::cout << " : " << node->type;
    }
    std::cout << std::endl;
    // leaf node
    if (node->childrenSize == 0)
    {
        return;
    }
    // internal node
    else
    {
        for (auto child : node->children)
        {
            printTree(child);
        }
    }
}

bool isInSymbolTable(const std::shared_ptr<Procedure> currProcedure, const std::string variable)
{
    if (currProcedure->symbolTable.find(variable) == currProcedure->symbolTable.end())
    {
        return false;
    }
    return true;
}

std::string getRhsStr(const std::shared_ptr<Node> node)
{
    std::string rhs_str;
    for (auto s : node->rhs)
    {
        if (rhs_str.size() == 0)
        {
            rhs_str = s;
            continue;
        }
        rhs_str = rhs_str + " " + s;
    }
    return rhs_str;
}

void parseID(const std::shared_ptr<Node> node, const std::vector<std::shared_ptr<Procedure>> symbolTable, bool &isPass)
{
    node->isVisited = true;
    std::shared_ptr<Procedure> currProcedure = symbolTable.back();
    // multiple declaration using the same variable name
    if (currProcedure->symbolTable.find(node->rhs[0]) != currProcedure->symbolTable.end())
    {
        std::cout << "1" << std::endl;
        printError("parseID: multiple declaration using the same variable name");
        isPass = false;
        return;
    }
    // find variable's type
    // trace back to parent: dcl → type ID
    std::shared_ptr<Node> parent = node->parent.lock();
    int i = 0;
    for (; i < parent->children.size(); i++)
    {
        if (parent->children[i] == node)
            break;
    }
    if (parent->children[i - 1]->lhs == "type")
    {
        // type → INT
        if (parent->children[i - 1]->rhs.size() == 1)
        {
            node->type = "int";
        }
        // type → INT STAR
        else if (parent->children[i - 1]->rhs.size() == 2)
        {
            node->type = "int*";
        }
    }
    currProcedure->symbolTable[node->rhs[0]] = node->type;
}

std::pair<std::string, std::string> parseDcl(const std::shared_ptr<Node> node, const std::vector<std::shared_ptr<Procedure>> symbolTable, bool &isPass)
{
    node->isVisited = true;
    // node: dcl → type ID
    // type → INT
    std::string type = "int";
    // type → INT STAR
    if (node->children[0]->rhs.size() > 1)
        type = "int*";
    // ID lexeme
    parseID(node->children[1], symbolTable, isPass);
    std::string id = node->children[1]->rhs[0];
    return std::pair<std::string, std::string>(type, id);
}

bool isOperation(std::string s)
{
    for (auto i : OPERATIONS)
    {
        if (s.find(i) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}
std::string annotateTypes(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> symbolTable, bool &isPass, bool &nullable);
void parseArglist(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass, std::vector<std::string> &returnArr)
{
    bool nullable;
    std::string type = annotateTypes(node->children[0], symbolTable, isPass, nullable);
    returnArr.emplace_back(type);
    // arglist → expr
    if (node->rhs.size() == 1)
    {
        return;
    }
    // arglist → expr COMMA arglist
    else
    {
        parseArglist(node->children[2], symbolTable, isPass, returnArr);
    }
}
void checkProcedureCalls(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    std::shared_ptr<Procedure> currProcedure = symbolTable.back();
    std::string rhs_str = getRhsStr(node);
    std::string id = node->children[0]->rhs[0];
    // id refers to a local variable
    if (currProcedure->symbolTable.find(id) != currProcedure->symbolTable.end())
    {
        printError("procedure call error");
        isPass = false;
        return;
    }
    std::shared_ptr<Procedure> toCall;
    // check if procedure has been declared
    bool found = false;
    for (auto p : symbolTable)
    {
        if (p->name == id)
        {
            toCall = p;
            found = true;
            break;
        }
    }
    if (!found)
    {
        printError("procedure has not been declared");
        isPass = false;
        return;
    }
    // check if signature matches
    // factor → ID LPAREN RPAREN
    if (rhs_str == "ID LPAREN RPAREN")
    {
        if (toCall->signature.size() != 0)
        {
            printError("signature size mismatched: factor → ID LPAREN RPAREN");
            isPass = false;
            return;
        }
    }
    // factor → ID LPAREN arglist RPAREN
    else if (rhs_str == "ID LPAREN arglist RPAREN")
    {
        std::vector<std::string> returnArr;
        parseArglist(node->children[2], symbolTable, isPass, returnArr);
        if (returnArr.size() == toCall->signature.size())
        {
            for (int i = 0; i < returnArr.size(); i++)
            {
                if (returnArr[i] != toCall->signature[i])
                {
                    printError("signature type mismatched: factor → ID LPAREN arglist RPAREN");
                    isPass = false;
                    return;
                }
            }
        }
        else
        {
            printError("signature size mismatched: factor → ID LPAREN arglist RPAREN");
            isPass = false;
            return;
        }
    }
}
std::string checkOperations(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass);
std::string annotateTypes(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> symbolTable, bool &isPass, bool &nullable)
{
    node->isVisited = true;
    int index = 0;
    std::shared_ptr<Procedure> currProcedure = symbolTable.back();
    // base case: node is a leaf and lhs = NUM/NULL, or if lhs = ID and rhs != procedure name
    if (node->childrenSize == 0)
    {
        // NUM int
        // has parent: factor → NUM
        if (node->lhs == "NUM")
        {
            node->type = NUM_TYPE;
            return NUM_TYPE;
        }
        // NULL NULL
        // has parent: factor → NULL
        else if (node->lhs == "NULL")
        {
            nullable = true;
            node->type = NULL_TYPE;
            return NULL_TYPE;
        }
        else if (node->lhs == "ID")
        {
            std::string type;
            auto it = currProcedure->symbolTable.find(node->rhs[0]);
            if (it != currProcedure->symbolTable.end())
            {
                type = it->second;
                node->type = type;
            }
            // used before initialized
            else
            {
                bool found = false;
                for (auto p : symbolTable)
                {
                    if (p->name == node->rhs[0])
                    {
                        type = "int";
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    std::cout << "99" << std::endl;
                    printError("annotate types: variable used before being initialized");
                    isPass = false;
                    return "";
                }
            }
            return type;
        }
    }

    // get production string
    std::string production = node->lhs;
    for (auto s : node->rhs)
    {
        production = production + " " + s;
    }
    bool isOp = isOperation(production);
    if (isOp || production.find("lvalue") != std::string::npos)
        node->type = checkOperations(node, symbolTable, isPass);

    // expr → term
    // term → factor
    // factor → NUM
    // factor → NULL
    // factor → ID
    // lvalue → ID
    // factor → ID LPAREN RPAREN
    // factor → ID LPAREN arglist RPAREN
    if (production == "expr term" || production == "term factor" || production == "factor NUM" ||
        production == "factor NULL" || production == "factor ID" || production == "lvalue ID" ||
        production == "factor ID LPAREN RPAREN" || production == "factor ID LPAREN arglist RPAREN")
    {
        index = 0;
        node->type = annotateTypes(node->children[0], symbolTable, isPass, nullable);
        if (production == "factor ID LPAREN RPAREN" || production == "factor ID LPAREN arglist RPAREN")
        {
            checkProcedureCalls(node, symbolTable, isPass);
        }
    }
    // factor → LPAREN expr RPAREN
    if (production == "factor LPAREN expr RPAREN")
    {
        index = 1;
        node->type = annotateTypes(node->children[1], symbolTable, isPass, nullable);
    }
    return node->type;
}

std::string twoVariableOperation(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable,
                                 bool &isPass, std::map<std::pair<std::string, std::string>, std::string> typeComb)
{
    std::string type1, type2;
    bool nullable = false;
    type1 = annotateTypes(node->children[0], symbolTable, isPass, nullable);
    type2 = annotateTypes(node->children[2], symbolTable, isPass, nullable);
    std::pair<std::string, std::string> comb = std::pair<std::string, std::string>(type1, type2);
    // type mismatch
    if (typeComb.find(comb) == typeComb.end())
    {
        printError("two varaible operation: type mismatched");
        isPass = false;
        return "";
    }
    node->type = typeComb[comb];
    return node->type;
}

std::string checkOperations(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    bool nullable = false;
    node->isVisited = true;
    std::string rhs_str = getRhsStr(node);
    std::string type;
    // factor → AMP lvalue
    if (rhs_str.find("AMP") != std::string::npos)
    {
        type = annotateTypes(node->children[1], symbolTable, isPass, nullable);
        if (type != "int")
        {
            printError("operation type mismatched: factor → AMP lvalue");
            isPass = false;
            return "";
        }
        node->type = "int*";
    }
    // factor → STAR factor
    if (node->lhs == "factor" && rhs_str.find("STAR") != std::string::npos)
    {
        type = annotateTypes(node->children[1], symbolTable, isPass, nullable);
        if (type != "int*")
        {
            printError("operation type mismatched: factor → STAR factor");
            isPass = false;
            return "";
        }
        node->type = "int";
    }
    // factor → NEW INT LBRACK expr RBRACK
    if (node->lhs == "factor" && rhs_str.find("NEW") != std::string::npos)
    {
        type = annotateTypes(node->children[3], symbolTable, isPass, nullable);
        if (type != "int")
        {
            printError("operation type mismatched: factor → NEW INT LBRACK expr RBRACK");
            isPass = false;
            return "";
        }
        node->type = "int*";
    }
    // factor → GETCHAR LPAREN RPAREN
    if (node->lhs == "factor" && rhs_str.find("GETCHAR") != std::string::npos)
    {
        node->type = "int";
    }

    std::map<std::pair<std::string, std::string>, std::string> typeComb;
    typeComb[std::pair<std::string, std::string>("int", "int")] = "int";
    if (node->lhs == "term")
    {
        // term → term STAR factor
        if (rhs_str.find("STAR") != std::string::npos)
            node->type = twoVariableOperation(node, symbolTable, isPass, typeComb);

        // term → term SLASH factor
        if (rhs_str.find("SLASH") != std::string::npos)
            node->type = twoVariableOperation(node, symbolTable, isPass, typeComb);

        // term → term PCT factor
        if (rhs_str.find("PCT") != std::string::npos)
            node->type = twoVariableOperation(node, symbolTable, isPass, typeComb);
    }
    if (node->lhs == "expr")
    {
        typeComb[std::pair<std::string, std::string>("int*", "int")] = "int*";
        typeComb[std::pair<std::string, std::string>("int", "int*")] = "int*";
        // expr → expr PLUS term
        if (rhs_str.find("PLUS") != std::string::npos)
            node->type = twoVariableOperation(node, symbolTable, isPass, typeComb);

        typeComb.erase(std::pair<std::string, std::string>("int", "int*"));
        typeComb[std::pair<std::string, std::string>("int*", "int*")] = "int";
        // expr → expr MINUS term
        if (rhs_str.find("MINUS") != std::string::npos)
            node->type = twoVariableOperation(node, symbolTable, isPass, typeComb);
    }

    if (node->lhs == "lvalue")
    {
        // lvalue → LPAREN lvalue RPAREN - does not change type
        if (rhs_str.find("LPAREN") != std::string::npos)
            node->type = checkOperations(node->children[1], symbolTable, isPass);
        // lvalue → ID
        if (rhs_str == "ID")
            node->type = annotateTypes(node->children[0], symbolTable, isPass, nullable);
        // lvalue → STAR factor
        if (rhs_str.find("STAR") != std::string::npos)
        {
            // in this case, factor must be non-null int*
            std::string ftype = annotateTypes(node->children[1], symbolTable, isPass, nullable);
            if (ftype == "int" || nullable)
            {
                printError("operation type mismatched: lvalue → STAR factor");
                isPass = false;
                return "";
            }
            node->type = "int";
        }
    }
    if (!isPass)
        return "";
    return node->type;
}

void collectParamlist(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    std::pair<std::string, std::string> type_id;
    std::shared_ptr<Procedure> currProcedure = symbolTable.back();
    // paramlist → dcl
    if (node->rhs.size() == 1)
    {
        type_id = parseDcl(node->children[0], symbolTable, isPass);
        // push to signature and symbol table
        currProcedure->signature.emplace_back(type_id.first);
        currProcedure->symbolTable[type_id.second] = type_id.first;
        return;
    }
    // paramlist → dcl COMMA paramlist
    else
    {
        type_id = parseDcl(node->children[0], symbolTable, isPass);
        // push to signature and symbol table
        currProcedure->signature.emplace_back(type_id.first);
        currProcedure->symbolTable[type_id.second] = type_id.first;
        collectParamlist(node->children[2], symbolTable, isPass);
    }
}

void typeCheck(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass,
               const std::string targetType, const int index)
{
    bool nullable;
    std::string type = annotateTypes(node->children[index], symbolTable, isPass, nullable);
    if (type != targetType)
    {
        isPass = false;
        return;
    }
}

void testTwoSidesType(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass,
                      const int index1, const int index2, const std::string sign)
{
    bool nullable;
    std::string type = annotateTypes(node->children[index1], symbolTable, isPass, nullable);
    // check if type is the same
    typeCheck(node, symbolTable, isPass, type, index2);
}

void checkTest(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    // test → expr EQ expr
    // test → expr NE expr
    // test → expr LT expr
    // test → expr LE expr
    // test → expr GE expr
    // test → expr GT expr
    testTwoSidesType(node, symbolTable, isPass, 0, 2, node->rhs[1]);
    if (!isPass)
    {
        printError("two sides of test rhs are not of the same type");
        return;
    }
}
void typeCheckStatement(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    std::string rhs_str = getRhsStr(node);
    // statements → statements statement
    if (rhs_str == "statements statement")
    {
        typeCheckStatement(node->children[0], symbolTable, isPass);
        node = node->children[1];
        rhs_str = getRhsStr(node);
    }

    if (!node->isVisited && node->lhs == "statement")
    {
        // statement → PRINTLN LPAREN expr RPAREN SEMI
        if (rhs_str.find("PRINTLN") != std::string::npos)
        {
            typeCheck(node, symbolTable, isPass, "int", 2);
            if (!isPass)
            {
                printError("statement → PRINTLN LPAREN expr RPAREN SEMI: expr is not int");
                return;
            }
        }
        // statement → PUTCHAR LPAREN expr RPAREN SEMI
        else if (rhs_str.find("PUTCHAR") != std::string::npos)
        {
            typeCheck(node, symbolTable, isPass, "int", 2);
            if (!isPass)
            {
                printError("statement → PUTCHAR LPAREN expr RPAREN SEMI: expr is not int");
                return;
            }
        }
        // statement → DELETE LBRACK RBRACK expr SEMI
        else if (rhs_str.find("DELETE") != std::string::npos)
        {
            typeCheck(node, symbolTable, isPass, "int*", 3);
            if (!isPass)
            {
                printError("statement → DELETE LBRACK RBRACK expr SEMI: expr is not int*");
                return;
            }
        }
        // statement → lvalue BECOMES expr SEMI
        else if (rhs_str.find("BECOMES") != std::string::npos)
        {
            testTwoSidesType(node, symbolTable, isPass, 0, 2, "EQ");
            if (!isPass)
            {
                printError("statement → lvalue BECOMES expr SEMI: left and right side are of the different type");
                return;
            }
        }
        // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
        else if (rhs_str == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE")
        {
            checkTest(node->children[2], symbolTable, isPass);
            typeCheckStatement(node->children[5], symbolTable, isPass);
            typeCheckStatement(node->children[9], symbolTable, isPass);
            if (!isPass)
            {
                printError("statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE: component is not well-defined");
                return;
            }
        }
        // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
        else if (rhs_str == "WHILE LPAREN test RPAREN LBRACE statements RBRACE")
        {
            checkTest(node->children[2], symbolTable, isPass);
            typeCheckStatement(node->children[5], symbolTable, isPass);
            if (!isPass)
            {
                printError("statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE: component is not well-defined");
                return;
            }
        }
    }
}

void semanticCheck(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Procedure>> &symbolTable, bool &isPass)
{
    std::shared_ptr<Procedure> currProcedure;
    std::string rhs_str = getRhsStr(node);
    if (symbolTable.size() != 0)
        currProcedure = symbolTable.back();

    // reach leaf node
    if (node->childrenSize == 0)
    {
        // return type must be int
        // parent: ... RETURN expr ...
        if (node->lhs == "RETURN")
        {
            int index = 0;
            std::string returnType;
            std::shared_ptr<Node> parent = node->parent.lock();
            auto it = std::find(parent->children.begin(), parent->children.end(), node);

            if (it != parent->children.end())
            {
                index = std::distance(parent->children.begin(), it);
            }
            bool nullable;
            std::shared_ptr<Node> returnExpr = parent->children[index + 1];
            returnType = annotateTypes(returnExpr, symbolTable, isPass, nullable);
            if (returnType != "int")
            {
                printError("return type error");
                return;
            }
        }
        return;
    }

    // get new procedures
    // procedure -> INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    if (node->lhs == "procedure")
    {
        std::string name = node->children[1]->rhs[0];
        // check duplicate procedure declaration
        for (auto p : symbolTable)
        {
            if (p->name == name)
            {
                std::cout << "2" << std::endl;
                printError("duplicate procedure declaration");
                isPass = false;
                return;
            }
        }
        symbolTable.emplace_back(std::make_shared<Procedure>(name));
        currProcedure = symbolTable.back();
        // collect procedure signature
        std::vector<std::string> params = node->children[3]->rhs;
        // params → (.EMPTY) then no modifications to signature
        // params → paramlist
        if (params[0] != ".EMPTY")
        {
            std::shared_ptr<Node> paramlist = node->children[3]->children[0];
            collectParamlist(paramlist, symbolTable, isPass);
        }
    }
    // main -> INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    else if (node->lhs == "main")
    {
        for (auto p : symbolTable)
        {
            if (p->name == "wain")
            {
                std::cout << "3" << std::endl;
                printError("duplicate wain declaration");
                isPass = false;
                return;
            }
        }
        symbolTable.emplace_back(std::make_shared<Procedure>("wain"));
        currProcedure = symbolTable.back();

        // collect procedure signature
        // dcl → type ID
        // push first dcl to signature
        std::pair<std::string, std::string> type_id = parseDcl(node->children[3], symbolTable, isPass);
        currProcedure->signature.emplace_back(type_id.first);
        currProcedure->symbolTable[type_id.second] = type_id.first;
        node->children[3]->isVisited = true;
        // second param must be int type
        type_id = parseDcl(node->children[5], symbolTable, isPass);
        if (type_id.first != "int")
        {
            std::cout << "14" << std::endl;
            printError("second param must be int type");
            return;
        }
        currProcedure->signature.emplace_back(type_id.first);
        currProcedure->symbolTable[type_id.second] = type_id.first;
        node->children[5]->isVisited = true;
    }

    // within procedures, collect parameter list
    // dcl → type ID
    if (!node->isVisited && node->lhs == "dcl")
    {
        std::pair<std::string, std::string> type_id = parseDcl(node, symbolTable, isPass);
        // if variable is not in the symbol table
        if (!isInSymbolTable(currProcedure, type_id.second))
        {
            currProcedure->symbolTable[type_id.second] = type_id.first;
        }
        else
        {
            std::cout << "4" << std::endl;
            printError("mutiple variable declaration");
            isPass = false;
            return;
        }
    }

    // type of a variable declared must match the type of its initialization value
    // dcls → (.EMPTY)
    // dcls → dcls dcl BECOMES NUM SEMI
    // dcls → dcls dcl BECOMES NULL SEMI
    if (!node->isVisited && node->lhs == "dcls")
    {
        if (node->rhs.size() > 1)
        {
            std::string type = node->rhs[3] == "NUM" ? NUM_TYPE : NULL_TYPE;
            node->children[3]->type = type;
            node->children[3]->isVisited = true;
            std::pair<std::string, std::string> type_id = parseDcl(node->children[1], symbolTable, isPass);
            if (type_id.first != type)
            {
                printError("variable type and initializtion value mismatched");
                isPass = false;
                return;
            }
        }
    }

    // statements → statements statement
    if (!node->isVisited && node->lhs == "statements")
    {
        typeCheckStatement(node, symbolTable, isPass);
        if (!isPass)
        {
            printError("statements type checking error");
            return;
        }
    }

    // check procedure calls
    if (!node->isVisited && node->lhs == "factor" && (rhs_str == "ID LPAREN RPAREN" || rhs_str == "ID LPAREN arglist RPAREN"))
        checkProcedureCalls(node, symbolTable, isPass);

    node->isVisited = true;
    for (auto n : node->children)
    {
        semanticCheck(n, symbolTable, isPass);
    }
}

int main()
{
    const std::shared_ptr<Node> NO_CHILD;
    std::string line, word;
    std::vector<std::shared_ptr<Procedure>> symbolTable;

    std::getline(std::cin, line);
    std::istringstream stream(line);
    std::string lhs;
    std::vector<std::string> rhs;
    stream >> lhs;
    while (stream >> word)
    {
        rhs.emplace_back(word);
    }
    std::shared_ptr<Node> root = std::make_shared<Node>(lhs, rhs);
    for (int i = 0; i < rhs.size(); i++)
    {
        root->children.emplace_back(NO_CHILD);
    }
    std::shared_ptr<Node> currNode = root;

    // build parse tree
    while (std::getline(std::cin, line))
    {
        std::istringstream stream(line);
        std::string lhs;
        std::vector<std::string> rhs;

        if (line.empty())
            continue;
        stream >> lhs;
        while (stream >> word)
        {
            rhs.emplace_back(word);
        }
        while (true)
        {
            std::vector<std::string> curRhs;
            curRhs = currNode->rhs;
            int index = -1;
            for (int i = 0; i < curRhs.size(); i++)
            {
                if (curRhs[i] == lhs && currNode->children[i] == NO_CHILD)
                {
                    index = i;
                    break;
                }
            }
            if (index != -1)
            {
                std::shared_ptr<Node> newNode = std::make_shared<Node>(lhs, rhs);
                newNode->parent = currNode;
                for (int i = 0; i < rhs.size(); i++)
                {
                    newNode->children.emplace_back(NO_CHILD);
                }
                currNode->children[index] = newNode;
                currNode->childrenSize++;
                currNode = newNode;
                break;
            }
            else
            {
                currNode = currNode->parent.lock();
            }
        }
    }
    // traverse the tree to annotate types and check semantic errors
    bool isPass = true;
    semanticCheck(root, symbolTable, isPass);
    if (isPass)
        printTree(root);
}