#include <sys/types.h>
#include <fcntl.h>
#include <db.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "Btree.h"
#include "DirectlyMappedArray.h"

using namespace std;
using namespace Btree;

char fileName[100];
const int batchSize = 10000000;
Key_t keys[batchSize];
void bdb(char t, int infile);

int main(int argc, char **argv)
{
	const int required = 4;
	//srand(12874938);
    /*
	unsigned int tm;
	if (argc >= required+1)
		tm = atoi(argv[required]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;
    */
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <input file> <splitter type> <dest dir>"<<endl
			//<<"insertion sequence: S(sequential), D(strided), R(random)"<<endl
			//<<"size: #rows/cols of the square matrix"<<endl
			<<"input file: e.g., S2000 (sequential sequence of 2k x 2k matrix"<<endl
			<<"splitter: M,A,R,S,T,X(Berkeley DB Btree), Y(Berkeley DB recno)"<<endl
			<<"dest dir: where to write the result file (named mb)"<<endl;
			//<<"sparsity: e.g., .4 means 40% of the elements are non-zero"<<endl;
		return 0;
	}

    strcpy(fileName, argv[3]);
	char *infilename = argv[1];
	char splitterType = argv[2][0];

	int infile = open(infilename, O_RDONLY);
    infilename = basename(infilename);

    // if infilename contains 'x', then it contains both dimensions of the
    // matrix; otherwise the two dims are equanl and only one is provided
    Key_t total = 0;
    if (strstr(infilename, "x")) {
        Key_t d1,d2;
        sscanf(infilename+1, "%lux%lu", &d1, &d2);
        total = d1*d2;
    }
    else {
        total = atoi(infilename+1);
        total *= total;
    }

    if (splitterType=='X' || splitterType=='Y') {
        strcat(fileName, "/mbdb");
        bdb(splitterType, infile);
    }
    else if (splitterType == 'D') {
        strcat(fileName, "/mb");
        DirectlyMappedArray dma(fileName, total);
        while (true) {
            ssize_t c = read(infile, keys, sizeof(keys));
            if (!c)
                break;
            int count = c/sizeof(Key_t);
            for (int i=0; i<count; ++i) {
                dma.put(keys[i], 1.0);
            }
            cerr<<count<<" ";
        }
    }
    else {
        strcat(fileName, "/mb");
        BTree *tree = new BTree(fileName, total, splitterType, 'M', config->useDenseLeaf);
        while (true) {
            ssize_t c = read(infile, keys, sizeof(keys));
            if (!c)
                break;
            int count = c/sizeof(Key_t);
            for (int i=0; i<count; ++i) {
                tree->put(keys[i], 1.0);
            }
            cerr<<count<<" ";
        }
        delete tree;
    }

    cerr<<endl;
    // finish up
	close(infile);
}

void bdb(char t, int infile)
{
    DBTYPE type;
    DB *dbp;
    DBT key, data;
    int k;
    double x = 1.0;
    int ret, t_ret;

    if (t == 'X')
        type = DB_BTREE;
    else if (t == 'Y')
        type = DB_RECNO;
    else {
        cerr<<"unknown db type"<<endl;
        exit(1);
    }

    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        exit (1);
    }
    if (dbp->set_cachesize(dbp, 0, 4200*8*1024, 1)) {
        cerr<<"cannot set cache size"<<endl;
        exit(1);
    }
    if ((ret = dbp->open(dbp,
                    NULL, fileName, NULL, type, DB_CREATE|DB_TRUNCATE, 0664)) != 0) {
        dbp->err(dbp, ret, "%s", fileName);
        cerr<<"cannot open db"<<endl;
        exit(1);
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    data.data = &x;
    data.size = sizeof(double);

    // It seems recno only accepts int as keys
	while (true) {
		ssize_t c = read(infile, keys, sizeof(keys));
		int count = c/sizeof(Key_t);
		for (int i=0; i<count; ++i) {
            k = keys[i]+1;
            key.data = &k;
            key.size = sizeof(int);
            if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
                cerr<<"put "<<keys[i]<<"failed"<<endl;
                dbp->err(dbp, ret, "DB->put");
                exit(1);
            }
		}
		if (count < batchSize)
			break;
	}
    /*
    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0)
        printf("db: %d: key retrieved: data was %f.\n",
                *(int *)key.data, *(double *)data.data);
    else {
        dbp->err(dbp, ret, "DB->get");
        goto err;
    }
*/

    if ((t_ret = dbp->close(dbp, 0)) != 0 && ret == 0)
        ret = t_ret; 
}
