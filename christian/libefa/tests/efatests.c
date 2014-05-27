/*
 * Author: Christian Göttel <christian.goettel@unifr.ch>
 */

#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"
#include "efatooltests.h"

int main(int argc __attribute__ ((unused)),
	 char *argv[] __attribute__ ((unused))) {
  CuString *output = NULL;
  CuSuite *suite = NULL;

  output = CuStringNew();
  suite = CuSuiteNew();

  CuSuiteAddSuite(suite, efatoolGetSuite());

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);

  free(output);
  free(suite);
  output = NULL;
  suite = NULL;

  return 0;
}
