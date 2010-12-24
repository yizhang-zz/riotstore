#include "Parser.h"
#include "MMParser.h"

template<int nDim>
Parser<nDim> *Parser<nDim>::createParser(const char *type, const char *fileName)
{
    return NULL;
}

template<>
Parser<2> *Parser<2>::createParser(const char *type, const char *fileName)
{
    if (strcmp(type, "MM") == 0) {
        return new MMParser(fileName);
    }
    else {
        return NULL;
    }
}

template class Parser<1>;
template class Parser<2>;
