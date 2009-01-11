#include <check.h>
#include <opensync/opensync.h>
#include <opensync/opensync-group.h>
#include <opensync/opensync_internals.h>
#include <stdlib.h>

#include <glib.h>
#include <gmodule.h>

#include "support.h"

START_TEST (member_new)
{
  OSyncMember *member = NULL;
  member = osync_member_new(NULL);
  fail_unless(member != NULL, "Member == NULL on creation");
  osync_member_unref(member);
}
END_TEST

OSYNC_TESTCASE_START("member")
OSYNC_TESTCASE_ADD(member_new)
OSYNC_TESTCASE_END

