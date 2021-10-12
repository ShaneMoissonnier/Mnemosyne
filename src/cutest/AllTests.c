#include "../common.h"
#include "../mem.h"
#include "../mem_os.h"

#include <stdio.h>

#include "CuTest.h"
#include "AllTests.h"

void testAlloc(CuTest *tc)
{
  mem_init();
  void *ptr = mem_alloc(48);
  CuAssertPtrNotNull(tc, ptr);
  mem_free(ptr);
}

CuSuite *MaTestSuite()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, testAlloc);
  return suite;
}

void RunAllTests(void)
{
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();

  CuSuiteAddSuite(suite, MaTestSuite());

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
}

int main(int argc, char *argv[])
{
  RunAllTests();
  return 0;
}