#ifndef DENSE_ARRAY_BLOCK
#define DENSE_ARRAY_BLOCK

#include "../common/block.h"
#include "../common/ArrayInternalIterator.h"

/**
 * A DenseArrayBlock does not need a header for the block.  The page
 * simply stores a data array.  It stores a continuguous subrange for a
 * DirectlyMappedArray.
 */
class DenseArrayBlock : public Block<Key_t, Datum_t> 
{
   protected:
      /// A data array that corresponds to the portion of the
      /// DirectlyMappedArray in the index range [beginsAt, endsBy).
      Datum_t *data;

   public:
      /// Initializes the block by reading from a page image.  The index
      /// range and the default data value will be passed in---the caller,
      /// with the knowledge of the overall DirectlyMappedArray, should
      /// know the exact index range that this page is responsible for.
      /// The default data value will also be supplied by the caller.
      DenseArrayBlock(PageHandle *ph, Key_t lower, Key_t upper);
      ~DenseArrayBlock();

      // Datum_t getDefaultValue() const;
      bool isFull() const;

      /// assume key is within range
      Datum_t get(Key_t key) const;

      /// assume key is within range
      void put(Key_t key, Datum_t datum);

      /// assume beginsAt and endsBy are within upperBound and lowerBound
      ArrayInternalIterator* getIterator(Key_t beginsAt, Key_t endsBy);
      ArrayInternalIterator* getIterator();
};

#endif
