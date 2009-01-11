#include "support.h"

#include <opensync/opensync-xmlformat.h>

#include "opensync/xmlformat/opensync-xmlformat_internals.h"
#include "opensync/xmlformat/opensync_xmlformat_schema_private.h"	/* FIXME: dierct access of private header */

START_TEST (xmlformat_new)
{
	char *testbed = setup_testbed("capabilities");

	OSyncError *error = NULL;
	OSyncXMLFormat *xmlformat = osync_xmlformat_new("contact", &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_xmlformat_ref(xmlformat);
	osync_xmlformat_unref(xmlformat);
	
	osync_xmlformat_unref(xmlformat);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlformat_parse)
{
	char *testbed = setup_testbed("capabilities");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("capabilities.xml", &buffer, &size, &error), NULL);

	OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_xmlformat_unref(xmlformat);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlformat_sort)
{
	char *testbed = setup_testbed("capabilities");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("capabilities.xml", &buffer, &size, &error), NULL);

	OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_xmlformat_sort(xmlformat);
	
	osync_xmlformat_unref(xmlformat);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlformat_is_sorted)
{
	char *testbed = setup_testbed("xmlformats");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("xmlfield_unsorted.xml", &buffer, &size, &error), NULL);

	OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_xmlformat_is_sorted(xmlformat) == FALSE, NULL);

	osync_xmlformat_sort(xmlformat);

	fail_unless(osync_xmlformat_is_sorted(xmlformat) == TRUE, NULL);
	
	osync_xmlformat_unref(xmlformat);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST


START_TEST (xmlformat_search_field)
{
	char *testbed = setup_testbed("capabilities");

	char *buffer;
	unsigned int size;
	OSyncError *error = NULL;

	fail_unless(osync_file_read("contact.xml", &buffer, (unsigned int *)(&size), &error), NULL);
	OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	g_free(buffer);
	osync_xmlformat_sort(xmlformat);
	
	OSyncXMLFieldList *xmlfieldlist = osync_xmlformat_search_field(xmlformat, "Name", &error, NULL);
	fail_unless(xmlfieldlist != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(size != osync_xmlfieldlist_get_length(xmlfieldlist), NULL);

	osync_xmlfieldlist_free(xmlfieldlist);
	osync_xmlformat_unref(xmlformat);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlfield_new)
{
	char *testbed = setup_testbed("capabilities");

	OSyncError *error = NULL;
	OSyncXMLFormat *xmlformat = osync_xmlformat_new("contact", &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncXMLField *xmlfield = osync_xmlfield_new(xmlformat, "Name", &error);
	fail_unless(xmlfield != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_xmlformat_unref(xmlformat);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlfield_sort)
{
	char *testbed = setup_testbed("xmlformats");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("xmlfield_unsorted.xml", &buffer, &size, &error), NULL);

	OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
	fail_unless(xmlformat != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncXMLField *xmlfield = osync_xmlformat_get_first_field(xmlformat);
	for (; xmlfield != NULL; xmlfield = osync_xmlfield_get_next(xmlfield)) {
		osync_xmlfield_sort(xmlfield);
	}

	xmlfield = osync_xmlformat_get_first_field(xmlformat);
	for (; xmlfield != NULL; xmlfield = osync_xmlfield_get_next(xmlfield)) {
		int count = osync_xmlfield_get_key_count(xmlfield);

		if (count > 0)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 0), "ABCDEFG"), NULL);

		if (count > 1)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 1), "BCDEFG"), NULL);

		if (count > 2)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 2), "CDEFG"), NULL);

		if (count > 3)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 3), "DEFG"), NULL);

		if (count > 4)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 4), "EFG"), NULL);

		if (count > 5)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 5), "FG"), NULL);

		if (count > 6)
			fail_unless(!strcmp(osync_xmlfield_get_nth_key_name(xmlfield, 6), "G"), NULL);

	}
	
	osync_xmlformat_unref(xmlformat);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (xmlformat_schema_validate)
{
        char *testbed = setup_testbed("xmlformats");
        char *buffer;
        unsigned int size;
        OSyncError *error = NULL;
        OSyncXMLFormatSchema *schema = NULL;

        fail_unless(osync_file_read("mockobjtype.xml", &buffer, &size, &error), NULL);
        fail_unless(error == NULL, NULL);

        OSyncXMLFormat *xmlformat = osync_xmlformat_parse(buffer, size, &error);
        fail_unless(error == NULL, NULL);

        g_free(buffer);
        schema = osync_xmlformat_schema_new_xmlformat(xmlformat, testbed, &error);
        fail_if( schema == NULL );
        fail_unless( osync_xmlformat_schema_validate(schema, xmlformat, &error) );

        osync_xmlformat_schema_unref(schema);

        osync_xmlformat_unref(xmlformat);

        destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("xmlformat")
// xmlformat
OSYNC_TESTCASE_ADD(xmlformat_new)
OSYNC_TESTCASE_ADD(xmlformat_parse)
OSYNC_TESTCASE_ADD(xmlformat_sort)
OSYNC_TESTCASE_ADD(xmlformat_is_sorted)
OSYNC_TESTCASE_ADD(xmlformat_search_field)
// xmlformat schema
OSYNC_TESTCASE_ADD(xmlformat_schema_validate)
// xmlfield
OSYNC_TESTCASE_ADD(xmlfield_new)
OSYNC_TESTCASE_ADD(xmlfield_sort)
OSYNC_TESTCASE_END

