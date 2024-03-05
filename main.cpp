#include"Parse.h"

int main() {
	ifstream ifs;
	string a;
	cout << "输入文件地址：" << endl;
	cin >> a;
	ifs.open(a);
	if (!ifs.is_open())
		cerr << "Error Filename" << endl;
	else {


		Parse par(a);
		par.beginParse();
	}

}