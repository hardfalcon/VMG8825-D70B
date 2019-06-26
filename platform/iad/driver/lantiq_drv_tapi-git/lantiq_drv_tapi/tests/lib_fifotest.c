#include "lib_fifo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  TEST_ELEMENTS 30
#define  SHOW_STEPS    1

#if(SHOW_STEPS == 1)
#define printStep(s)({printf s;})
#else
#define printStep(s)({})
#endif

/*******************************************************************************
Description:
   Report a TestCase
Arguments:
   s        - string describing the test case
   r        - boolean result (0 failed, 1 passed)
Return:
   none
*******************************************************************************/
int      tc = 0;
int      ok_all = 1;

#define TC(s,r) ({\
   int i;\
   tc++;\
   printf("FifoTest %2d: %s ", tc, s);\
   for (i=0; i < (58-strlen(s)); i++) printf(".");\
   if (r)\
   {\
      printf("     OK\n");\
   }\
   else\
   {\
      printf(" FAILED\n"); ok_all = 0;\
   }\
})


/*******************************************************************************
Description:
   TBD
Arguments:
   none
Return:
   none
*******************************************************************************/
int main ()
{
   FIFO_ID         *pf = NULL;
   unsigned int     test[TEST_ELEMENTS];
   unsigned int    *ptest;
   unsigned int     i, len;
   int      ok_test = 1;

   for (i=0; i < TEST_ELEMENTS; i++) {
      test[i] = i;
   }
   /* *********************************************************************** */
   pf = fifoInit(50);
   TC("Init FIFO", (pf!=NULL));
   printStep(("fifoElements %d\n", fifoElements(pf)));
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   ok_test = 1;
   printStep(("put "));
   for (i=0;i<10 ;i++ )
   {
      if (fifoPut(pf, &(test[i]), sizeof(test[i])) == -1)
      {
         ok_test = 0;
      }
      printStep(("%d ", test[i]));
   }
   printStep(("\n"));
   TC("fifoPut 10", ok_test);
   /* *********************************************************************** */
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 10", fifoElements(pf) == 10);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   ok_test = 1;
   printStep(("get "));
   for (i=0;i<10 ;i++ )
   {
      ptest = (unsigned int*)fifoGet(pf, &len);
      if (ptest != NULL)
      {
        printStep(("%d ", *ptest));
        if (*ptest!=test[i]) ok_test = 0;
      }
      else
      {
        printStep(("NULL "));
        ok_test = 0;
      }
   }
   printStep(("\n"));
   TC("fifoGet10", ok_test);
   /* *********************************************************************** */
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 0", fifoElements(pf) == 0);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   ok_test = 1;
   printStep(("put "));
   for (i=10;i<20 ;i++ )
   {
      if (fifoPut(pf, &(test[i]), sizeof(test[i])) == -1)
         ok_test = 0;
      printStep(("%d ", test[i]));
   }
   printStep(("\n"));
   TC("fifoPut 10", ok_test);
   /* *********************************************************************** */
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 10", fifoElements(pf) == 10);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   printStep(("get "));
   for (i=0;i<5 ;i++ )
   {
      ptest = (unsigned int*)fifoGet(pf, &len);
      if (ptest != NULL)
      {
        printStep(("%d ", *ptest));
        if (*ptest!=test[i+10]) ok_test = 0;
      }
      else
      {
        printStep(("NULL "));
        ok_test = 0;
      }
   }
   printStep(("\n"));
   TC("fifoGet 5", ok_test);
   /* *********************************************************************** */
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 5", fifoElements(pf) == 5);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   ok_test = 1;
   printStep(("put "));
   for (i=20;i<30 ;i++ )
   {
      if (fifoPut(pf, &(test[i]), sizeof(test[i])) == -1)
         ok_test = 0;
      printStep(("%d ", test[i]));
   }
   printStep(("\n"));
   TC("fifoPut 10", ok_test);
   /* *********************************************************************** */
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 15", fifoElements(pf) == 15);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
   /* *********************************************************************** */
   printStep(("all "));
   ok_test = 1;
   ptest = NULL;
   i = 0;
   do {
      ptest = (unsigned int*)fifoGet(pf, &len);
      if (ptest != NULL)
      {
        printStep(("%d ", *ptest));
        if (*ptest!=test[i+15]) ok_test = 0;
      }
      else
      {
        printStep(("NULL "));
        if (i!=15)ok_test = 0;
      }
      i++;
   } while(ptest != NULL);
   printStep(("\n"));
   TC("fifoGet All", ok_test);
   /* *********************************************************************** */
   ok_test = 1;
   printStep(("fifoElements %d\n", fifoElements(pf)));
   TC("ElementCheck 0", fifoElements(pf) == 0);
   /* *********************************************************************** */
   TC("IntegrityCheck", fifoIntegrity(pf) == 0);
  /* *********************************************************************** */
   TC("Free", fifoFree(pf) != -1);
   /* *********************************************************************** */
   TC("ALL TESTS", ok_all);
   /* *********************************************************************** */
   printf("\n");
   return(0);
}
