#ifndef DMA_BLOCK
#define DMA_BLOCK

#include "Block.h"
#include "../common/ArrayInternalIterator.h"
#include <vector>

class DirectlyMappedArray;
/**
 * A DenseArrayBlock does not need a header for the block.  The page
 * simply stores a data array.  It stores a continuguous subrange for a
 * DirectlyMappedArray.
 */
class DMABlock : public Block<Key_t, Datum_t> 
{
protected:
    // header is placed at the end of the block
    struct Header {
        size_t nnz;
    } *header;

    /// A data array that corresponds to the portion of the
    /// DirectlyMappedArray in the index range [beginsAt, endsBy).
    Datum_t *data;

public:
    const static Datum_t DefaultValue;
    const static size_t CAPACITY;

    /// Initializes the block by reading from a page image.  The index
    /// range and the default data value will be passed in---the caller,
    /// with the knowledge of the overall DirectlyMappedArray, should
    /// know the exact index range that this page is responsible for.
    /// The default data value will also be supplied by the caller.
    DMABlock(PageHandle ph, Key_t lower, Key_t upper);
    ~DMABlock();

    // Clears all values to zero
    void init();

    /// assume key is within range
    Datum_t get(Key_t key) const;
    void batchGet(i64 getCount, Entry *gets);
    void batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v);

    /// assume key is within range
    int put(Key_t key, Datum_t datum);
    int batchPut(i64 putCount, const Entry *puts);
    int batchPut(std::vector<Entry>::const_iterator &begin,
            std::vector<Entry>::const_iterator end);

    /// assume beginsAt and endsBy are within upperBound and lowerBound
    ArrayInternalIterator* getIterator(Key_t beginsAt, Key_t endsBy);
    ArrayInternalIterator* getIterator();
};

#endif
