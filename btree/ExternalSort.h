#ifndef EXTERNALSORT_H
#define EXTERNALSORT_H

#include "../lower/BitmapPagedFile.h"
#include "../common/common.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <algorithm>
#include <utility>
#include <vector>
#include <functional>

using namespace std;

typedef pair<uint64_t,uint64_t> IndexKeyPair;

class ExternalSort{
  
 public:
  ExternalSort(const char*, const uint64_t);
  ExternalSort(const uint64_t);
  bool done;
  void fileToChunk();
  void mergeSortToFile();
  void streamToChunk(Key_t *, Datum_t *, const int length);

  int mergeSortToStream(Entry*, uint64_t);
  int getRecordCount(){return m_recordCount;}
  int getBufferSize(){return m_bufferSize;}
  int getSortedCount(){return m_sortedCount;}
  double getTimeTaken(){return end_run - begin_run;}
  static const int keySize = 32;

 private:
  const char* m_fileName;
  const uint64_t m_bufferSize;
  unsigned int m_recordCount;
  unsigned int m_sortedCount;
  unsigned int m_chunkCount;
  ifstream* ifile;
  vector<IndexKeyPair> v;
  double begin_run, end_run;
  timeval tim;

  bool streamsInitialized;
  bool isDoneSorting(ifstream *ifile, unsigned int size);

  void print(const string & str){cout << "[ExternalSort] " << str << endl;}
  void print(const string & str, int i){
    cout << "[ExternalSort] " << str << " = " << i << endl;
  }
};

#endif
