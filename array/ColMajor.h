#ifndef COL_MAJOR_H
#define COL_MAJOR_H

#include "Linearization.h"

class ColMajor : public Linearization

{
//   private:
   public:
      /**
       * Storage structure to store state on matrix being linearized
       * dimension.nDim corresponds to number of dimensions in matrix
       * each coordinate in dimension.coords corresponds to size of the matrix
       * along that dimension
       */
      u8 nDim;
      i64 *dims;
      // MDCoord *dimension;

public:

      ColMajor(u8 nDim, const i64 *coords);
      ColMajor(const MDCoord &coord);
      ~ColMajor();
    /**
     * Linearizes the given coord.
     *
     * Given \f$ N_1\times N_2\times \cdots \times N_m \f$ matrix,
     * linearization of given tuple \f$ \( n_1, n_2, \ldots , n_m) \f$ is given
     * by
     * \f$ f(n_1, n_2, \ldots , n_m) = \sum_{i=1}^{m}(\prod_{k=i+1}^{m}N_k\)n_i\
     * 
     * \param coord The coord to be linearized.
     * \return Result of linearization.
     */
    Key_t linearize(const MDCoord &coord);

    /**
     * Unlinearizes the given 1-D coord.
     *
     * \param key Key in 1-D to be unlinearized.
     * \return The result f unlinearization.
     */
    MDCoord unlinearize(Key_t key);

    /*
     * Incrementally linearizes the coord specified as the sum of the
     * remembered last state and the give difference. Provided as a
     * potential optimization for quick linearization if the change in
     * coordinates is incremental.
     *
     * \param diff The difference in coordinate based on last state.
     * \return The result of linearization.
     */
    // virtual Key_t linearizeIncremental(MDCoord &diff) = 0;

    /**
     * Incrementally calculate \f$y\f$ such that \f$ f(y)=f(from)+diff
     * \f$ from the given anchor and diff. Provided as a potential
     * optimization for quick calculation of the next or pervious
     * coordinate when iterating through an array.
     *
     * \param from The anchor coordinate in n-D space.
     * \param diff The difference in linearized space.
     * \return The new coordinate in n-D space.
     */
    MDCoord move(const MDCoord &from, KeyDiff_t diff);

    /**
     * Clone method to simulate virtual copy constructor. IMPORTANT: a
     * subclass A shoule return type A* instead of Linearization*.
     *
     * \return A pointer to a clone of subclass type.
     */
    ColMajor* clone();
    

};


#endif
