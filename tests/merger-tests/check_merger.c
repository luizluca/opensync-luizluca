#include "support.h"

#include <opensync/opensync-merger.h>
#include <opensync/opensync-xmlformat.h>
#include "opensync/merger/opensync-merger_internals.h"
#include "opensync/xmlformat/opensync_xmlformat_internals.h"


START_TEST (merger_new)
{
	char *testbed = setup_testbed("merger");

	OSyncError *error = NULL;
	OSyncCapabilities *capabilities = osync_capabilities_new(&error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncMerger *merger = osync_merger_new(capabilities, &error);
	fail_unless(merger != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_merger_ref(merger);
	osync_merger_unref(merger);
	
	osync_merger_unref(merger);
	osync_capabilities_unref(capabilities);

	destroy_testbed(testbed);
}
END_TEST

Suite *filter_suite(void)
{
	Suite *s = suite_create("Merger");
	create_case(s, "merger_new", merger_new);
	return s;
}

int main(void)
{
	int nf;

	Suite *s = filter_suite();
	
	SRunner *sr;
	sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
