#include "support.h"

#include <opensync/opensync-xmlformat.h>

#include "opensync/xmlformat/opensync-xmlformat_internals.h"
#include "opensync/xmlformat/opensync_xmlformat_schema_private.h"	/* FIXME: dierct access of private header */

START_TEST (xmlformat_new)
{
	char *testbed = setup_testbed("merger");

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
	char *testbed = setup_testbed("merger");
	
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
	char *testbed = setup_testbed("merger");
	
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
	char *testbed = setup_testbed("merger");

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
	char *testbed = setup_testbed("merger");

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

START_TEST (xmlformat_schema_get_instance)
{
	char *testbed = setup_testbed("xmlformats");	

	OSyncError *error = NULL;

	//TODO disable libxml2 output to stderr

	OSyncXMLFormat *xmlformat = osync_xmlformat_new("abc", &error);
	OSyncXMLFormatSchema *failschema = osync_xmlformat_schema_get_instance_with_path(xmlformat, testbed, &error);
	fail_unless(failschema == NULL);
	osync_xmlformat_unref(xmlformat);

	xmlformat = osync_xmlformat_new("mockobjtype", &error);
	fail_if(xmlformat == NULL, NULL);
	fail_if(error == NULL, NULL);
	
	OSyncXMLFormatSchema *schema1 = osync_xmlformat_schema_get_instance_with_path(xmlformat, testbed, &error);
	OSyncXMLFormatSchema *schema2 = osync_xmlformat_schema_get_instance_with_path(xmlformat, testbed, &error);
	fail_if(schema1 == NULL);
	fail_if(schema2 == NULL);
	fail_unless( schema1 == schema2 );
	fail_unless( schema1->ref_count == 2 );

	osync_xmlformat_schema_unref(schema1);
	osync_xmlformat_schema_unref(schema2);

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
        schema = osync_xmlformat_schema_new(xmlformat, testbed, &error);
        fail_if( schema == NULL );
        fail_unless( osync_xmlformat_schema_validate(schema, xmlformat, &error) );

        osync_xmlformat_schema_unref(schema);

        osync_xmlformat_unref(xmlformat);

        destroy_testbed(testbed);
}
END_TEST

Suite *xmlformat_suite(void)
{
	Suite *s = suite_create("XMLFormat");
//	Suite *s2 = suite_create("XMLFormat");

	// xmlformat
	create_case(s, "xmlformat_new", xmlformat_new);
	create_case(s, "xmlformat_parse", xmlformat_parse);
	create_case(s, "xmlformat_sort", xmlformat_sort);
	create_case(s, "xmlformat_is_sorted", xmlformat_is_sorted);
	create_case(s, "xmlformat_search_field", xmlformat_search_field);

	// xmlformat schema
	create_case(s, "xmlformat_schema_get_instance", xmlformat_schema_get_instance);
	create_case(s, "xmlformat_schema_validate", xmlformat_schema_validate);

	// xmlfield
	create_case(s, "xmlfield_new", xmlfield_new);
	create_case(s, "xmlfield_sort", xmlfield_sort);

	return s;
}

int main(void)
{
	int nf;

	Suite *s = xmlformat_suite();
	
	SRunner *sr;
	sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
