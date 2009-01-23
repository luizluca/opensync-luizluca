#include "support.h"

#include <opensync/opensync.h>
#include <opensync/opensync-archive.h>
#include "archive/opensync_archive_internals.h"


START_TEST (archive_new)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new((const char *)"archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_archive_ref(archive);
	osync_archive_unref(archive);
	
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (archive_load_changes)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncList *ids;
	OSyncList *uids;
	OSyncList *mappingids;
	OSyncList *memberids;
	osync_archive_load_changes(archive, "contact", &ids, &uids, &mappingids, &memberids, &error);
		
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (archive_save_change)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncList *ids;
	OSyncList *uids;
	OSyncList *mappingids;
	OSyncList *memberids;
	osync_archive_load_changes(archive, "contact", &ids, &uids, &mappingids, &memberids, &error);
	
	long long int id = osync_archive_save_change(archive, 0, "uid", "contact", 1, 1, "contact", &error);
	fail_unless(id != 0, NULL);
	fail_unless(error == NULL, NULL);
		
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (archive_save_data)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncList *ids;
	OSyncList *uids;
	OSyncList *mappingids;
	OSyncList *memberids;
	osync_archive_load_changes(archive, "contact", &ids, &uids, &mappingids, &memberids, &error);
	
	long long int id = osync_archive_save_change(archive, 0, "uid", "contact", 1, 1, "contact", &error);
	fail_unless(id != 0, NULL);
	fail_unless(error == NULL, NULL);
	
	const char *testdata = "testdata";
	unsigned int testsize = strlen(testdata);
	fail_unless(osync_archive_save_data(archive, 1, "contact", testdata, testsize, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);
		
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (archive_load_data)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncList *ids;
	OSyncList *uids;
	OSyncList *mappingids;
	OSyncList *memberids;
	osync_archive_load_changes(archive, "contact", &ids, &uids, &mappingids, &memberids, &error);
	
	long long int id = osync_archive_save_change(archive, 0, "uid", "contact", 1, 1, "contact", &error);
	fail_unless(id != 0, NULL);
	fail_unless(error == NULL, NULL);
	
	const char *testdata = "testdata";
	unsigned int testsize = strlen(testdata);
	fail_unless(osync_archive_save_data(archive, 1, "contact", testdata, testsize, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);
	
	char *buffer;
	unsigned int size;
	fail_unless(osync_archive_load_data(archive, "uid", "contact", &buffer, &size, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(size == testsize);
	fail_unless(memcmp(buffer, testdata, testsize) == 0);

	g_free(buffer);
		
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (archive_load_data_with_closing_db)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncArchive *archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncList *ids;
	OSyncList *uids;
	OSyncList *mappingids;
	OSyncList *memberids;
	osync_archive_load_changes(archive, "contact", &ids, &uids, &mappingids, &memberids, &error);
	
	long long int id = osync_archive_save_change(archive, 0, "uid", "contact", 1, 1, "contact", &error);
	fail_unless(id != 0, NULL);
	fail_unless(error == NULL, NULL);
	
	const char *testdata = "testdata";
	unsigned int testsize = strlen(testdata);
	fail_unless(osync_archive_save_data(archive, 1, "contact", testdata, testsize, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_archive_unref(archive);
	archive = osync_archive_new("archive.db", &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *buffer;
	unsigned int size;
	fail_unless(osync_archive_load_data(archive, "uid", "contact", &buffer, &size, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(size == testsize);
	fail_unless(memcmp(buffer, testdata, testsize) == 0);

	g_free(buffer);
		
	osync_archive_unref(archive);

	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("archive")
OSYNC_TESTCASE_ADD(archive_new)
OSYNC_TESTCASE_ADD(archive_load_changes)
OSYNC_TESTCASE_ADD(archive_save_change)
OSYNC_TESTCASE_ADD(archive_save_data)
OSYNC_TESTCASE_ADD(archive_load_data)
OSYNC_TESTCASE_ADD(archive_load_data_with_closing_db)
OSYNC_TESTCASE_END

