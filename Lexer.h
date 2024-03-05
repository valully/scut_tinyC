#pragma once
#include"head.h"
enum tokeneof
{
	TOKEOF = -100
};
class Lexer
{
protected:
	int TOKEN;
	string word;
	static unsigned int line;
	static bool err;
	static bool retFlag;
	static ifstream ifs;
	static ofstream ofs;
	static char nowLetter;
	static char nextLetter;
	static map<string, int> KEY;
public:
	Lexer(string a) {
		ifs.open(a);
		ofs.open(a + ".oo");
		if (!ifs.is_open()) {
			cerr << "Error Filename" << endl;
			err = 1;
		}
	}
	virtual ~Lexer() {
		ifs.close();
		ofs.close();
	}
	void getNextLetter() {
		nowLetter = nextLetter;
		nextLetter = ifs.get();
	}
	void strError(const string& s) {//词法报错
		err = 1;
		cerr << "ERROR: when analysed the word: " << s << " in line: " << line << endl;
	}
	int getTok();//获取token值
	int getIdentifier() {//读取标识符
		word = "";
		while (isalnum(nowLetter)) {
			word += nowLetter;
			getNextLetter();
		}
		if (KEY.count(word))
			return KEY[word];
		return KEY["Ident"];
	}
	int getNumber() {
		word = "";
		while (isdigit(nowLetter)) {
			word += nowLetter;
			getNextLetter();
		}
		if (nowLetter == '.') {//遇到小数进行去尾操作
			getNextLetter();
			while (isdigit(nowLetter)) {
				//word += nowLetter;
				getNextLetter();
			}
		}
		if (isalpha(nowLetter)) {
			word += nowLetter;
			strError(word);
			return INT_MIN;
		}
		return KEY["IntConst"];
	}
	int getSymbol() {
		word = "";
		if (nowLetter == '&' && nextLetter == '&')
		{
			word = "&&";
			getNextLetter(); getNextLetter();
			return KEY["&&"];
		}
		if (nowLetter == '|' && nextLetter == '|')
		{
			word = "||";
			getNextLetter(); getNextLetter();
			return KEY["||"];
		}
		if (nextLetter == '=' && ispunct(nowLetter))
		{
			switch (nowLetter)
			{
			case '<':
			{
				word = "<=";
				getNextLetter(); getNextLetter();
				return KEY["<="];
			}
			case '>':
			{
				word = ">=";
				getNextLetter(); getNextLetter();
				return KEY[">="];
			}
			case'!':
			{
				word = "!=";
				getNextLetter(); getNextLetter();
				return KEY["!="];
			}
			case'=':
			{
				word = "==";
				getNextLetter(); getNextLetter();
				return KEY["=="];
			}
			default:
			{
				word = nowLetter + nextLetter;
				strError(word);
				return INT_MIN;
			}
			}
		}
		word += nowLetter;//特殊符号通常只有一位
		getNextLetter();
		if (KEY.count(word))
			return KEY[word];
		strError(word);
		return INT_MIN;
	}
	int getString() {
		word = "";
		do {
			if (nowLetter == '\n') {
				strError(word);
				return INT_MIN;
			}
			word += nowLetter;
			getNextLetter();
		} while (nowLetter != '"');
		word += nowLetter;//eat "
		getNextLetter();
		return KEY["FString"];
	}
	int getCharConst() {
		word = "'";
		getNextLetter();
		if (nowLetter == '\n') {
			strError(word);
			return INT_MIN;
		}
		//任意字符
		word += nowLetter;
		getNextLetter();
		if (nowLetter != '\'') {
			strError(word);
			return INT_MIN;
		}
		word += nowLetter;
		getNextLetter();
		return KEY["CharConst"];
	}

	void beginLex();
};

