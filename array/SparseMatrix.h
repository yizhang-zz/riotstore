#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include <ostream>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include "cholmod.h"
#include "MDCoord.h"

class SparseMatrix
{
 public:
    typedef MDArrayElement<2> Element;
    typedef MDCoord<2> Coord;
    SparseMatrix(Element *elements, int num, const Coord &begin, const Coord
		 &end, bool inColMajor);
    SparseMatrix(cholmod_sparse *);
    SparseMatrix(int nrow, int ncol);
    SparseMatrix(const SparseMatrix &other);
    ~SparseMatrix();
    SparseMatrix operator+ (const SparseMatrix &other) const;
    SparseMatrix operator* (const SparseMatrix &other) const;
    SparseMatrix &operator= (const SparseMatrix &other);
    SparseMatrix &operator+= (const SparseMatrix &other);
    double operator() (int i, int j) const;
    void swap(SparseMatrix &other);
    size_t size() const { return cholmod_nnz(sp, chmCommon); }
    cholmod_sparse *storage() const { return sp; }
    void print() const;
    friend std::ostream & operator<< (std::ostream &, const SparseMatrix &);

    static cholmod_common *chmCommon;
    static cholmod_common *initCholmodCommon();

    class Iterator:public boost::iterator_facade<
	Iterator,
	Element,
	boost::forward_traversal_tag,
	Element
	>
	    {
	    public:
	    Iterator(cholmod_sparse *sp_, int i_=0, int j_=0): sp(sp_), i(i_), j(j_)
		{
		    // col i may be empty, in which case p[i+1]==p[i]
		    int *p = (int*) sp->p;
		    int ncol = (int) sp->ncol;
		    while (i < ncol && p[i+1] == p[i])
			++i;
		    if (p[i] > j)
			j = p[i];
		}

	    private:
		friend class boost::iterator_core_access;
		cholmod_sparse *sp;
		int i; // index in sp->p, column number
		int j; // index in sp->i and sp->x, sp->i[j] is row number

		void increment()
		{
		    int *p = (int*) sp->p; // col ptr
		    ++j;
		    while (i < (int) sp->ncol && j == p[i+1])
			++i;
		}

		bool equal(const Iterator &other) const
		{
		    return sp==other.sp && i==other.i && j==other.j;
		}

		Element dereference() const
		{
		    int *r = (int*) sp->i; // row index
		    double *x = (double*) sp->x; // values
		    Element e;
		    e.coord = Coord(r[j], i);
		    e.datum = x[j];
		    return e;
		}
	    };

    Iterator begin() const
    {
        return Iterator(sp);
    }

    Iterator end() const
    {
        return Iterator(sp, sp->ncol, cholmod_nnz(sp, chmCommon));
    }
 private:
    cholmod_sparse *sp;
};

std::ostream & operator<< (std::ostream &, const SparseMatrix &);

#endif
