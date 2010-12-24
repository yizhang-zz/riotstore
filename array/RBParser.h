#ifndef RBPARSER_H
#define RBPARSER_H

#include "Parser.h"
#include <vector>
#include <fstream>

/* Author: Kevin Jang
 * 11/17/2010
 * RBParser.h
 *
 * Type of Parser class describing how a RB compressed column format is
 * parsed and batch-loaded into storage
 *
 */

template<int nDim>
class RBParser: public Parser<nDim>
{
 public:

    RBParser(const char *fileName);

    int parse(int bufferSize, MDCoord<nDim> *coord, Datum_t *datum);

 private:
    std::ifstream in;
    bool headerRead;
  int numRows;
  int numCols;
  int col;
  int numValues;
  int rowIdx;
  int colIdx;
  int valIdx;
  int colptr;
  int colRead;
  int colPerLine;
  int rowPerLine;
};

#endif
