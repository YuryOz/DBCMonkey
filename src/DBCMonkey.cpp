#include <iostream>
#include"DBCParser.h"

using namespace std;

int main(int argc, char **argv) {
	if(argc < 2){
		cout << "Usage: DBCMonkey dbcfile [channel]"  << endl;
		return 0;
	}

	uint32_t channel = 0;

	if(argc >= 3) {
		channel = stoi(argv[2]);
	}

	DBCParser p;
	p.ReadDBC(argv[1], channel);

	p.Print();

	return 0;
}
