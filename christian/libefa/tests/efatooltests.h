/*
 * Author: Christian Göttel <christian.goettel@unifr.ch>
 */
#include "efatool.h"
#include "CuTest.h"

CuSuite *efatoolGetSuite(void);

// parse_opt_algorithm() tests
void Test_1_parse_opt_algorithm(CuTest *);
void Test_2_parse_opt_algorithm(CuTest *);

// parse_opt_at() tests
void Test_1_parse_opt_at(CuTest *);
void Test_2_parse_opt_at(CuTest *);
void Test_3_parse_opt_at(CuTest *);
#if HAVE_MODULE_BUECHI
void Test_4_parse_opt_at(CuTest *);
#endif /* HAVE_MODULE_BUECHI */

// parse_opt_class() tests
void Test_1_parse_opt_class(CuTest *);
void Test_2_parse_opt_class(CuTest *);
void Test_3_parse_opt_class(CuTest *);
void Test_4_parse_opt_class(CuTest *);

// parse_opt_ds() tests
void Test_1_parse_opt_ds(CuTest *);
void Test_2_parse_opt_ds(CuTest *);

// parse_opt_hash() tests
#if HAVE_MODULE_HASH
void Test_1_parse_opt_hash(CuTest *);
void Test_2_parse_opt_hash(CuTest *);
#endif /* HAVE_MODULE_HASH */

void Test_1_parse_opt_verbose(CuTest *);
void Test_2_parse_opt_verbose(CuTest *);
