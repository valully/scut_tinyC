#pragma once
#include"Lexer.h"

struct retType {
	int codebegin;
	string op;
	string arg1;
	string arg2;
	string ret;
	friend ostream& operator<<(ostream& output, const retType& a) {
		output << "(" << a.codebegin << ")" << " [ " << a.op << " , " << a.arg1 << " , " << a.arg2 << " , " << a.ret << " ]\n";
		return output;
	}
};
struct Rop {
	int codebegin;
	int ETrue;
	int EFalse;
	string name;
	Rop(int codebegin = 0, int ETrue = 0, int EFalse = 0, string name = "") :
		codebegin(codebegin), ETrue(ETrue), EFalse(EFalse), name(name) {};
};
class Parse :public Lexer
{
	long long int Number;
	int nextstat;
	map<string, int> VarMap;//0为常量， 1为变量，2为char类型,3为const char
	map<string, int> tempIntMap;
	map<string, char> tempCharMap;
	vector<retType> output;
	static int t;
public:
	Parse(string a) :Lexer(a) {
		nextstat = 1;

		output.push_back({ 0,"0","0","0","0" });
	}
	~Parse() {
		ifs.close();
		ofs.close();
	}
	void ErrorP(const string&);
	void emit(string op, string arg1, string arg2, string ret) {
		retType temp;
		temp.codebegin = nextstat;
		temp.op = op;
		temp.arg1 = arg1;
		temp.arg2 = arg2;
		temp.ret = ret;
		output.push_back(temp);
	}
	int merge(int p1, int p2) {
		if (p2 == 0)
			return p1;
		output[p2].ret = "(" + to_string(output[p1].codebegin) + ")";
		return p2;
	}
	void backpatch(int p, int t) {
		output[p].ret = "(" + to_string(t) + ")";
	}
	void parseCompUnit();

	void parseDecl();
	void parseConstDecl();
	void parseConstDef();
	string parseConstInitVal();
	string parseConstExp();
	void parseVarDecl();
	void parseVarDef();
	string parseInitVal();

	void parseFuncDef();
	void parseFuncType();
	void parseBlock();

	void parseBlockItem();

	void parseStmt(stack<int>& CHAIN);
	int parseNumber();

	string parseExp();
	string parseAddExp();
	string parseMulExp();
	string parseUnaryExp();
	string parsePrimaryExp();
	int parseUnaryOp();

	string parseAddExp2();//返回值为临时变量
	string parseMulExp2();

	Rop parseCond();
	Rop parseRelExp();
	Rop parseRelExp2();
	Rop parseEqExp();
	Rop parseEqExp2();
	Rop parseLAndExp();
	Rop parseLAndExp2(stack<int>&);
	Rop parseLOrExp();
	Rop parseLOrExp2(stack<int>&);

	void parseCharDecl();
	void parseCharDef();
	void beginParse() {
		beginLex();
		parseCompUnit();
		for (int i = 1; i < output.size(); ++i) {
			cout << output[i];
		}
	}
};

