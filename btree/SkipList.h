#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <stdlib.h>
#include <apr_pools.h>
#include <string>
//#include <fstream>
//#include <iostream>
#include "../common/Iterator.h"

template<class Key, class Value>
class SkipList
{
public:
    class Entry
    {
    public:
        int level;
        Key key;
        Value value;
        int dist;
        Entry* right;
        Entry* below;
        Entry* left;

        Entry(int l, Key k, Value v)
        {
            level = l;
            key = k;
            value = v;
            dist = 0;
            left = right = below = NULL;
        }

        void *operator new(size_t size, apr_pool_t *pool)
        {
            return apr_palloc(pool, size);
        }
    };
    class Iterator: public ::Iterator<Key, Value>
    {
    public:
        Iterator(SkipList<Key, Value> *l)
        {
            list = l;
        }
        bool moveNext()
        {
            if (entry->right) {
                entry = entry->right;
                return true;
            }
            return false;
        }
        bool movePrev()
        {
            if (entry->left) {
                entry = entry->left;
                return true;
            }
            return false;
        }
        void get(Key &k, Value &v)
        {
            k = entry->key;
            v = entry->value;
        }
        void put(const Value &v)
        {
            entry->value = v;
        }
        void reset()
        {
            Entry *path[MAX_LEVEL];
            int dists[MAX_LEVEL];
            list->_locate(begin-1, path, dists);
            entry = path[0];       
        }
        bool setRange(const Key &b, const Key &e)
        {
            throw("not implemented");
            return false;
        }
        bool setIndexRange(Key_t begin, Key_t end)
        {
            if (begin >= end || begin <0 || end > list->getSize())
                return false;
            this->begin = begin;
            this->end = end;
            reset();
            return true;
        }
            
    private:
        int begin;
        int end;
        SkipList<Key, Value> *list;
        //int current;
        Entry *entry;
    };
    friend class SkipList<Key, Value>::Iterator;

    enum {
        OK,
        NotFound,
        Overwrite
    };

    SkipList(apr_pool_t *p, double prob=0.5) : probability(prob)
    {
        apr_pool_create(&this->pool, p);
        Value v; // dummy value
        head = new(this->pool) Entry(0, -1000, v);
        size = 0;
    }

    ~SkipList()
    {
        /*
        Entry* start = head;
        for (int i=head->level; i>=0; i--) {
            Entry* p = start, *q;
            start = start->below;
            while (p) {
                q = p->right;
                delete p;
                p = q;
            }
        }
        */
        apr_pool_destroy(pool);
    }

    int locate(int index, Key& key, Value& value) const
    {
        if (index >= size || index < 0)
            return NotFound;
        Entry* path[MAX_LEVEL];
        int dists[MAX_LEVEL];
        _locate(index, path, dists);
        key = path[0]->key;
        value = path[0]->value;
        return OK;
    }

    int search(const Key key, Value& value, int& index) const
    {
        Entry* path[MAX_LEVEL];
        int dists[MAX_LEVEL];
        _search(key, path, dists);
        value = path[0]->value;
        index = -1;
        for (int i=0; i<=head->level; i++)
            index += dists[i];
        if (path[0]->key == key) {
            return OK;
        }
        else {
            index++;
            return NotFound;
        }   
    }

    int insert(const Key key, const Value &value)
    {
        // turning points where the down pointer is followed
        Entry* path[MAX_LEVEL];
        // distance between the i-th turning point and the (i+1)-th
        int dists[MAX_LEVEL];
        _search(key, path, dists);
        
        if (path[0]->key == key) {
            // overwrite
            path[0]->value = value;
            return Overwrite;
        }

        int height = getNewLevel();
        // raise the head column
        if (head->level < height) {
            for (int j=head->level+1; j <= height; j++) {
                Entry* r = new(pool) Entry(j, head->key, head->value);
                r->left = r->right = NULL;
                r->below = head;
                head = r;
                path[j] = r;
                dists[j] = 0;
            }
        }

        Entry* below =  NULL;
        int sum = 1;
        for (int j=0; j<=height; j++) {
            Entry* q = new(pool) Entry(j, key, value);
            q->right = path[j]->right;
            if (q->right) q->right->left = q;
            q->below = below;
            q->dist = path[j]->dist+1-sum;
            path[j]->right = q;
            q->left = path[j];
            below = q;
            path[j]->dist = sum;
            sum += dists[j];
        }
        for (int j=height+1; j<=head->level; j++) {
            path[j]->dist ++;
        }

        size++;
        return OK;
    }
    
    int removeAt(const int index)
    {
        if (index <0 || index >= size)
            return NotFound;
        Entry *path[MAX_LEVEL];
        int dists[MAX_LEVEL];
        _locate(index, path, dists);
        bool sameCol = true;
        for (int j=0; j<=head->level; j++) {
            if (sameCol) {
                path[j]->left->right = path[j]->right;
                if (path[j]->right) path[j]->right->left = path[j]->left;
                path[j]->left->dist += path[j]->dist-1;
                //delete path[j];
            }
            else {
                path[j]->dist--;
            }
            sameCol = sameCol && (dists[j]==0);
        }
        size--;
        return OK;
    }
    
    int remove(const Key key)
    {
        // turning points where the down pointer is followed
        Entry* path[MAX_LEVEL];
        // distance between the i-th turning point and the (i+1)-th
        int dists[MAX_LEVEL];

        _search(key, path, dists);

        if (path[0]->key != key) {
            return NotFound;
        }

        for (int j=0; j<=head->level; j++) {
            if (path[j]->key == key) {
                path[j]->left->right = path[j]->right;
                if (path[j]->right) path[j]->right->left = path[j]->left;
                path[j]->left->dist += path[j]->dist-1;
                // delete path[j];
            }
            else {
                path[j]->dist--;
            }
        }
        size--;
        return OK;
    }

    /**
     * Removes all entries starting from the index-th.
     */
    void truncate(int index)
    {
        Entry *path[MAX_LEVEL];
        int dists[MAX_LEVEL];
        _locate(index, path, dists);

        bool sameCol = true;
        for (int j=0; j<=head->level; j++) {
            if (sameCol) {
                path[j]->left->right = NULL;
                path[j]->left->dist = 0;
                //deleteChain(path[j]);
            }
            else {
                path[j]->dist = 0;
                //deleteChain(path[j]->right);
                path[j]->right = NULL;
            }
            sameCol = sameCol && (dists[j]==0);
        }
        size = index;
    }

    
    std::string toString()
    {
        std::string s;
        char buf[32];
        Entry* start = head;
        for (int i=head->level; i>=0; i--) {
            Entry *e = start;

            while (e) {
                sprintf(buf, "%ud", e->key);
                s += buf;
                for (int k=0; k<e->dist; k++)
                    s+= "\t";
                e = e->right;
            }
            s += "\n";
            start = start->below;
        }
        return s;
    }
    
    int getSize() {return size;}

    Iterator* getIterator(int begin, int end)
    {
        Iterator *it = new Iterator(this);
        it->setIndexRange(begin, end);
        return it;
    }

    Iterator* getIterator()
    {
        Iterator *it = new Iterator(this);
        it->setIndexRange(0, size);
        return it;
    }
    
private:
    const static int MAX_LEVEL = 10;
    apr_pool_t *pool;
    Entry* head;
    double probability;
    int size;
    int getNewLevel()
    {
        int l = 0;
        while (l < MAX_LEVEL-1 && ((double)rand())/RAND_MAX < probability)
            l++;
        return l;
    }

    void deleteChain(Entry *p)
    {
        while (p) {
            Entry *q = p->right;
            delete p;
            p = q;
        }
    }

    void _search(Key key, Entry **points, int *dists) const
    {
        Entry* p = head;
        for(int i=head->level; i>=0; i--) {
            int temp = 0;
            while (p->right && p->right->key <= key) {
                temp += p->dist;
                p = p->right;
            }
            dists[i] = temp;
            points[i] = p;
            if (p->below)
                p = p->below;
        }
    }

    void _locate(int index, Entry **points, int *dists) const
    {
        int k = 0;
        index++; // index is 0-based but internal counting is 1-based
        Entry* p = head;
        for(int i=head->level; i>=0; i--) {
            int temp = 0;
            while (p->right && k+p->dist <= index) {
                k += p->dist;
                temp += p->dist;
                p = p->right;
            }
            dists[i] = temp;
            points[i] = p;
            if (p->below)
                p = p->below;
        }
    }
public:
};
#endif
