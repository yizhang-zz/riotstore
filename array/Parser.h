#ifndef PARSER_H
#define PARSER_H

#include "../common/common.h"
#include "MDCoord.h"
#include <limits.h>
#include <iostream>
#include <fstream>

using namespace std;

/* Author: Kevin Jang
 * 11/17/2010
 * Parser.h
 *
 * Base class describing how a file must be read in so that MDArray can
 * include in its constructor the type of parser
 *
 */

template<int nDim>
class Parser
{
 public:

  Parser() {}
  virtual ~Parser() {}

  virtual int parse(ifstream &in, int bufferSize, MDCoord<nDim> *coord, Datum_t *datum) = 0;

  //  MDCoord<nDim> getDimension(){return dim;}

  //virtual i64 getRecordCount() {return nElem;}

  i64 nElem;

 protected:
  
  //  MDCoord<nDim> dim;
};


#endif
