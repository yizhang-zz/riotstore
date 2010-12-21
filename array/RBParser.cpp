#include "RBParser.h"
/* Author: Kevin Jang
 * 11/17/2010
 *
 */

template<int nDim>
RBParser<nDim>::RBParser(int cpl, int rpl)
{
  headerRead = false;
  numRows = 0;
  numCols = 0;
  numValues = 0;
  rowIdx = 0;
  colIdx = 0;
  valIdx = 0;
  colptr = 0;
  colRead = 0;
  colPerLine = cpl;
  rowPerLine = rpl;
}

template<int nDim>
int RBParser<nDim>::parse(ifstream &in, int bufferSize, MDCoord<nDim> *coord, Datum_t *datum) {
  
    if (in.eof() || in.peek() == -1) return 0;
    /*
     * First, parse the header to get the matrix dimensions and the
     * number of non-zero elements.
     */
    if (!headerRead)
    {
      headerRead = true;
      col = 1;
      in.ignore(INT_MAX, '\n');
      in.ignore(INT_MAX, '\n');
      in.ignore(15);
      in >> numRows;
      in >> numCols;
      in >> numValues;
      in.ignore(INT_MAX, '\n');
      in.ignore(INT_MAX, '\n');

      int dummy;
      in >> dummy; // to pass the first column pointer, which is always '1'
      in >> colptr;
      colIdx = in.tellg();

      int skipcols = numCols / colPerLine + (numCols % colPerLine > 0 ? 1:0);
      int skiprows = numValues / rowPerLine + (numValues % rowPerLine > 0 ? 1:0);
      for (int i=0; i<skipcols; i++)	in.ignore(INT_MAX, '\n');
      rowIdx = in.tellg();
      for (int i=0; i<skiprows; i++)	in.ignore(INT_MAX, '\n');
      valIdx = in.tellg();
    }
    
    int count = 0;
    while(count < bufferSize){
      if (in.peek() == -1) return count;
      int rowind = -1;
      double value = -1.0;
      colRead++;
      if (colRead >= colptr) {
	if (colptr >= numValues) return count;
	in.seekg(colIdx);
	if (in.peek() == -1) return count;
	int nextColptr;
	do {
	  in >> nextColptr;
	  if (colptr == nextColptr) col++;
	} while( colptr == nextColptr);
	//cout << "colptr = " << colptr << endl;
	colIdx = in.tellg();
	colptr = nextColptr;
	col++;
      }

      in.seekg(rowIdx);
      in >> rowind;
      rowIdx = in.tellg();

      in.seekg(valIdx);
      in >> value;
      valIdx = in.tellg();
      
      MDCoord<nDim> temp(COORD(rowind), COORD(col));
      coord[count] = temp;
      datum[count] = value;
      //cout << "colread = " << colRead << ": " << rowind << ", " << col << ", " << value << endl;
      count++;
    }
    return count;
}
 
template class RBParser<1>;
template class RBParser<2>;
template class RBParser<3>;
