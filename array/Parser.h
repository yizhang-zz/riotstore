#ifndef PARSER_H
#define PARSER_H

#include "../common/common.h"
#include "MDCoord.h"
#include <limits.h>
#include <iostream>
#include <fstream>

/* Author: Kevin Jang, Yi Zhang
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

    static Parser<nDim> *createParser(const char *type, const char *fileName);

    Parser() {}
    virtual ~Parser() {}

    virtual int parse(int bufferSize, MDCoord<nDim> *coord, Datum_t *datum) = 0;

    MDCoord<nDim> dim() { return dim_; }
    size_t size() { return nElem; }

 protected:
    i64 nElem;
    MDCoord<nDim> dim_;
};

template<> Parser<2> *Parser<2>::createParser(const char *, const char *);

#endif
