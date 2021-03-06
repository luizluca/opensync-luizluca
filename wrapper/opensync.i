%module(docstring="Python bindings for the OpenSync library") opensync
%feature("autodoc", "1");
%include "cstring.i"

%{
#include <opensync/opensync.h>
#include <opensync/opensync-capabilities.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-engine.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-group.h>
#include <opensync/opensync-helper.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync-version.h>
#include <opensync/opensync-xmlformat.h>

typedef OSyncCapabilities Capabilities;
typedef OSyncCapability Capability;
typedef OSyncChange Change;
typedef OSyncChangeType ChangeType;
typedef OSyncPluginConfigurationType ConfigurationType;
typedef OSyncConflictResolution ConflictResolution;
typedef OSyncContext Context;
typedef OSyncConvCmpResult ConvCmpResult;
/*
typedef OSyncCustomFilter CustomFilter;
*/
typedef OSyncData Data;
typedef OSyncEngine Engine;
typedef OSyncEngineEvent EngineEvent;
typedef OSyncError Error;
typedef OSyncErrorType ErrorType;
/*
typedef OSyncFilterAction FilterAction;
typedef OSyncFilter Filter;
*/
typedef OSyncFormatConverter FormatConverter;
typedef OSyncFormatConverterPath FormatConverterPath;
typedef OSyncFormatEnv FormatEnv;
typedef OSyncGroupEnv GroupEnv;
typedef OSyncGroup Group;
typedef OSyncHashTable HashTable;
typedef OSyncLockState LockState;
typedef OSyncMember Member;
typedef OSyncObjFormat ObjFormat;
typedef OSyncObjFormatSink ObjFormatSink;
typedef OSyncObjTypeSink ObjTypeSink;
typedef OSyncPluginConfig PluginConfig;
typedef OSyncPluginEnv PluginEnv;
typedef OSyncPluginInfo PluginInfo;
typedef OSyncPlugin Plugin;
typedef OSyncStartType StartType;
typedef OSyncTraceType TraceType;
typedef OSyncVersion Version;
typedef OSyncXMLField XMLField;
typedef OSyncXMLFieldList XMLFieldList;
typedef OSyncXMLFormat XMLFormat;

/* make SWIG treat osync_bool as real Python booleans */
typedef osync_bool bool;
#define true TRUE
#define false FALSE
%}

%pythoncode %{
	class _ListWrapper:
		"""Utility class to wrap a common idiom in OpenSync as a Python list."""
		def __init__(self, lenf, getf):
			self.__lenf = lenf
			self.__getf = getf
		
		def __len__(self):
			return self.__lenf()
		
		def __getitem__(self, num):
			if not isinstance(num, int):
				raise TypeError
			if num < 0 or num >= len(self):
				raise IndexError
			return self.__getf(num)
%}

/* macro to define arbitrary output argument pointer typemaps, based on:
 * http://embedded.eecs.berkeley.edu/Alumni/pinhong/scriptEDA/pyTypemapFAQ.html#22
 */
%define define_ptr_argout(T)
%typemap(argout) T* OUTPUT {
	PyObject *o = SWIG_NewPointerObj((void *)$1, SWIGTYPE_p_##T);
	$result = l_output_helper($result, o);
}
%enddef

/* define output typemaps for pointer types we need */
define_ptr_argout(ConflictResolution);

%{
/* convert an OSyncList linked list to a python list of the given type */
static PyObject *osynclist_to_pylist(const OSyncList *elt, swig_type_info *type)
{
	PyObject *ret = PyList_New(0);
	if (ret == NULL)
		return NULL;
	for (; elt != NULL; elt = elt->next) {
		PyObject *obj = SWIG_NewPointerObj(elt->data, type, 0);
		if (!obj || PyList_Append(ret, obj) != 0) {
			Py_DECREF(ret);
			return NULL;
		}
	}
	return ret;
}
%}

/* force all functions to return NULL if a python exception has been raised */
%exception {
	$action
	if (PyErr_Occurred())
		return NULL;
}

%include "opensync-error.i"

%include "opensync-context.i"
%include "opensync-data.i"
%include "opensync-engine.i"
%include "opensync-format.i"
%include "opensync-group.i"
%include "opensync-helper.i"
%include "opensync-merger.i"
%include "opensync-plugin.i"
%include "opensync-support.i"
%include "opensync-version.i"
