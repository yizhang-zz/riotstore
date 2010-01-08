DIRS = common lower directly_mapped btree

all :
	for dir in $(DIRS); do \
	echo "make all in $$dir"; \
	(cd $$dir; $(MAKE) ); done
