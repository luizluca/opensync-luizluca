%inline %{
	static bool sink_state_equal(OSyncSinkStateDB *state_db, const char *key, const char *value) {
		Error *err = NULL;
                osync_bool ret;

		osync_sink_state_equal(state_db, key, value, &ret, &err);

                if (raise_exception_on_error(err))
                        return FALSE;

                return ret;
	}

	static bool sink_state_set(OSyncSinkStateDB *state_db, const char *key, const char *value) {
		Error *err = NULL;
		osync_sink_state_set(state_db, key, value, &err);
                if (raise_exception_on_error(err))
                        return FALSE;

                return TRUE;
	}

	static char *sink_state_get(OSyncSinkStateDB *state_db, const char *key) {
                Error *err = NULL;
                char *ret;
		ret = osync_sink_state_get(state_db, key, &err);

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
