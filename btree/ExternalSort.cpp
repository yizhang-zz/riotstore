/* -*- mode: c++; c-basic-offset:2; -*- */
#include "ExternalSort.h"

class Compare{
public:
  bool operator()(IndexKeyPair a, IndexKeyPair b){
    return a.second > b.second;
  }
};

ExternalSort::ExternalSort(const char* fileName, const uint64_t bufferSize)
  :done(false),m_fileName(fileName),m_bufferSize(bufferSize){
  m_recordCount = 0;
  m_sortedCount = 0;
  m_chunkCount = 0;
  streamsInitialized = false;
}

ExternalSort::ExternalSort(const uint64_t bufferSize)
  :done(false),m_bufferSize(bufferSize){
  m_recordCount = 0;
  m_sortedCount = 0;
  m_chunkCount = 0;
  streamsInitialized = false;
}
  

// this creates chunk-wise sorted files
void ExternalSort::streamToChunk(Key_t *key, Datum_t *datum, const int length){
  Entry *chunk;

  try{
    chunk = new Entry[length];
  }  catch (bad_alloc &e){
    cerr << e.what() << endl;
  }
  for (int i = 0; i<length; i++){
    chunk[i].key = key[i];
    chunk[i].pdatum = datum+i;
  }
  
  std::sort(chunk, chunk+length);
  //qsort(chunk, length, sizeof(KVPair_t), compareKVPair);

  char name[32];
  sprintf(name,"temp/temp%d.out", m_chunkCount);
  cout << "[ExternalSort] Creating a sorted chunk file: " << name << "...\n";
  ofstream chunkSortedFile(name, ofstream::binary);
  char *buf = new char[keySize];
  int count=0;
  for (int i=0; i<length; i++){
    sprintf(buf,"%d",(int) chunk[i].key);
    chunkSortedFile.write(buf,keySize);
    sprintf(buf,"%f",*(chunk[i].pdatum));
    chunkSortedFile.write(buf,keySize);
    count++;
    //    cout << i << ". writing (key, datum) = " << (int) chunk[i].key << ", " << *(chunk[i].datum) << endl;
  }
  cout << "... done: " << count << " records" << endl;
  chunkSortedFile.close();
  m_chunkCount++;
  delete[] buf;
}

/*
 * Initial chunk-wise sort using quick sort
 */
void ExternalSort::fileToChunk(){
  gettimeofday(&tim, NULL);
  begin_run = tim.tv_sec + tim.tv_usec/1000000.0;
  //begin_run = time(NULL);

  ifstream in_file(m_fileName);

  in_file.seekg(0, ios::end);
  int end = in_file.tellg();
  in_file.seekg(0, ios::beg);
  char * buf = new char[keySize];

  while (!in_file.eof()){
    if (in_file.tellg() >= end) break;
    char name[32];
    sprintf(name,"temp/temp%d.out", m_chunkCount);
    ofstream chunkSortedFile(name, ofstream::binary);
    Entry *chunk;
    try{
      chunk = new Entry[m_bufferSize];
    }
    catch (bad_alloc &e){
      cerr << e.what() << endl;
    }    
    unsigned int index = 0;
    while (index < m_bufferSize){
      if (in_file.tellg() == -1) break;
      if (in_file.peek() == -1) break;
      m_recordCount++;
      in_file >> chunk[index].key;
      in_file.ignore(1);
      in_file >> chunk[index].datum;
      in_file.ignore(INT_MAX, '\n');
      index++;
    } 
    std::sort(chunk, chunk+index);     
    //qsort(chunk, index, sizeof(KVPair_t), compareKVPair);
    print("Finished sorting chunk#", m_chunkCount);
    
    for (unsigned int j=0; j<index; j++){
      sprintf(buf,"%d",(int) chunk[j].key);
      chunkSortedFile.write(buf,keySize);

      sprintf(buf,"%f",chunk[j].datum);
      chunkSortedFile.write(buf,keySize);
      //      sprintf(buf,"%c", '\n');
      //      chunkSortedFile.write(buf,1);
    }
    sprintf(buf,"%s", "done");
    chunkSortedFile.write(buf, keySize);
    chunkSortedFile.close();
    m_chunkCount++;
  }
  in_file.close();
  delete[] buf;
}



void printV(vector<IndexKeyPair> &v){
  for (unsigned int i=0; i<v.size(); i++){
    cout << "(" << v[i].first << ", " << v[i].second << ")";
  }
  cout << endl;
}

/*
 * N-way Merge Phase, and out to a record array to feed Btree.load
 */
int ExternalSort::mergeSortToStream(Entry *rec, uint64_t maxRecs){
  int count = 0;
  uint64_t key;
  char * buf = new char[keySize];

  // Initial stream processing (Heap set-up)
  // Done only once
  if (!streamsInitialized){
    cout << "[ExternalSort] Initializing the sorted input stream..." << endl;
    streamsInitialized = true;
    ifile = new ifstream[m_chunkCount];
    uint64_t key;
    for (uint64_t i=0; i<m_chunkCount; i++){
      char filename[32];
      sprintf(filename,"temp/temp%d.out",(int) i);
      ifile[i].exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
      ifile[i].open(filename, ios::binary);
      if (ifile[i].eof()) continue;
      try{
	ifile[i].read(buf,keySize);
      } catch(ifstream::failure e){
	cout << "Error while reading the file " << filename << endl;
	exit(0);
      }
      key = atoi(buf);
      assert(key > 0);
      cout << "(chunk index, key) = " << i << ", " << key << endl;
      v.push_back(make_pair(i, key));    
    }
    cout << "[ExternalSort] Stream initialization done" << endl;
    make_heap(v.begin(), v.end(), Compare());  
  }

  // merge-sort process
  for (unsigned int j=0; j<maxRecs; j++){
    if (isDoneSorting(ifile,m_chunkCount)){
      cout << "[ExternalSort] Finished merge-sort" << endl;
      break;
    }
    assert(m_chunkCount > 0);
    IndexKeyPair smallestPair = v.front();
    pop_heap(v.begin(), v.end(), Compare());
    v.pop_back();
    uint64_t smallestKey = smallestPair.second;
    uint64_t chunkIndex = smallestPair.first;
    //if (smallestKey != 0)
    cout << "Top of the Heap = (" << chunkIndex << ", " << smallestKey << ")" << endl;

    double value;
    try {
      ifile[chunkIndex].read(buf,keySize);
    } catch (ifstream::failure e){
      cout << "[ExternalSort] couldn't read ifile" << endl;
    }
    value = atof(buf);

    // detecting the end of binary file
    bool end = true;
    try{
      ifile[chunkIndex].read(buf,keySize);
      end = false;
    } catch (ifstream::failure e){
      cout << "[ExternalSort] ifile[" << chunkIndex << "] done processing" << endl;
    }
    if (!end) {
      key = atoi(buf);
      v.push_back(make_pair(chunkIndex, key));
      push_heap(v.begin(), v.end(), Compare());
    }

    rec[j].key = smallestKey;
    rec[j].datum = value;
    m_sortedCount++;
    count++;
  }
  delete[] buf;
  gettimeofday(&tim,NULL);
  end_run = tim.tv_sec + tim.tv_usec/1000000.0;
  return count;
}

bool ExternalSort::isDoneSorting(ifstream *ifile, unsigned int chunkcount){
  for (uint64_t i=0; i<chunkcount; i++){
    if (!ifile[i].eof()){
      return false;
    }
  }
  return true;
}
/*
int main(int argc, const char *argv[]){
  //getData();
  //return 0;
  assert(argc == 3);
  uint64_t bufferSize = atoi(argv[2]);
  ExternalSort es(argv[1], bufferSize);
  ofstream test1("test.out");
  es.sortInChunk();
  //es.mergeSortToFile();
  cout << "Entering n_way merge phase..." << endl;
  cout << "Total # of input  records = " << es.getRecordCount() << endl;

  uint64_t maxrec = bufferSize; 
  record *rec = new record[maxrec];
  int count = maxrec;
  while(count == maxrec){
    count = es.mergeSortToChunk(rec, maxrec);
    for (int i=0; i<count; i++){
      test1 << rec[i].key << " " << rec[i].value << endl;
    }
  }
  delete[] rec;
  test1.close(); 

  cout << "Total # of output records = " << es.getSortedCount() << endl;
  cout << "Process finished in " << es.getTimeTaken() << " seconds" << endl;
}

*/


/*
 * N-way Merge Phase, and out to a file
 *
void ExternalSort::mergeSortToFile(){
  ofstream sortedFile("sortedFile.out");

  if (m_chunkCount == 1) {
    m_sortedCount = m_recordCount;
    end_run = time(NULL); 
    return;
  }

  while (!isDoneSorting(ifile,m_chunkCount)){
    assert(m_chunkCount > 0);
    uint64_t smallestKey = INT_MAX;
    uint64_t chunkWithSmallestKey = INT_MAX;
    for (uint64_t i=0; i<m_chunkCount; i++){
      if (ifile[i].eof()) {
	continue;
      }
      uint64_t key;
      int pos = ifile[i].tellg();
      ifile[i] >> key;
      if (key < smallestKey){
	smallestKey = key;
	chunkWithSmallestKey = i;
      }
      ifile[i].seekg(pos);
    }
    // handle the last line if empty
    if (ifile[chunkWithSmallestKey].tellg()==-1) {
      continue; 
    }
    double value;
    ifile[chunkWithSmallestKey] >> smallestKey;
    ifile[chunkWithSmallestKey].ignore(1);
    ifile[chunkWithSmallestKey] >> value;
    ifile[chunkWithSmallestKey].ignore(INT_MAX, '\n');
    sortedFile << smallestKey << " " << value << endl;
    m_sortedCount++;
  }
  sortedFile.close();
  //end_run = time(NULL);
  gettimeofday(&tim, NULL);
  end_run = tim.tv_sec + tim.tv_usec/1000000.0;

}
*/
