%inline %{
	static bool anchor_compare(OSyncAnchor *anchor, const char *new_anchor) {
		Error *err = NULL;
                bool ret;

		osync_anchor_compare(anchor, new_anchor, &ret, &err);

                if (raise_exception_on_error(err))
                        return FALSE;

                return ret;
	}

	static bool anchor_update(OSyncAnchor *anchor, const char *new_anchor) {
		Error *err = NULL;
		osync_anchor_update(anchor, new_anchor, &err);
                if (raise_exception_on_error(err))
                        return FALSE;

                return TRUE;
	}

	static char *anchor_retrieve(OSyncAnchor *anchor) {
                Error *err = NULL;
                char *ret;
		ret = osync_anchor_retrieve(anchor, &err);

                if (raise_exception_on_error(err))
                        return NULL;

                return ret;
	}
%}

%inline %{
        static bool hashtable_slowsync(OSyncHashTable *hashtable) {
                Error *err = NULL;
                osync_hashtable_slowsync(hashtable, &err);

                if (raise_exception_on_error(err))
                        return FALSE;

                return TRUE;
        }

	static int hashtable_num_entries(OSyncHashTable *hashtable) {
		return osync_hashtable_num_entries(hashtable);
	}

	void hashtable_update_change(OSyncHashTable *hashtable, Change *change) {
		osync_hashtable_update_change(hashtable, change);
	}

	/* returns a list of deleted UIDs as strings */
	static PyObject *hashtable_get_deleted(OSyncHashTable *hashtable) {
		OSyncList *uids = osync_hashtable_get_deleted(hashtable);
		if (uids == NULL) {
			wrapper_exception("osync_hashtable_get_deleted failed");
			return NULL;
		}

		PyObject *ret = PyList_New(0);
		if (ret != NULL) {
                        OSyncList *u;
			for (u = uids; u; u = u->next) {
                                char *uid = u->data;
				PyObject *item = PyString_FromString(uid);
				if (item == NULL || PyList_Append(ret, item) != 0) {
					Py_XDECREF(item);
					ret = NULL;
					break;
				}
				Py_DECREF(item);
			}
		}

		return ret;
	}

	static ChangeType hashtable_get_changetype(OSyncHashTable *hashtable, Change *change) {
		return osync_hashtable_get_changetype(hashtable, change);
	}
%}
