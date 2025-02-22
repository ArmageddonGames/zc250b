#include "DataStructs.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <iostream>

int StackFrame::getOffset(int vid)
{
	map<int, int>::iterator it = stackoffset.find(vid);
	if(it == stackoffset.end())
	{
		box_out("Internal Error: Can't find variable stack offset!");
		box_eol();
		return 0;
	}
	return stackoffset[vid];
}

int SymbolTable::getVarType(AST *obj)
{
	map<AST *, int>::iterator it = astToID.find(obj);
	if(it == astToID.end())
	{
		box_out("Internal Error: Can't find the AST ID!");
		box_eol();
		return -1;
	}
	return getVarType(it->second);
}

int SymbolTable::getVarType(int varID)
{
	map<int, int>::iterator it = varTypes.find(varID);
	if(it == varTypes.end())
	{
		box_out("Internal Error: Can't find the variable type!");
		box_eol();
		return -1;
	}
	return it->second;
}

bool SymbolTable::isConstant(string name)
{
	return constants->find(name) != constants->end();
}

long SymbolTable::getConstantVal(string name)
{
	return (*constants)[name];
}

int VariableSymbols::addVariable(string name, int type)
{
	
	map<string, pair<int, int> >::iterator it = symbols.find(name);
	if(it != symbols.end())
		return -1;
	int id = ScriptParser::getUniqueVarID();
	symbols[name] = pair<int, int>(type, id);
	return id;
}

bool VariableSymbols::containsVariable(string name)
{
	return (symbols.find(name) != symbols.end());
}

int VariableSymbols::getID(string name)
{
	map<string, pair<int, int> >::iterator it = symbols.find(name);
	assert(it != symbols.end());
	return it->second.second;
}

int FunctionSymbols::addFunction(string name, int rettype, vector<int> paramtype)
{
	map<pair<string, vector<int> >, pair<int, int> >::iterator it = symbols.find(pair<string, vector<int> >(name, paramtype));
	if(it != symbols.end())
		return -1;
	int id = ScriptParser::getUniqueFuncID();
	symbols[pair<string, vector<int> >(name, paramtype)] = pair<int, int>(rettype, id);
	map<string, vector<int> >::iterator it2 = ambiguous.find(name);
	if(it2 == ambiguous.end())
	{
		vector<int> newv;
		newv.push_back(id);
		ambiguous[name] = newv;
	}
	else
	{
		(*it2).second.push_back(id);
	}
	return id;
}

bool FunctionSymbols::containsFunction(string name, vector<int> &params)
{
	return symbols.find(pair<string, vector<int> >(name,params)) != symbols.end();
}

int FunctionSymbols::getID(string name, vector<int> &params)
{
	map<pair<string, vector<int> >, pair<int, int> >::iterator it = symbols.find(pair<string, vector<int> >(name, params));
	assert(it != symbols.end());
	return it->second.second;
}

vector<int> FunctionSymbols::getFuncIDs(string name)
{
	map<string, vector<int> >::iterator it = ambiguous.find(name);
	if(it == ambiguous.end())
		return vector<int>();
	return it->second;
}

bool Scope::addNamedChild(string name, Scope *child)
{
	map<string, Scope *>::iterator it = namedChildren.find(name);
	if(it != namedChildren.end())
		return false;
	namedChildren[name]=child;
	return true;
}

Scope::~Scope()
{
	map<string, Scope *>::iterator it;
	for(it = namedChildren.begin(); it != namedChildren.end(); it++)
	{
		delete it->second;
	}
	namedChildren.clear();
}

int Scope::getVarInScope(string nspace, string name)
{
    if(nspace == "" && getVarSymbols().containsVariable(name))
		return getVarSymbols().getID(name);
	map<string, Scope *>::iterator it = namedChildren.find(nspace);
	if(it != namedChildren.end())
	{
		int id = (*it).second->getVarInScope("", name);
		if(id != -1)
			return id;
	}
	if(parent == NULL)
		return -1;
	return parent->getVarInScope(nspace, name);
}

vector<int> Scope::getFuncsInScope(string nspace, string name)
{
	vector<int> rval;
	if(nspace == "")
	{
		vector<int> thisscope = getFuncSymbols().getFuncIDs(name);
		for(vector<int>::iterator it = thisscope.begin(); it != thisscope.end(); it++)
		{
			rval.push_back(*it);
		}
	}
	map<string, Scope *>::iterator it = namedChildren.find(nspace);
	if(it != namedChildren.end())
	{
		vector<int> childscope = (*it).second->getFuncsInScope("", name);
		for(vector<int>::iterator it2 = childscope.begin(); it2 != childscope.end(); it2++)
		{
			rval.push_back(*it2);
		}
	}
	if(parent != NULL)
	{
		vector<int> abovescope = parent->getFuncsInScope(nspace,name);
		for(vector<int>::iterator it2 = abovescope.begin(); it2 != abovescope.end(); it2++)
		{
			rval.push_back(*it2);
		}
	}
	return rval;
}

Scope *Scope::getNamedChild(string name)
{
	 return namedChildren[name];
}

void SymbolTable::putFunc(int ID, int type)
{
	funcTypes[ID]=type; 
}

void SymbolTable::putAST(AST *obj, int ID)
{
	astToID[obj]=ID;
	}

int SymbolTable::getFuncType(AST *obj)
{
	return getFuncType(astToID[obj]);
}
  
void SymbolTable::printDiagnostics()
{
	cout << (unsigned int)varTypes.size() << " variable symbols" << endl;
	cout << (unsigned int)funcTypes.size() << " function symbols" << endl;
	cout << (unsigned int)funcParams.size() << " function declarations (should = function symbols)" << endl;
}
 
