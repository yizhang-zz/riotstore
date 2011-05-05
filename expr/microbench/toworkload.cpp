#include <iostream>
#include <fcntl.h>
#include "common/common.h"
#include "array/MDCoord.h"
#include "array/MMParser.h"

using namespace std;

struct compS {
    bool operator()(const MDCoord<2> &a, const MDCoord<2> &b) {
        return a[0] < b[0] || (a[0]==b[0] && a[1] < b[1]);
    }
};

struct compD {
    bool operator()(const MDCoord<2> &a, const MDCoord<2> &b) {
        return a[1] < b[1] || (a[1]==b[1] && a[0] < b[0]);
    }
};

struct compI {
    bool operator()(const MDCoord<2> &a, const MDCoord<2> &b) {
        bool incol1 = a[0] > a[1]; // if a is on a column 
        bool incol2 = b[0] > b[1];
        i64 i = incol1 ? a[1] : a[0];
        i64 j = incol2 ? b[1] : b[0];
        if (i < j)
            return true;
        if (i == j &&
            (!incol1 && incol2 || 
                    !incol1 & !incol2 && a[1] < b[1] ||
                    incol1 && incol2  && a[0] < b[0]))
            return true;
        return false;
    }
};

int main(int argc, char **argv)
{
	const int required = 4;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <inputformat> <inputfile> <type> "<<endl
			<<"  <inputformat>: MM"<<endl
			<<"  <type>: S, D, R, I"<<endl;
		return 0;
	}

    Parser<2> *parser = Parser<2>::createParser(argv[1], argv[2]);
	Key_t total = parser->size();
    MDCoord<2> dim = parser->dim();

	char type = argv[3][0];
	char filename[100];
	sprintf(filename, "%c%"PRId64"x%"PRId64, type, dim[0], dim[1]);
	int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);

	Key_t *keys = new Key_t[total];
    MDCoord<2> *coords = new MDCoord<2>[total];
    Datum_t *data = new Datum_t[total];
    parser->parse(total, coords, data);

	switch (type) {
	case 'S':
        std::sort(coords, coords+total, compS());
		break;
    case 'R':
        permute(coords, total);
		break;
	case 'D':
        std::sort(coords, coords+total, compD());
		break;
	case 'I':
        std::sort(coords, coords+total, compI());
		break;
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

    for (Key_t i=0; i<total; ++i) {
        keys[i] = coords[i][0] * dim[1] + coords[i][1];
    }

	write(file, keys, sizeof(Key_t)*total);
	close(file);
    delete[] keys;
    delete[] data;
    delete[] coords;
	return 0;
}
