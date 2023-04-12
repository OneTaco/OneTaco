#include "ObjectManager.h"
// You can add other header files if needed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
// tracks the next reference (ID) to use, we start at 1 so we can use 0 as the NULL reference
static Ref nextRef = 1;

// A Memblock holds the relevent information associated with an allocated block of memory by our memory manager
typedef struct MEMBLOCK MemBlock;

static void compact();
// information needed to track our objects in memory
struct MEMBLOCK
{
  int numBytes;    // how big is this object?
  int startAddr;   // where the object starts
  Ref ref;         // the reference used to identify the object
  int count;       // the number of references to this object
  MemBlock *next;  // pointer to next block.  Blocks stored in a linked list.
};


// The blocks are stored in a linked list where the start of the list is pointed to by memBlockStart.
static MemBlock *memBlockStart; // start of linked list of blocks allocated
static MemBlock *memBlockEnd;   // end of linked list of blocks allocated
static int numBlocks;            // number of blocks allocated

// our buffers.  This is where we allocate memory from.  One of these is always the current buffer.  The other is used for swapping
//during compaction stage.

static unsigned char buffer1[MEMORY_SIZE];
static unsigned char buffer2[MEMORY_SIZE];

// points to the current buffer containing our data
static unsigned char *currBuffer = buffer1;

// points to the location of the next available memory location
static int freeIndex = 0;


// performs required setup
void initPool()
{
  //write your code here
  
  numBlocks = 0;
  freeIndex = 0;
  memBlockStart = NULL;
  memBlockEnd = NULL;
  nextRef = 1;
}

// performs required clean up
void destroyPool()
{
  //we want to delete all nodes from the linked list.
  //write your code here
  MemBlock *head = memBlockStart;
  MemBlock *curr = NULL;
  #ifndef NDEBUG
    assert( head != NULL );

  #endif
  while (head){
    curr = head->next;
    free(head);
    head = curr;
  }
  
  memBlockEnd = NULL;
  numBlocks = 0;
  numBlocks = 0;
  freeIndex = 0;
  nextRef = 1;
  memBlockStart = NULL;
}

// Adds the given object into our buffer. It will fire the garbage collector as required.
// We always assume that an insert always creates a new object...
// On success it returns the reference number for the block allocated.
// On failure it returns NULL_REF (0)
Ref insertObject( const int size )
{
    //write your code here
    Ref ref = NULL_REF;
    int left_size = MEMORY_SIZE - freeIndex;
    #ifndef NDEBUG
    assert( size > 0 );
    //assert( left_size > 0 );
    
    #endif
    
    //printf("%d\n",left_size);
    if ( size >= left_size ){
        compact();
        left_size = MEMORY_SIZE - freeIndex;
    }


    if (size >= left_size){
        return ref;
    }

    MemBlock *memBlock = (MemBlock*)malloc(sizeof(MemBlock));

    if (memBlock){
      ref = nextRef++;
      #ifndef NDEBUG
      assert( ref < nextRef );
      assert( ref >= 0 );
      assert( freeIndex >= 0 );
      assert( freeIndex < MEMORY_SIZE );
      #endif
      memBlock->numBytes = size;
      memBlock->startAddr = freeIndex;
      memBlock->ref = ref;
      memBlock->next = NULL;
      
      memBlock->count = 1;
      
      
      if (numBlocks == 0){
        memBlockStart = memBlock;
      }
      numBlocks+=1;
      
      if (memBlockEnd){
        memBlockEnd->next = memBlock;
      }
      memBlockEnd = memBlock;
      
      memset(&currBuffer[freeIndex], 0, size);
      freeIndex += size;
      

    }

   
    return ref;
}

// returns a pointer to the object being requested
void *retrieveObject( const Ref ref )
{
    //write your code here
    MemBlock *memBlock = NULL;
    MemBlock *head = memBlockStart; 
    #ifndef NDEBUG
    
    assert( ref < nextRef );
    assert( head != NULL );
    assert( ref >= 0 );
    #endif


    
    while (head) {
        if (head->ref == ref) {
            memBlock = head;
            break;
        } 
        head=head->next;

    }


    if (memBlock == NULL){
        return NULL;
    }

    return &currBuffer[memBlock->startAddr];
}

// update our index to indicate that we have another reference to the given object
void addReference( const Ref ref )
{  
  //write your code here
    MemBlock *memBlock = NULL;
    MemBlock *head = memBlockStart; 
    #ifndef NDEBUG
    assert( head != NULL );
    assert( ref < nextRef );
    assert( ref >= 0 );
    #endif

    
    while (head) {
        if (head->ref == ref) {
            memBlock = head;
            break;
        } 
        head=head->next;

    }


    if (memBlock) {
        memBlock->count++;
    }
}

// update our index to indicate that a reference is gone
void dropReference( const Ref ref )
{  
  //write your code here
    MemBlock *memBlock = NULL;
    MemBlock *head = memBlockStart; 

    #ifndef NDEBUG
    
    assert( head != NULL );
    assert( ref < nextRef );
    assert( ref >= 0 );
    #endif
    
    while (head) {
        if (head->ref == ref) {
          memBlock = head;
          break;
        } 
        head = head->next;
        
    }

    if (memBlock == NULL){
        return;
    }

    if (memBlock->count == 1) {

        if (memBlock != memBlockStart){
            MemBlock *prev = memBlockStart;
            MemBlock *curr = prev->next;
            while (memBlock != curr){
                prev = curr;
                curr = curr->next;
            }

            if (memBlockEnd == curr){
                memBlockEnd = prev;
            }

            prev->next = memBlock->next;
            
            
        } else {
            memBlockStart = memBlockStart->next;
     
        }
        free(memBlock);

        if (--numBlocks == 1){
            memBlockEnd = memBlockStart;
        }
      
    }
    memBlock->count -= 1;
  
}

// performs our garbage collection
static void compact()
{
  //write your code here
    #ifndef NDEBUG
    printf("compact\n");
    #endif
    unsigned char *buffer = NULL;
    if(currBuffer == buffer1) {
        buffer = buffer2 ;
        freeIndex = 0;
    }else{
        buffer =  buffer1;
        freeIndex = 0;
    }
  
    
    MemBlock *head = memBlockStart;
    #ifndef NDEBUG

    assert( buffer != NULL );
    assert( head != NULL );
    assert( freeIndex == 0 );

    #endif
    
    while (head != NULL){

        memcpy(&buffer[freeIndex], &currBuffer[head->startAddr], head->numBytes);
        head->startAddr = freeIndex;
        freeIndex += head->numBytes;
        head = head->next;
    }

  
    currBuffer = buffer;
}

void dumpPool()
{
    //write your code here
    MemBlock * memBlock = memBlockStart;
    #ifndef NDEBUG

    assert( memBlock != NULL );

    #endif
    while (memBlock){
        printf( "reference id = %lu, start address = %d, size = %d\n", \
            memBlock->ref, memBlock->startAddr, memBlock->numBytes);
        memBlock = memBlock->next;
    }
 
}

//you may add additional function if needed