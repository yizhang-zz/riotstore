/* -*- mode: c++; c-basic-offset:2 -*- */
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

typedef std::pair<unsigned, Key_t> IndexKeyPair;

class ExternalSort{
  
 public:
  ExternalSort(const char*, const uint64_t);
  ExternalSort(const uint64_t);
  ~ExternalSort();
  bool done;
  //void fileToChunk();
  void mergeSortToFile();
  void streamToChunk(Entry *entries, const int length);

  int mergeSortToStream(Entry*, uint64_t);
  int getRecordCount(){return m_recordCount;}
  int getBufferSize(){return m_bufferSize;}
  int getSortedCount(){return m_sortedCount;}
  double getTimeTaken(){return end_run - begin_run;}

 private:
  std::string m_fileName;
  const uint64_t m_bufferSize;
  unsigned int m_recordCount;
  unsigned int m_sortedCount;
  unsigned int m_chunkCount;
  std::ifstream* ifile;
  std::vector<IndexKeyPair> v;
  double begin_run, end_run;
  timeval tim;
  bool streamsInitialized;

  void genTempFileName(char *s, unsigned number);
};

#endif
