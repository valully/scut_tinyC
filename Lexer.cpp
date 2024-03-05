#include "Lexer.h"

ifstream Lexer::ifs;
ofstream Lexer::ofs;
char Lexer::nextLetter = ' ';
char Lexer::nowLetter = ' ';
unsigned int Lexer::line = 1;
bool Lexer::err = 0;
bool Lexer::retFlag = 0;

map<string, int> Lexer::KEY = {
	{"Ident",-1},{"IntConst",-2},{"FString",-3},
	{"main",-4},{"const",-5},{"int",-6},{"break",-7},
	{"continue",-8},{"if",-9},{"else",-10}, {"return",-11},
	{"!",-12},{"&&",-13},{"||",-14},{"while",-15},{"+",-16},{"-",-17},
	{"void",-18},{"*",-19},{"/",-20},{"%",-21},{"<",-22},{"<=",-23},
	{">",-24},{">=",-25},{"==",-26},{"!=",-27},{"[",-28},{"]",-29},
	{"printf",-30},{"getint",-31},{"=", -32},{";",-33},
	{",",-34}, {"(",-35},{")",-36},{"{",-37},{"}",-38}, {"char", -39}, {"CharConst", -40} };

int Lexer::getTok() {
	if (isalpha(nowLetter) || nowLetter == '_')
		return getIdentifier();

	if (isdigit(nowLetter))
		return getNumber();

	if (nowLetter == '\'')
		return getCharConst();

	if (nowLetter == '"')
		return getString();

	if (ispunct(nowLetter))
		return getSymbol();

	strError("nowLetter");
	return INT_MIN;
}
void Lexer::beginLex() {
	while (nowLetter != EOF) {
		while (isspace(nowLetter)) {
			if (nextLetter == EOF) {
				cout << "success" << endl;
				return;
			}
			if (nowLetter == '\n')
				++line;
			getNextLetter();
		}
		if (nowLetter == '/' && nextLetter == '/') {
			while (nowLetter != '\n')
				getNextLetter();
			getNextLetter();//eat '\n'
			++line;
			continue;
		}
		if (nowLetter == '/' && nextLetter == '*') {
			while (nowLetter != '*' || nextLetter != '/')
				getNextLetter();
			getNextLetter();
			getNextLetter();//eat '*' and '/'
			continue;
		}
		if (err)
			return;
		//ofs << getTok() << " " << word << endl;
		TOKEN = getTok();
		return;
	}
	TOKEN = TOKEOF;
	//cout << "success" << endl;
}