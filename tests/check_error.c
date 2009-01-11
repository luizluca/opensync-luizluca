#include "support.h"
#include <opensync/opensync-error.h>

START_TEST (error_create)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test%i", 1);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_get_type(&error) == OSYNC_ERROR_GENERIC, NULL);
	fail_unless(!strcmp(osync_error_print(&error), "test1"), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	fail_unless(error == NULL, NULL);
}
END_TEST

START_TEST (error_create_null)
{
  osync_error_set(NULL, OSYNC_ERROR_GENERIC, "test%i", 1);
}
END_TEST

START_TEST (error_get_name_null)
{
	fail_unless(osync_error_get_name(NULL) == NULL, NULL);

}
END_TEST

START_TEST (error_get_name_null2)
{
	OSyncError *error = NULL;
	fail_unless(osync_error_get_name(&error) != NULL, NULL);

}
END_TEST

START_TEST (error_get_name)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test");
	fail_unless(osync_error_get_name(&error) != NULL, NULL);
	osync_error_unref(&error);
}
END_TEST

START_TEST (error_free_null)
{
	osync_error_unref(NULL);

}
END_TEST

START_TEST (error_free_null2)
{
	OSyncError *error = NULL;
	osync_error_unref(&error);

}
END_TEST

START_TEST (error_free)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test");
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	fail_unless(error == NULL, NULL);
}
END_TEST

START_TEST (error_check_null)
{
	fail_unless(osync_error_is_set(NULL) == FALSE, NULL);

}
END_TEST

START_TEST (error_check_null2)
{
	OSyncError *error = NULL;
	fail_unless(osync_error_is_set(&error) == FALSE, NULL);

}
END_TEST

START_TEST (error_check)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test");
	fail_unless(osync_error_is_set(&error) == TRUE, NULL);
	osync_error_unref(&error);
}
END_TEST

START_TEST (error_check2)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_NO_ERROR, NULL);
	fail_unless(osync_error_is_set(&error) == FALSE, NULL);
}
END_TEST

START_TEST (error_stack_null)
{
	osync_error_stack(NULL, NULL);

}
END_TEST

START_TEST (error_stack_null2)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test");
	
	char *msg = osync_error_print_stack(&error);
	fail_unless(msg != NULL, NULL);
	g_free(msg);
	
	osync_error_stack(&error, NULL);
	osync_error_unref(&error);
}
END_TEST

START_TEST (error_stack)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "test");
	OSyncError *error2 = NULL;
	osync_error_set(&error2, OSYNC_ERROR_GENERIC, "test2");
	osync_error_stack(&error, &error2);
	fail_unless(!strcmp(osync_error_print(&error), "test"), NULL);
	OSyncError *error3 = osync_error_get_child(&error);
	fail_unless(!strcmp(osync_error_print(&error3), "test2"), NULL);
	error3 = osync_error_get_child(&error2);
	fail_unless(error3 == NULL, NULL);
	
	osync_error_unref(&error2);
	osync_error_unref(&error);
}
END_TEST

START_TEST (error_stack2)
{
	OSyncError *error1 = NULL;
	osync_error_set(&error1, OSYNC_ERROR_GENERIC, "test1");
	OSyncError *error2 = NULL;
	osync_error_set(&error2, OSYNC_ERROR_GENERIC, "test2");
	OSyncError *error3 = NULL;
	osync_error_set(&error3, OSYNC_ERROR_GENERIC, "test3");
	
	osync_error_stack(&error1, &error2);
	osync_error_stack(&error2, &error3);
	
	osync_error_unref(&error2);
	osync_error_unref(&error3);
	
	fail_unless(!strcmp(osync_error_print(&error1), "test1"), NULL);
	OSyncError *error = osync_error_get_child(&error1);
	fail_unless(!strcmp(osync_error_print(&error), "test2"), NULL);
	error = osync_error_get_child(&error);
	fail_unless(!strcmp(osync_error_print(&error), "test3"), NULL);
	
	char *msg = osync_error_print_stack(&error1);
	fail_unless(msg != NULL, NULL);
	g_free(msg);
	
	osync_error_unref(&error1);
}
END_TEST

START_TEST (error_set_null)
{
	osync_error_set(NULL, OSYNC_NO_ERROR, NULL);

}
END_TEST

START_TEST (error_duplicate_null)
{
	OSyncError *error = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "asd");
	osync_error_set_from_error(NULL, &error);
	osync_error_unref(&error);
}
END_TEST


START_TEST (error_duplicate)
{
	OSyncError *error = NULL;
	OSyncError *error2 = NULL;
	osync_error_set(&error, OSYNC_ERROR_GENERIC, "asd");
	osync_error_set_from_error(&error2, &error);
	
	fail_unless(error2 != NULL, NULL);
	
	osync_error_unref(&error);
	osync_error_unref(&error2);
}
END_TEST

OSYNC_TESTCASE_START("error")
OSYNC_TESTCASE_ADD(error_create)
OSYNC_TESTCASE_ADD(error_create_null)
OSYNC_TESTCASE_ADD(error_get_name_null)
OSYNC_TESTCASE_ADD(error_get_name_null2)
OSYNC_TESTCASE_ADD(error_get_name)
OSYNC_TESTCASE_ADD(error_free_null)
OSYNC_TESTCASE_ADD(error_free_null2)
OSYNC_TESTCASE_ADD(error_free)
OSYNC_TESTCASE_ADD(error_check_null)
OSYNC_TESTCASE_ADD(error_check_null2)
OSYNC_TESTCASE_ADD(error_check)
OSYNC_TESTCASE_ADD(error_check2)
OSYNC_TESTCASE_ADD(error_stack_null)
OSYNC_TESTCASE_ADD(error_stack_null2)
OSYNC_TESTCASE_ADD(error_stack)
OSYNC_TESTCASE_ADD(error_stack2)
OSYNC_TESTCASE_ADD(error_set_null)
OSYNC_TESTCASE_ADD(error_duplicate_null)
OSYNC_TESTCASE_ADD(error_duplicate)
OSYNC_TESTCASE_END

