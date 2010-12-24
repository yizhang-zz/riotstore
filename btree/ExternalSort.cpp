/* -*- mode: c++; c-basic-offset:2 -*- */
#include "ExternalSort.h"

using namespace std;

class Compare{
public:
  bool operator()(IndexKeyPair a, IndexKeyPair b){
    return a.second > b.second;
  }
};

void ExternalSort::genTempFileName(char *s, unsigned number)
{
  sprintf(s,".es_temp%08x", number);
}

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
  
ExternalSort::~ExternalSort()
{
  if (streamsInitialized)
    delete[] ifile;
  char name[32];
  for (unsigned i = 0; i < m_chunkCount; ++i) {
    genTempFileName(name, i);
    remove(name);
  }
}

// this creates chunk-wise sorted files
void ExternalSort::streamToChunk(Entry *chunk, int length){
  std::sort(chunk, chunk+length);
  //qsort(chunk, length, sizeof(KVPair_t), compareKVPair);

  char name[32];
  genTempFileName(name, m_chunkCount);
  //cout << "[ExternalSort] Creating a sorted chunk file: " << name << "...\n";
  ofstream chunkSortedFile(name, ofstream::binary);
  for (int i=0; i<length; i++){
    chunkSortedFile.write((char*)(chunk+i),sizeof(Key_t));
    chunkSortedFile.write(((char*)(chunk+i))+offsetof(Entry, datum), sizeof(Datum_t));
  }
  chunkSortedFile.close();
  m_chunkCount++;
}

/*
 * Initial chunk-wise sort using quick sort
 */
/*
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
*/

/*
 * N-way Merge Phase, and out to a record array to feed Btree.load
 */
int ExternalSort::mergeSortToStream(Entry *rec, uint64_t maxRecs){
  static Compare compare;
  // Initial stream processing (Heap set-up)
  // Done only once
  if (!streamsInitialized){
    //cout << "[ExternalSort] Initializing the sorted input stream..." << endl;
    streamsInitialized = true;
    ifile = new ifstream[m_chunkCount];
    Key_t key;
    char filename[32];
    for (unsigned i=0; i<m_chunkCount; i++){
      genTempFileName(filename, i);
      ifile[i].open(filename, ios::binary);
      if (!ifile[i].eof()) {
        ifile[i].read((char*)&key, sizeof(key));
        if (!ifile[i].eof()) {
          v.push_back(make_pair(i, key));
        }
      }
    }
    //cout << "[ExternalSort] Stream initialization done" << endl;
    make_heap(v.begin(), v.end(), compare);
  }

  // merge-sort process
  unsigned j = 0;
  for (j=0; j<maxRecs && !v.empty(); j++){
    IndexKeyPair smallestPair = v.front();
    pop_heap(v.begin(), v.end(), compare);
    v.pop_back();
    Key_t smallestKey = smallestPair.second;
    unsigned chunkIndex = smallestPair.first;
    ifstream &file = ifile[chunkIndex];
    Datum_t value;
    file.read((char*)&value, sizeof(value));
    assert(!file.eof());
    rec[j].key = smallestKey;
    rec[j].datum = value;
    m_sortedCount++;
    //cout << "Top of the Heap = ("<<smallestKey<<","<<value<<") from chunk "<<chunkIndex << endl;

    // read a new entry from the same chunk
    Key_t key;
    if (!file.eof()) {
      file.read((char*)&key, sizeof(key));
      if (!file.eof()) {
        v.push_back(make_pair(chunkIndex, key));
        push_heap(v.begin(), v.end(), compare);
      }
    }
  }
  return j;
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
