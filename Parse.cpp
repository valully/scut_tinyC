#include "Parse.h"
int Parse::t = 0;
void Parse::ErrorP(const string& error) {
	err = 1;
	cerr << error << "in line: " << line << endl;
	exit(0);
}

void Parse::parseCompUnit() {
	parseFuncDef();
}

void Parse::parseDecl() {
	switch (TOKEN)
	{
	case -5: {
		parseConstDecl();
		break;
	}
	case -6: {
		parseVarDecl();
		break;
	}
	case -39: {
		parseCharDecl();
		break;
	}
	default: {
		return;
	}
	}
}

void Parse::parseCharDecl() {
	if (TOKEN == -39) {
		do {
			beginLex();
			parseCharDef();
		} while (TOKEN == -34);
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
	}
}

void Parse::parseCharDef() {
	if (TOKEN != -1) {
		ErrorP("Expected an Ident ");
	}
	string name = word;
	if (VarMap.count(word)) {
		ErrorP("变量名" + name + "重复定义！！");
	}
	VarMap[name] = 2;//char类型
	beginLex();
	if (TOKEN == -32) {
		beginLex();
		string temp = "T" + to_string(t);
		++t;
		emit("itc", parseInitVal(), "_", temp);
		++nextstat;
		emit(":=", temp, "_", name);
		++nextstat;
	}
}

void Parse::parseConstDecl() {
	beginLex();//eat "const"
	if (TOKEN == -6 || TOKEN == -39) {//去掉BType函数
		do {
			beginLex();
			parseConstDef();
		} while (TOKEN == -34);
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
	}
}

void Parse::parseConstDef() {
	if (TOKEN != -1) {
		ErrorP("Expected an Ident ");
	}
	string name = word;
	if (VarMap.count(name)) {
		ErrorP("常量名" + name + "重复定义！！");
	}
	beginLex();
	if (TOKEN != -32) {
		ErrorP("Expected: \"=\" ");
	}
	beginLex();
	emit(":=", parseConstInitVal(), "_", name);
	++nextstat;
}

string Parse::parseConstInitVal() {
	return parseConstExp();
}

string Parse::parseConstExp() {
	return parseAddExp();
}

void Parse::parseVarDecl() {
	if (TOKEN == -6) {
		do {
			beginLex();
			parseVarDef();
		} while (TOKEN == -34);
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
	}
}

void Parse::parseVarDef() {
	if (TOKEN != -1) {
		ErrorP("Expected an Ident ");
	}
	string name = word;
	if (VarMap.count(word)) {
		ErrorP("变量名" + name + "重复定义！！");
	}
	VarMap[name] = 1;
	beginLex();
	if (TOKEN == -32) {
		beginLex();
		string temp = parseInitVal();
		if (VarMap[temp] == 2) {
			string temp2 = "T" + to_string(t);
			emit("cti", temp, "_", temp2);
			++t, ++nextstat;
			emit(":=", temp2, "_", name);
			++nextstat;
			return;
		}
		emit(":=", temp, "_", name);
		++nextstat;
	}
}

string Parse::parseInitVal() {
	return parseExp();
}

void Parse::parseFuncDef() {
	parseFuncType();
	beginLex();
	if (TOKEN != -4) {
		ErrorP("Expected: \"main\" ");
	}
	beginLex();
	if (TOKEN != -35) {
		ErrorP("Expected: \"(\" ");
	}
	beginLex();//eat "("
	if (TOKEN != -36) {
		ErrorP("Expected: \")\" ");
	}
	beginLex();
	parseBlock();
}

void Parse::parseFuncType() {
	if (TOKEN != -6) {
		ErrorP("Expected: \"int\" ");
	}
}


void Parse::parseBlock() {
	if (TOKEN != -37) {
		ErrorP("Expected: \"{\" ");
	}
	beginLex();//eat "{"
	while (TOKEN != -38) {
		parseBlockItem();
	}

	if (TOKEN != -38) {
		ErrorP("Expected: \"}\" ");
	}
}

void Parse::parseBlockItem() {
	if (TOKEN == -5 || TOKEN == -6 || TOKEN == -39) {
		parseDecl();
		beginLex();
	}
	else {
		stack<int> CHAIN;
		parseStmt(CHAIN);
	}
}

void Parse::parseStmt(stack<int>& CHAIN) {
	switch (TOKEN)
	{
	case -9: {//if
		beginLex();
		if (TOKEN != -35) {
			ErrorP("Expected: \"(\" ");
		}
		beginLex();
		Rop tempE = parseCond();//条件归约
		if (TOKEN != -36) {
			ErrorP("Expected: \")\" ");
		}
		beginLex();
		if (TOKEN != -37) {
			backpatch(tempE.ETrue, nextstat);
			CHAIN.push(tempE.EFalse);
			parseStmt(CHAIN);
			CHAIN.push(merge(tempE.EFalse, CHAIN.top()));
		}
		else
		{
			backpatch(tempE.ETrue, nextstat);
			parseBlock();
			beginLex();
		}
		emit("jump", "_", "_", "0");
		int temp1 = nextstat;
		++nextstat;
		backpatch(tempE.EFalse, nextstat);
		if (TOKEN == -10) {//else
			beginLex();
			if (TOKEN != -37) {
				CHAIN.push(tempE.EFalse);
				parseStmt(CHAIN);
				CHAIN.push(merge(tempE.EFalse, CHAIN.top()));
			}
			else
			{
				parseBlock();
				beginLex();
			}
		}
		backpatch(temp1, nextstat);
		break;
	}
	case -15: {//while
		beginLex();
		if (TOKEN != -35) {
			ErrorP("Expected: \"(\" ");
		}
		beginLex();
		int codebegin = nextstat;
		Rop tempE = parseCond();//条件归约	
		if (TOKEN != -36) {
			ErrorP("Expected: \")\" ");
		}
		beginLex();
		if (TOKEN != -37) {
			backpatch(tempE.ETrue, nextstat);
			CHAIN.push(tempE.EFalse);
			parseStmt(CHAIN);
			CHAIN.push(merge(tempE.EFalse, CHAIN.top()));
		}
		else
		{
			backpatch(tempE.ETrue, nextstat);
			parseBlock();
			beginLex();
		}
		emit("jump", "_", "_", "0");
		int temp1 = nextstat;
		++nextstat;
		backpatch(tempE.EFalse, nextstat);
		backpatch(temp1, codebegin);
		break;
	}
	case -1: {
		string name = word;
		if (!VarMap.count(name)) {
			ErrorP("变量" + name + "未定义！！");
		}
		if (VarMap[name] == 0) {
			ErrorP("常量不能修改！！");
		}
		beginLex();
		if (TOKEN != -32) {
			ErrorP("Expected: \"=\" ");
		}
		beginLex();
		string tempVal = parseExp();
		if (VarMap[name] == VarMap[tempVal] || VarMap[name] == 1 && VarMap[tempVal] == 0)
			emit(":=", tempVal, "_", name);
		else if (VarMap[name] == 0)
			ErrorP("Const Value Can't Be Changed ");
		else if (VarMap[name] == 1)
		{
			string temp100 = 'T' + to_string(t);
			emit("cti", tempVal, "_", temp100);
			++nextstat;
			++t;
			emit(":=", temp100, "_", name);
		}
		else
		{
			string temp100 = 'T' + to_string(t);
			emit("itc", tempVal, "_", temp100);
			++nextstat;
			++t;
			emit(":=", temp100, "_", name);
		}
		++nextstat;
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
		beginLex();
		CHAIN.push(0);//赋值句无出口
		return;
	}
	case -38:
		return;
	case -11: {//return
		beginLex();
		if (TOKEN != -2) {
			ErrorP("Expected: \"IntConst\" ");
		}
		beginLex();//忽略0
		//parseExp();
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
		retFlag = 1;
		beginLex();//eat ;
		break;
	}
	default: {
		parseExp();
		if (TOKEN != -33) {
			ErrorP("Expected: \";\" ");
		}
		beginLex();
	}
	}
}

int Parse::parseNumber() {
	if (word.size() > 10) {
		cerr << "NAN number\n";
		err = 1;
		return 0;
	}
	Number = 0;
	for (int i = 0; i < word.size(); ++i) {
		Number *= 10;
		Number += word[i] - '0';
	}
	if (Number > INT_MAX) {
		cerr << "NAN number\n";
		err = 1;
		return 0;
	}
	return 1;
}

string Parse::parseExp() {
	return parseAddExp();
}

string Parse::parseAddExp() {
	string t1 = parseMulExp(), t3;
	switch (TOKEN)
	{
	case -16: {// +
		string t2 = parseAddExp2();
		if (t2 == "")
			return "";
		t3 = "T" + to_string(t);
		emit("+", t1, t2, t3);
		break;
	}
	case -17: {// -
		string t2 = parseAddExp2();
		if (t2 == "")
			return "";
		t3 = "T" + to_string(t);
		emit("-", t1, t2, t3);
		break;
	}
	default:
		return t1;
	}
	++t, ++nextstat;
	return t3;
}

string Parse::parseAddExp2() {
	string t1, t2, t3;
	switch (TOKEN)
	{
	case -16: {
		beginLex();
		t1 = parseMulExp();
		t2 = parseAddExp2();
		t3 = "T" + to_string(t);
		if (t2 == "") {
			return t1;
		}
		else {
			emit("+", t1, t2, t3);
		}
		break;
	}
	case -17: {
		beginLex();
		t1 = parseMulExp();
		t2 = parseAddExp2();
		t3 = "T" + to_string(t);
		if (t2 == "") {
			emit("-", "0", t1, t3);
		}
		else {
			emit("-", t2, t1, t3);
		}
		break;
	}
	default: {
		return "";
	}
	}
	++t, ++nextstat;
	return t3;
}

string Parse::parseMulExp() {
	string t1 = parseUnaryExp(), t2;
	beginLex();
	switch (TOKEN)
	{
	case -19: {
		t2 = parseMulExp2();
		string t3 = "T" + to_string(t);
		if (t2 != "") {
			emit("*", t1, t2, t3);
			++t, ++nextstat;
			return t3;
		}
		break;
	}
	case -20: {
		t2 = parseMulExp2();
		string t3 = "T" + to_string(t);
		if (t2 != "") {
			emit("/", t1, t2, t3);
			++t, ++nextstat;
			return t3;
		}
		break;
	}
	case -21: {
		t2 = parseMulExp2();
		string t3 = "T" + to_string(t);
		if (t2 != "") {
			emit("%", t1, t2, t3);
			++t, ++nextstat;
			return t3;
		}
		break;
	}
	}
	return t2 == "" ? t1 : t2;
}

string Parse::parseMulExp2() {
	string t1, t2, t3;
	if (TOKEN == -19 || TOKEN == -20 || TOKEN == -21) {
		beginLex();
		t1 = parseUnaryExp();
		beginLex();
		t2 = parseMulExp2();
		if (t2 == "") {
			return t1;
		}

	}
	else {
		return "";
	}
	return t3;
}

string Parse::parseUnaryExp() {
	if (TOKEN == -35 || TOKEN == -2 || TOKEN == -1 || TOKEN == -40) {
		string temp = parsePrimaryExp();
		return temp;
	}
	else {
		int temp = parseUnaryOp();
		beginLex();
		parseUnaryExp();
		string t1 = "T" + to_string(t - 1), t2 = "T" + to_string(t);
		++t;//临时变量
		if (temp == 1) {
			emit("+", "0", t1, t2);
			++nextstat;
			return t2;
		}
		else if (!temp) {
			emit("-", "0", t1, t2);
			++nextstat;
			return t2;
		}
	}
	return "";
}

string Parse::parsePrimaryExp() {
	switch (TOKEN)
	{
	case -35: {
		beginLex();//eat "("
		parseExp();
		if (TOKEN != -36) {
			ErrorP("Expected: \")\" ");
			return "";
		}
		return "T" + to_string(t - 1);
	}
	case -2: {
		parseNumber();
		if (!err) {
			string temp = "T" + to_string(t);
			++t;
			emit(":=", word, "_", temp);
			++nextstat;
			return temp;
		}
		break;
	}
	case -1: {//变量
		string name = word;
		if (!VarMap.count(name)) {
			ErrorP("变量" + name + "未定义！！");
			return "";
		}
		return name;
	}
	case -40: {
		if (!err) {
			string temp = "T" + to_string(t);
			++t;
			emit("cti", word, "_", temp);
			++nextstat;
			return temp;
		}
		break;
	}
	default: {
		ErrorP("Expected a PrimaryExp ");
		return "";
	}
	}
	return "";
}

int Parse::parseUnaryOp() {
	switch (TOKEN)
	{
	case -16: {
		return 1;
	}
	case -17: {
		return 0;
	}
	case -12: {// !
		return 2;
	}
	default: {
		ErrorP("Expected: \"}\" ");
		//ErrorP("Expected an UnaryOp ");
		return -1;
	}
	}
}

Rop Parse::parseCond() {
	return parseLOrExp();
}

Rop Parse::parseRelExp() {
	Rop E1, E2, E3;
	E1.name = parseAddExp();
	switch (TOKEN)
	{
	case -22: {// <
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("j<", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -23: {// <=
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("j<=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -24: {// >
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("j>", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -25: {// >=
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("j>=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	default:
		E3.name = E1.name;
	}
	return E3;
}

Rop Parse::parseRelExp2() {
	Rop E1, E2, E3;
	switch (TOKEN)
	{
	case -22: {// <
		beginLex();
		E1.name = parseAddExp();
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.codebegin == 0) {
			return E1;
		}
		else if (E2.name != "") {
			emit("j<", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -23: {// <=
		beginLex();
		E1.name = parseAddExp();
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.codebegin == 0)
			return E1;
		if (E2.name != "") {
			emit("j<=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -24: {// >
		beginLex();
		E1.name = parseAddExp();
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.codebegin == 0) {
			return E1;
		}
		if (E2.name != "") {
			emit("j>", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -25: {// >=
		beginLex();
		E1.name = parseAddExp();
		E2 = parseRelExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (!E2.codebegin) {
			return E1;
		}
		if (E2.name != "") {
			emit("j>=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	}
	return E3;
}

Rop Parse::parseEqExp() {
	Rop E1, E2, E3;
	E1 = parseRelExp();
	switch (TOKEN)
	{
	case -26: {//==
		E2 = parseEqExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("==", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -27: {//!=
		E2 = parseEqExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (E2.name != "") {
			emit("!=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	default:
		return E1;
	}
	return E3;
}

Rop Parse::parseEqExp2() {
	Rop E1, E2, E3;
	switch (TOKEN)
	{
	case -26: {// ==
		beginLex();
		E1 = parseRelExp();
		E2 = parseEqExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (!E2.codebegin)
			return E1;
		if (E2.name != "") {
			emit("==", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
		break;
	}
	case -27: {// !=
		beginLex();
		E1 = parseRelExp();
		E2 = parseEqExp2();
		E3.codebegin = nextstat;
		E3.ETrue = nextstat;
		E3.EFalse = nextstat + 1;
		if (!E2.codebegin)
			return E1;
		if (E2.name != "") {
			emit("!=", E1.name, E2.name, "0");
			++nextstat;
			emit("jump", "_", "_", "0");
			++nextstat;
		}
	}
	}
	return E3;
}

Rop Parse::parseLAndExp() {
	Rop E1, E2, E3;
	stack<int> EFalse;
	E1 = parseEqExp();
	int temp = nextstat;
	EFalse.push(E1.EFalse);
	E2 = parseLAndExp2(EFalse);
	E3.codebegin = E1.codebegin;
	if (E2.codebegin == 0) {
		if (TOKEN == -13) { backpatch(E1.ETrue, nextstat); }
		E3.ETrue = E1.ETrue;
		E3.EFalse = E1.EFalse;
	}
	else {
		backpatch(E1.ETrue, temp);
		E3.ETrue = E2.ETrue;
		E3.EFalse = merge(E1.EFalse, E2.EFalse);
	}
	int temp1 = EFalse.top(), temp2 = 0;
	EFalse.pop();
	while (!EFalse.empty()) {
		temp2 = EFalse.top();
		EFalse.pop();
		backpatch(temp1, temp2);
		temp1 = temp2;
	}
	E3.EFalse = temp1;
	return E3;
}

Rop Parse::parseLAndExp2(stack<int>& EFalse) {
	Rop E1, E2, E3;
	if (TOKEN == -13) {// &&
		beginLex();
		E1 = parseEqExp();
		EFalse.push(E1.EFalse);
		int temp = nextstat;
		E2 = parseLAndExp2(EFalse);
		E3.codebegin = E1.codebegin;
		if (E2.codebegin == 0) {
			E3.ETrue = E1.ETrue;
			backpatch(E1.ETrue, nextstat);
			E3.EFalse = nextstat - 1;
		}
		else {
			backpatch(E1.ETrue, temp);
			E3.ETrue = E2.ETrue;
			E3.EFalse = merge(E1.EFalse, E2.EFalse);
		}
	}
	return E3;
}

Rop Parse::parseLOrExp() {
	Rop E1, E2, E3;
	stack<int> ETrue;
	E1 = parseLAndExp();
	int temp = nextstat;
	ETrue.push(E1.ETrue);
	E2 = parseLOrExp2(ETrue);
	E3.codebegin = E1.codebegin;
	if (E2.codebegin == 0) {
		backpatch(E1.ETrue, nextstat);
		E3.ETrue = E1.ETrue;
		E3.EFalse = E1.EFalse;
	}
	else {
		backpatch(E1.EFalse, temp);
		E3.ETrue = merge(E1.ETrue, E2.ETrue);
		E3.EFalse = E2.EFalse;
	}
	int temp1 = ETrue.top(), temp2 = 0;
	ETrue.pop();
	while (!ETrue.empty()) {
		temp2 = ETrue.top();
		ETrue.pop();
		backpatch(temp1, temp2);
		temp1 = temp2;
	}
	E3.ETrue = temp1;
	return E3;
}

Rop Parse::parseLOrExp2(stack<int>& ETrue) {
	Rop E1, E2, E3;
	if (TOKEN == -14) {// ||
		beginLex();
		E1 = parseLAndExp();
		ETrue.push(E1.ETrue);
		int temp = nextstat;
		E2 = parseLOrExp2(ETrue);
		E3.codebegin = E1.codebegin;
		if (E2.codebegin == 0)
		{
			E3.ETrue = merge(E1.ETrue, 0);
			backpatch(E1.ETrue, nextstat);
			E3.EFalse = nextstat - 1;
		}
		else
		{
			backpatch(E1.EFalse, temp);
			E3.ETrue = merge(E1.ETrue, E2.ETrue);
			E3.EFalse = E2.EFalse;
		}
	}
	return E3;
}
