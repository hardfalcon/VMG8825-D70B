#include "lib_bufferpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INSERT_PATTERN_ERRORS    0 /* in % */
#define INSERT_MAGIC_ERRORS      0 /* in % */
#define INSERT_PUT_NULL_ERRORS   0
#define BUFFER_SIZE             25
#define TEST_ITERATIONS       1000

#define MAX_BUFFERS          10000

#define printstat(s) ({\
   printf("%8ld >> bufferPool size %5d = free %5d + out %5ld  buf[%5ld] | %s\r",\
          j, bufferPoolSize(pbp), bufferPoolAvail(pbp), calcBuffers, idx, s);\
})

typedef struct {
   unsigned char buf[BUFFER_SIZE+4];
} tBuffer;

#ifndef VXWORKS
int main ()
#else
int bufferTest()
#endif
{
   BUFFERPOOL    *pbp;
   void           *buf[MAX_BUFFERS] = {};
   long           i, idx=0, cur=0, action, byte;
   long           calcBuffers = 0;
   long           j;
   tBuffer        *pb;
   long           insertedPatternErrors = 0;
   long           insertedMagicErrors   = 0;
   long           insertedPutNullErrors = 0;

   srand((unsigned int)time(NULL));
   pbp = bufferPoolInit(BUFFER_SIZE, 100, 50);
   printf("\n\n");
   for ( j=TEST_ITERATIONS-1; j >= 0 ; j-- )
   {
      action = (int)((rand()*100.0/RAND_MAX)-51); /* -55..+45 */
      /*action = 10;*/

      if(action >= 0) {
         for (i=0 ; (i < action) && ((cur+i) < MAX_BUFFERS); i++ )
         {
            idx = cur + i;
            if (buf[idx] != NULL) {
               printf("\nERROR idx for get pointing already to sth. not NULL !!!\n");
            }
            buf[idx] = bufferPoolGet(pbp);
            calcBuffers++;
            if (INSERT_PATTERN_ERRORS > (int)(rand()*100.0/RAND_MAX)) {
               if (buf[idx] != NULL) {
                  pb =  buf[idx];
                  byte = (int)(rand()*2.0/RAND_MAX);
                  if (byte<0 || byte > 2) printf("\n\nERROR\n\n");
                  pb->buf[BUFFER_SIZE+byte] = 0xFF;
                  /*bufferPoolDump("patternError", buf[idx]);*/
                  insertedPatternErrors++;
               }
            }
            if (INSERT_MAGIC_ERRORS > (int)(rand()*100.0/RAND_MAX)) {
               if (buf[idx] != NULL) {
                  pb =  buf[idx];
                  pb->buf[BUFFER_SIZE+1] = 0xFF;
                  bufferPoolDump("magicError", buf[idx]);
                  insertedMagicErrors++;
               }
            }
            /*bufferPoolDump("test", buf[idx]);*/
            printstat("get");
         }
         cur += (action);
      }
      else/*------------------------------------------------------------- PUT */
      {
         action *= -1;
         for ( i=0; (i < action) && ((cur-i) > 0); i++ )
         {
            idx = cur - i;
         #if (INSERT_PUT_NULL_ERRORS == 0)
            if (buf[idx] != NULL) {
         #else
            if (buf[idx] == NULL) insertedPutNullErrors++;
         #endif
               if (bufferPoolPut(buf[idx])==0) {
                  calcBuffers--;
                  printstat("put");
               }
         #if (INSERT_PUT_NULL_ERRORS == 0)
            }
         #endif
            buf[idx] = NULL;
         }
         cur = idx;
         if (cur < 0) printf("ERROR cur < 0!!\n");
      }
   }/*------------------------------------------------------------ END OF PUT */

   printf("\n");
   /* give back the rest... */
   for (j=0; j<MAX_BUFFERS ; j++)
   {
      idx = j;
      if (buf[j] != NULL) {
         if (bufferPoolPut(buf[j])==0) {
            calcBuffers--;
            printstat("clean");
         }
         buf[j] = NULL;
      }
   }
   printstat("clean");


   printf("\n\nTEST STATISTICS\n");
   printf("insPatternErrors %5ld\ninsMagicErrors   %5ld\ninsPutNullErrors %5ld\nerrLost Buffers  %5ld",
           insertedPatternErrors, insertedMagicErrors, insertedPutNullErrors, calcBuffers);
   if (calcBuffers != 0) printf(" NOK");
   printBufferPoolErrors();
   printf("\n\n");

   bufferPoolFree(pbp);
  return 0;
}
