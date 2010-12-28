#include "LinearStorage.h"
#include "directly_mapped/DirectlyMappedArray.h"
#include "btree/Btree.h"

LinearStorage *LinearStorage::fromFile(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");
	fseek(file, STORAGE_METADATA_PAGES*PAGE_SIZE, SEEK_SET);
    int type;
    fread(&type, sizeof(int), 1, file);
	fclose(file);
	switch(type) {
	case DMA:
		return new DirectlyMappedArray(fileName);
	case BTREE:
		return new Btree::BTree(fileName);
	default:
		Error("Unknown linear storage format");
		return NULL;
	}
}
