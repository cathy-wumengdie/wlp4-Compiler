#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <limits>
#include <iomanip>
#include <deque>

using namespace std;

class Node
{
public:
  string lhs;
  vector<string> rhs;
  string type;
  deque<string> arglistTypes;
  vector<shared_ptr<Node>> children; 
};

class symbolTableInfo
{
public:
  int offset;
  string type;
};

class Procedure
{
public:
  int paramSize = 0;
  deque<string> signature;                  // parameter list
  map<string, symbolTableInfo> symbolTable; // symbol name -> <type,offset from $29>
};

int ifLabel = 0;
int whileLabel = 0;
int paramSize = 0;
int deleteCount = 0;
map<string, shared_ptr<Procedure>> procedures;
deque<string> asmCode;

void printSymbolTable()
{
  cout << "; " << setw(15) << "Symbol" << setw(11) << "Type" << setw(14) << "Offset" << endl;
  for (const auto &[name, p] : procedures)
  {
    cout << "; " << name << ":" << endl;
    for (auto &[key, info] : p->symbolTable)
      cout << "; " << setw(15) << key << setw(11) << info.type << setw(12) << info.offset << endl;
  }
  cout << endl;
}

void parseTree(shared_ptr<Node> &curNode)
{
  string line;
  getline(cin, line);
  stringstream ss(line);

  ss >> curNode->lhs;
  string token;
  while (ss >> token)
  {
    if (token == ":")
    {
      ss >> curNode->type;
      break;
    }
    curNode->rhs.emplace_back(token);
  }

  if (curNode->rhs[0] == ".EMPTY" || isupper(curNode->lhs[0]))
    return;
  for (int i = 0; i < curNode->rhs.size(); ++i)
  {
    shared_ptr<Node> childNode = make_shared<Node>();
    curNode->children.emplace_back(childNode);
    parseTree(childNode);
  }
}

void push(int num)
{
  string reg = "$" + to_string(num);
  asmCode.emplace_back("sw " + reg + ", -4($30)         ; push(" + reg + ")\n" + "sub $30, $30, $4\n");
}

void pop(int num)
{
  string reg = "$" + to_string(num);
  asmCode.emplace_back("add $30, $30, $4        ; pop(" + reg + ")\n" + "lw " + reg + ", -4($30)\n");
}

void pop()
{
  asmCode.emplace_back("add $30, $30, $4");
}

void jalr(int num)
{
  string reg = "$" + to_string(num);
  asmCode.emplace_back("jalr " + reg + "\n");
}

void getChar()
{
  asmCode.emplace_back("lis $5\n.word 0xffff0004\nlw $3, 0($5)          ; get char\n");
}

void println()
{
  asmCode.emplace_back("add $1, $3, $0          ; print $1\n");
  push(31);
  jalr(10);
  pop(31);
  pop(1);
}

void genBegin()
{
  asmCode.emplace_back(".import print");
  asmCode.emplace_back(".import init");
  asmCode.emplace_back(".import new");
  asmCode.emplace_back(".import delete\n");

  asmCode.emplace_back("; begin prologue");
  asmCode.emplace_back("lis $4");
  asmCode.emplace_back(".word 4");
  asmCode.emplace_back("lis $10");
  asmCode.emplace_back(".word print");
  asmCode.emplace_back("lis $11");
  asmCode.emplace_back(".word 1\n");

  asmCode.emplace_back("beq $0, $0, wain    ; jump to wain\n");
}

void codeGen(shared_ptr<Node> &curNode, string &curProcedure, int &reg)
{

  if (curNode->lhs == "start")
    codeGen(curNode->children[1], curProcedure, reg);

  else if (curNode->lhs == "procedures")
  {
    // procedures → main
    if (curNode->rhs[0] == "main")
    {
      curProcedure = "wain";
      asmCode.emplace_back("; =================================");
      asmCode.emplace_back(curProcedure + ": ");
      shared_ptr<Procedure> newProcedure = make_shared<Procedure>();
      procedures["wain"] = newProcedure;
      codeGen(curNode->children[0], curProcedure, reg);
    }

    // procedures → procedure procedures
    else if (curNode->rhs[0] == "procedure")
    {
      codeGen(curNode->children[0], curProcedure, reg);
      codeGen(curNode->children[1], curProcedure, reg);
    }
  }

  else if (curNode->lhs == "procedure")
  {
    // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    curProcedure = curNode->children[1]->rhs[0];
    asmCode.emplace_back("; =================================");
    asmCode.emplace_back("F" + curProcedure + ": ");
    push(5);
    push(6);
    push(7);
    push(9);
    shared_ptr<Procedure> newProcedure = make_shared<Procedure>();
    procedures[curProcedure] = newProcedure;

    paramSize = 0;
    reg = 3;
    codeGen(curNode->children[3], curProcedure, reg);
    procedures[curProcedure]->paramSize = paramSize;
    for (int i = 0; i < paramSize; ++i)
    {
      string name = procedures[curProcedure]->signature[i];
      asmCode.pop_back();
      asmCode.pop_back();
      procedures[curProcedure]->symbolTable[name].offset = (paramSize - i) * 4 + 16;
    }
    asmCode.emplace_back("sub $29, $30, $4");

    vector<int> nonTerminals = {6, 7, 9};
    for (int i = 0; i < nonTerminals.size(); ++i)
    {
      if (asmCode.back() != "")
        asmCode.emplace_back("");
      codeGen(curNode->children[nonTerminals[i]], curProcedure, reg);
    }

    int symTableLen = procedures[curProcedure]->symbolTable.size();
    asmCode.emplace_back("");
    for (int i = 0; i < symTableLen - paramSize; ++i)
      pop();

    pop(9);
    pop(7);
    pop(6);
    pop(5);

    asmCode.emplace_back("\nlis $12");
    asmCode.emplace_back(".word 16");
    asmCode.emplace_back("add $29, $29, $12");
    asmCode.emplace_back("add $30, $29, $4");
    asmCode.emplace_back("jr $31");
  }

  else if (curNode->lhs == "main")
  {
    // main → INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    reg = 1;
    codeGen(curNode->children[3], curProcedure, reg);
    reg = 2;
    codeGen(curNode->children[5], curProcedure, reg);

    procedures["wain"]->paramSize = 2;
    for (int i = 0; i < 2; ++i)
    {
      string name = procedures["wain"]->signature[i];
      procedures["wain"]->symbolTable[name].offset = (2 - i) * 4;
    }
    asmCode.emplace_back("sub $29, $30, $4\n");

    string firstParam = procedures["wain"]->signature[0];
    if (procedures["wain"]->symbolTable[firstParam].type != "int*")
      asmCode.emplace_back("add $2, $0, $0");

    push(31);
    asmCode.emplace_back("lis $5");
    asmCode.emplace_back(".word init");
    jalr(5);
    pop(31);

    reg = 3;
    vector<int> nonTerminals = {8, 9, 11};
    for (int i = 0; i < nonTerminals.size(); ++i)
    {
      if (asmCode.back() != "")
        asmCode.emplace_back("");
      codeGen(curNode->children[nonTerminals[i]], curProcedure, reg);
    }
    asmCode.emplace_back("\n; begin epilogue");
    int symTableLen = procedures["wain"]->symbolTable.size();
    for (int i = 0; i < symTableLen; ++i)
      pop();
    asmCode.emplace_back("jr $31");
  }

  else if (curNode->lhs == "dcls")
  {
    if (curNode->rhs.size() == 5)
    {
      // dcls → dcls dcl BECOMES NUM SEMI
      if (curNode->rhs[3] == "NUM")
      {
        codeGen(curNode->children[0], curProcedure, reg);
        string num = curNode->children[3]->rhs[0];
        asmCode.emplace_back("");
        asmCode.emplace_back("lis $3");
        asmCode.emplace_back(".word " + num);
        reg = 3;
        codeGen(curNode->children[1], curProcedure, reg);
      }

      // dcls → dcls dcl BECOMES NULL SEMI
      else if (curNode->rhs[3] == "NULL")
      {
        codeGen(curNode->children[0], curProcedure, reg);
        asmCode.emplace_back("");
        asmCode.emplace_back("lis $3");
        asmCode.emplace_back(".word 1");
        reg = 3;
        codeGen(curNode->children[1], curProcedure, reg);
      }
    }
  }

  else if (curNode->lhs == "dcl")
  {
    // dcl → type ID
    curNode = curNode->children[1];
    string name = curNode->rhs[0];
    procedures[curProcedure]->signature.emplace_back(name);

    int symTableLen = procedures[curProcedure]->symbolTable.size();
    int paramLen = procedures[curProcedure]->paramSize;
    int offset = -4 * (symTableLen - paramLen);

    procedures[curProcedure]->symbolTable[name].offset = offset;
    procedures[curProcedure]->symbolTable[name].type = curNode->type;

    push(reg);
  }

  else if (curNode->lhs == "params")
  {
    // params → paramlist
    if (curNode->rhs[0] == "paramlist")
    {
      codeGen(curNode->children[0], curProcedure, reg);
    }
  }

  else if (curNode->lhs == "paramlist")
  {
    // paramlist → dcl
    paramSize++;
    codeGen(curNode->children[0], curProcedure, reg);

    // paramlist → dcl COMMA paramlist
    if (curNode->children.size() == 3)
    {
      codeGen(curNode->children[2], curProcedure, reg);
    }
  }

  else if (curNode->lhs == "statements")
  {
    if (curNode->children.size() > 1)
    {
      // statements → statements statement
      codeGen(curNode->children[0], curProcedure, reg);
      codeGen(curNode->children[1], curProcedure, reg);
    }
  }

  else if (curNode->lhs == "statement")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");

    // statement → lvalue BECOMES expr SEMI
    if (curNode->rhs[0] == "lvalue")
    {
      codeGen(curNode->children[2], curProcedure, reg);
      push(3);

      codeGen(curNode->children[0], curProcedure, reg);
      pop(5);
      asmCode.emplace_back("sw $5, 0($3)");
    }

    // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
    else if (curNode->rhs[0] == "IF")
    {
      ifLabel++;
      string iflbl = "if" + to_string(ifLabel);
      string elselbl = "else" + to_string(ifLabel);

      asmCode.emplace_back("");
      reg = 3;
      codeGen(curNode->children[2], curProcedure, reg);
      asmCode.emplace_back("beq $11, $3, " + iflbl);
      asmCode.emplace_back("bne $11, $3, " + elselbl);

      asmCode.emplace_back("");
      asmCode.emplace_back(iflbl + ": ");
      reg = 3;
      codeGen(curNode->children[5], curProcedure, reg); // if
      asmCode.emplace_back("beq $0, $0, end" + iflbl);

      asmCode.emplace_back("");
      asmCode.emplace_back(elselbl + ": ");
      reg = 3;
      codeGen(curNode->children[9], curProcedure, reg); // else

      asmCode.emplace_back("");
      asmCode.emplace_back("end" + iflbl + ": ");
    }

    // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
    else if (curNode->rhs[0] == "WHILE")
    {
      whileLabel++;
      int curWhileCount = whileLabel;
      string startLabel = "while" + to_string(curWhileCount);

      asmCode.emplace_back("");
      asmCode.emplace_back(startLabel + ": ");
      reg = 3;
      codeGen(curNode->children[2], curProcedure, reg);
      asmCode.emplace_back("bne $11, $3, endWhile" + to_string(curWhileCount));

      reg = 3;
      codeGen(curNode->children[5], curProcedure, reg);
      asmCode.emplace_back("beq $0, $0, " + startLabel);

      asmCode.emplace_back("");
      asmCode.emplace_back("endWhile" + to_string(curWhileCount) + ": ");
    }

    // statement → PUTCHAR LPAREN expr RPAREN SEMI
    else if (curNode->rhs[0] == "PUTCHAR")
    {
      reg = 3;
      codeGen(curNode->children[2], curProcedure, reg);
      asmCode.emplace_back("");
      asmCode.emplace_back("lis $5");
      asmCode.emplace_back(".word 0xffff000c");
      asmCode.emplace_back("sw $3, 0($5)");
    }

    // statement → PRINTLN LPAREN expr RPAREN SEMI
    else if (curNode->rhs[0] == "PRINTLN")
    {
      reg = 3;
      codeGen(curNode->children[2], curProcedure, reg);
      asmCode.emplace_back("");
      push(1);
      println();
    }

    // statement → DELETE LBRACK RBRACK expr SEMI
    else if (curNode->rhs[0] == "DELETE")
    {
      codeGen(curNode->children[3], curProcedure, reg);

      deleteCount++;
      int curDelCount = deleteCount;

      push(1);
      push(31);
      asmCode.emplace_back("");
      asmCode.emplace_back("beq $3, $11, skipDelete" + to_string(curDelCount));
      asmCode.emplace_back("add $1, $3, $0");
      asmCode.emplace_back("lis $5");
      asmCode.emplace_back(".word delete");
      jalr(5);
      pop(31);
      pop(1);
      asmCode.emplace_back("skipDelete" + to_string(curDelCount) + ":");
    }
  }

  else if (curNode->lhs == "expr")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");

    // expr → term
    if (curNode->children.size() == 1)
      codeGen(curNode->children[0], curProcedure, reg);

    // expr → expr PLUS term
    // expr → expr MINUS term
    else
    {
      reg = 3;
      codeGen(curNode->children[0], curProcedure, reg);
      push(3);
      reg = 3;
      codeGen(curNode->children[2], curProcedure, reg);
      pop(5);

      string type1 = curNode->children[0]->type;
      string type2 = curNode->children[2]->type;

      // expr → expr PLUS term
      if (curNode->rhs[1] == "PLUS")
      {
        if (type1 == "int*")
        { 
          // *(a+2)
          asmCode.emplace_back("mult $3, $4");
          asmCode.emplace_back("mflo $3");
        }
        else if (type2 == "int*")
        { 
          // *(2+a)
          asmCode.emplace_back("mult $5, $4");
          asmCode.emplace_back("mflo $5");
        }
        asmCode.emplace_back("add $3, $5, $3");
      }

      // expr → expr MINUS term
      else if (curNode->rhs[1] == "MINUS")
      {
        if (type1 == "int" && type2 == "int")
          asmCode.emplace_back("sub $3, $5, $3");
        else if (type1 == "int*" && type2 == "int")
        {
          asmCode.emplace_back("mult $3, $4");
          asmCode.emplace_back("mflo $3");
          asmCode.emplace_back("sub $3, $5, $3");
        }
        else if (type1 == "int*" && type2 == "int*")
        {
          asmCode.emplace_back("sub $3, $5, $3");
          asmCode.emplace_back("div $3, $4");
          asmCode.emplace_back("mflo $3");
        }
      }
    }
  }

  else if (curNode->lhs == "term")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");

    // term → factor
    if (curNode->rhs[0] == "factor")
      codeGen(curNode->children[0], curProcedure, reg);

    // term → term STAR factor
    // term → term SLASH factor
    // term → term PCT factor
    else if (curNode->rhs[0] == "term")
    {
      reg = 3;
      codeGen(curNode->children[0], curProcedure, reg);
      push(3);
      codeGen(curNode->children[2], curProcedure, reg);
      pop(5);

      if (curNode->rhs[1] == "STAR")
      {
        asmCode.emplace_back("mult $3, $5       ; multiplication");
        asmCode.emplace_back("mflo $3");
      }
      else if (curNode->rhs[1] == "SLASH")
      {
        asmCode.emplace_back("div $5, $3        ; division");
        asmCode.emplace_back("mflo $3");
      }
      else if (curNode->rhs[1] == "PCT")
      {
        asmCode.emplace_back("div $5, $3        ; get remainder");
        asmCode.emplace_back("mfhi $3");
      }
    }
  }

  else if (curNode->lhs == "factor")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");

    // factor → ID
    if (curNode->children.size() == 1 && curNode->rhs[0] == "ID")
    {
      string id = curNode->children[0]->rhs[0];
      string type = procedures[curProcedure]->symbolTable[id].type;
      int offset = procedures[curProcedure]->symbolTable[id].offset;
      asmCode.emplace_back("lw $" + to_string(reg) + ", " + to_string(offset) + "($29)");
    }

    // factor → ID LPAREN RPAREN
    // factor → ID LPAREN arglist RPAREN
    else if (curNode->rhs[0] == "ID" && curNode->rhs[1] == "LPAREN")
    {
      // factor → ID LPAREN RPAREN
      push(29);
      push(31);

      // factor → ID LPAREN arglist RPAREN
      if (curNode->rhs[2] == "arglist")
        codeGen(curNode->children[2], curProcedure, reg);

      string name = curNode->children[0]->rhs[0];
      asmCode.emplace_back("");
      asmCode.emplace_back("lis $5");
      asmCode.emplace_back(".word F" + name);
      jalr(5);

      for (int i = 0; i < procedures[name]->paramSize; ++i)
        pop();

        pop(31);
      pop(29);
    }

    // factor → NUM
    else if (curNode->rhs[0] == "NUM")
    {
      string val = curNode->children[0]->rhs[0];
      asmCode.emplace_back("lis $3");
      asmCode.emplace_back(".word " + val);
    }

    // factor → LPAREN expr RPAREN
    else if (curNode->rhs[0] == "LPAREN")
      codeGen(curNode->children[1], curProcedure, reg);

    // factor → GETCHAR LPAREN RPAREN
    else if (curNode->rhs[0] == "GETCHAR")
    {
      reg = 3;
      getChar();
    }

    // factor → NULL
    else if (curNode->rhs[0] == "NULL")
      asmCode.emplace_back("add $3, $0, $11         ; = NULL\n");

    // factor → AMP lvalue
    else if (curNode->rhs[0] == "AMP")
      codeGen(curNode->children[1], curProcedure, reg);

    // factor → STAR factor
    else if (curNode->rhs[0] == "STAR")
    {
      codeGen(curNode->children[1], curProcedure, reg);
      asmCode.emplace_back("lw $3, 0($3)");
    }

    // factor → NEW INT LBRACK expr RBRACK
    else if (curNode->rhs[0] == "NEW")
    {
      codeGen(curNode->children[3], curProcedure, reg);
      asmCode.emplace_back("");
      push(1);
      push(31);
      asmCode.emplace_back("add $1, $3, $0");
      asmCode.emplace_back("lis $5");
      asmCode.emplace_back(".word new");
      jalr(5);
      pop(31);
      pop(1);
      asmCode.emplace_back("bne $3, $0, 1");
      asmCode.emplace_back("add $3, $11, $0");
    }
  }

  else if (curNode->lhs == "arglist")
  {
    // arglist → expr
    codeGen(curNode->children[0], curProcedure, reg);
    push(3);
    paramSize++;
    // arglist → expr COMMA arglist
    if (curNode->children.size() == 3)
      codeGen(curNode->children[2], curProcedure, reg);
  }

  else if (curNode->lhs == "lvalue")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");

    // lvalue → ID
    if (curNode->rhs[0] == "ID")
    {
      string idName = curNode->children[0]->rhs[0];
      int offset = procedures[curProcedure]->symbolTable[idName].offset;
      asmCode.emplace_back("lis $3");
      asmCode.emplace_back(".word " + to_string(offset));
      asmCode.emplace_back("add $3, $29, $3");
    }

    // lvalue → STAR factor
    else if (curNode->rhs[0] == "STAR")
      codeGen(curNode->children[1], curProcedure, reg);

    // lvalue → LPAREN lvalue RPAREN
    else if (curNode->rhs[0] == "LPAREN")
      codeGen(curNode->children[1], curProcedure, reg);
  }

  else if (curNode->lhs == "test")
  {
    if (asmCode.back() != "")
      asmCode.emplace_back("");
    // test → expr EQ expr
    // test → expr NE expr
    // test → expr LE expr
    // test → expr GE expr
    // test → expr GT expr
    // test → expr LT expr
    reg = 3;
    codeGen(curNode->children[0], curProcedure, reg);
    push(3);
    codeGen(curNode->children[2], curProcedure, reg);
    pop(5);

    string sign = "slt";
    if (curNode->children[0]->type == "int*" || curNode->children[2]->type == "int*")
      sign = "sltu";

    if (curNode->rhs[1] == "EQ")
    {
      asmCode.emplace_back(sign + " $6, $3, $5");
      asmCode.emplace_back(sign + " $7, $5, $3");
      asmCode.emplace_back("add $3, $7, $6");
      asmCode.emplace_back("sub $3, $11, $3");
    }
    else if (curNode->rhs[1] == "NE")
    {
      asmCode.emplace_back(sign + " $6, $3, $5");
      asmCode.emplace_back(sign + " $7, $5, $3");
      asmCode.emplace_back("add $3, $7, $6");
    }
    else if (curNode->rhs[1] == "LT")
    {
      asmCode.emplace_back(sign + " $3, $5, $3");
    }
    else if (curNode->rhs[1] == "LE")
    {
      asmCode.emplace_back(sign + " $3, $3, $5");
      asmCode.emplace_back("sub $3, $11, $3");
    }
    else if (curNode->rhs[1] == "GT")
    {
      asmCode.emplace_back(sign + " $3, $3, $5");
    }
    else if (curNode->rhs[1] == "GE")
    {
      asmCode.emplace_back(sign + " $3, $5, $3");
      asmCode.emplace_back("sub $3, $11, $3");
    }
  }
}

int main()
{
  shared_ptr<Node> root = make_shared<Node>();

  string line;
  getline(cin, line);
  stringstream ss(line);

  ss >> root->lhs;
  string token;
  while (ss >> token)
    root->rhs.emplace_back(token);

  for (int i = 0; i < root->rhs.size(); ++i)
  {
    shared_ptr<Node> curNode = make_shared<Node>();
    root->children.emplace_back(curNode);
    parseTree(curNode);
  }

  genBegin();

  string curProcedure = "";
  int reg = 0;
  codeGen(root, curProcedure, reg);

  printSymbolTable();

  for (const auto &code : asmCode)
    cout << code << endl;
}
