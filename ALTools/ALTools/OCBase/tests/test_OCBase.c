#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#define VAR_UNUSED(x) (void)(x)

START_TEST(testname)
{
  fail_if(0, "true");
}
END_TEST



int main(int argc, char** argv){
  int number_failed;

  Suite* suite = suite_create("OCBase Suite");
  TCase* tcase = tcase_create("case");
  tcase_add_test(tcase, testname);
  suite_add_tcase(suite, tcase);

  SRunner* runner = srunner_create(suite);
  srunner_run_all(runner, CK_NORMAL);
  number_failed = srunner_ntests_failed(runner);

  srunner_free(runner);
  
  VAR_UNUSED(argc);
  VAR_UNUSED(argv);

  return number_failed;
}
