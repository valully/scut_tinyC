#include"Parse.h"

int main() {
	ifstream ifs;
	string a;
	cout << "�����ļ���ַ��" << endl;
	cin >> a;
	ifs.open(a);
	if (!ifs.is_open())
		cerr << "Error Filename" << endl;
	else {


		Parse par(a);
		par.beginParse();
	}

}