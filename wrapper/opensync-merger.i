/* FIXME - this needs to be overhauled, since a Capability is of no use
   unless it is one that is connected to the Capabilities hierarchy.

typedef struct {} Capability;
%extend Capability {
	Capability() {
		Error *err = NULL;
		Capability *cap = osync_capability_new(&err);
		if (raise_exception_on_error(err))
			return NULL;
		else
			return cap;
	}

	~Capability() {
                osync_capability_unref(self);
	}

	const char *get_name() {
		return osync_capability_get_name(self);
	}

%pythoncode %{
	name = property(get_name)

	# extend the SWIG-generated constructor, so that we can setup our list-wrapper classes
	__oldinit = __init__
	def __init__(self, *args):
		self.__oldinit(*args)
%}
}
*/

typedef struct {} Capabilities;
%extend Capabilities {
	Capabilities(const char *capsformat) {
		Error *err = NULL;
		Capabilities *caps = osync_capabilities_new(capsformat, &err);
		if (raise_exception_on_error(err))
			return NULL;
		else
			return caps;
	}

	~Capabilities() {
		osync_capabilities_unref(self);
	}
}

/* FIXME: cstring_input_binary is broken in my version of swig, so I've recreated it here */
%typemap(in) (const char *buffer, size_t size) {
	int alloc = 0;
	int res = SWIG_AsCharPtrAndSize($input, &$1, &$2, &alloc);
	if (!SWIG_IsOK(res)) {
		%argument_fail(res, "(char *buf, size_t size)", $symname, $argnum);
	}
}

%{
/* convert an XMLFieldList to a python list */
static PyObject *xmlfieldlist_to_pylist(XMLFieldList *list)
{
	PyObject *ret = PyList_New(0);
	if (ret == NULL)
		return NULL;
	int n, max = osync_xmlfieldlist_get_length(list);
	for (n = 0; n < max; n++) {
		PyObject *obj = SWIG_NewPointerObj(osync_xmlfieldlist_item(list, n), SWIGTYPE_p_XMLField, 0);
		if (!obj || PyList_Append(ret, obj) != 0) {
			Py_DECREF(ret);
			return NULL;
		}
	}
	return ret;
}
%}

typedef struct {} XMLFormat;
%extend XMLFormat {
	XMLFormat(const char *objtype) {
		Error *err = NULL;
		XMLFormat *xmlformat = osync_xmlformat_new(objtype, &err);
		if (raise_exception_on_error(err))
			return NULL;
		else
			return xmlformat;
	}

	~XMLFormat() {
		osync_xmlformat_unref(self);
	}

	XMLField *get_first_field() {
		return osync_xmlformat_get_first_field(self);
	}

	/* FIXME: this is a varargs function, it should take a list of tuples or similar */
	PyObject *search_field(const char *name) {
		OSyncError *err = NULL;
		XMLFieldList *list = osync_xmlformat_search_field(self, name, &err, NULL);
		if (raise_exception_on_error(err))
			return NULL;
		if (!list) {
			wrapper_exception("osync_xmlformat_search_field failed but did not set error code");
			return NULL;
		}

		PyObject *ret = xmlfieldlist_to_pylist(list);
		/* FIXME: osync_xmlfieldlist_free frees the list structure and the nodes as well,
		 * but we are returning references to the nodes, so here we just want to free the list.
		 * So, we reach around the API and free the list object directly. */
		free(list);
		return ret;
	}

	/* returns a python string object */
	PyObject *assemble() {
		char *buf;
		unsigned int size;
                Error *err = NULL;
		osync_xmlformat_assemble(self, &buf, &size, &err);
		if (raise_exception_on_error(err))
			return NULL;

		PyObject *obj = PyString_FromStringAndSize(buf, size);
		free(buf);
		return obj;
	}

	void sort() {
                Error *err = NULL;
		osync_xmlformat_sort(self, &err);
		if (raise_exception_on_error(err))
			return;
	}

	const char *get_objtype() {
		return osync_xmlformat_get_objtype(self);
	}

%pythoncode %{
	objtype = property(get_objtype)
%}
}

%inline %{
	XMLFormat *xmlformat_parse(const char *buffer, size_t size) {
		Error *err = NULL;
		XMLFormat *xmlformat = osync_xmlformat_parse(buffer, (unsigned int)size, &err);
		if (raise_exception_on_error(err))
			return NULL;
		else
			return xmlformat;
	}

%}


typedef struct {} XMLField;
%extend XMLField {
	XMLField(XMLFormat *xmlformat, const char *name) {
		Error *err = NULL;
		XMLField *xmlfield = osync_xmlfield_new(xmlformat, name, &err);
		if (raise_exception_on_error(err))
			return NULL;
		else
			return xmlfield;
	}

	~XMLField() {
                /* TODO: re-export xmlfield_unref for wrapper deconstructor?
		osync_xmlfield_unref(self);
                */
	}

	const char *get_name() {
		return osync_xmlfield_get_name(self);
	}

	XMLField *get_next() {
		return osync_xmlfield_get_next(self);
	}

	const char *get_attr(const char *attr) {
		return osync_xmlfield_get_attr(self, attr);
	}

	void set_attr(const char *attr, const char *value) {
		osync_xmlfield_set_attr(self, attr, value);
	}

	int get_attr_count() {
		return osync_xmlfield_get_attr_count(self);
	}

	const char *get_nth_attr_name(int nth) {
		return osync_xmlfield_get_nth_attr_name(self, nth);
	}

	const char *get_nth_attr_value(int nth) {
		return osync_xmlfield_get_nth_attr_value(self, nth);
	}

	const char *get_key_value(const char *key) {
		return osync_xmlfield_get_key_value(self, key);
	}

        /* TODO return value bool */
	void set_key_value(const char *key, const char *value) {
                OSyncError *err = NULL;
		osync_xmlfield_set_key_value(self, key, value, &err);
		if (raise_exception_on_error(err))
			return;
	}

        /* TODO return value bool */
	void add_key_value(const char *key, const char *value) {
                OSyncError *err;
		osync_xmlfield_add_key_value(self, key, value, &err);
		if (raise_exception_on_error(err))
			return;
	}

	int get_key_count() {
		return osync_xmlfield_get_key_count(self);
	}

	const char *get_nth_key_name(int nth) {
		return osync_xmlfield_get_nth_key_name(self, nth);
	}

	const char *get_nth_key_value(int nth) {
		return osync_xmlfield_get_nth_key_value(self, nth);
	}

	void set_nth_key_value(int nth, const char *value) {
		osync_xmlfield_set_nth_key_value(self, nth, value);
	}

%pythoncode %{
	name = property(get_name)
	next = property(get_next)

	# map attributes and keys to two properties of the XMLField object
	class Attrs:
		def __init__(self, xmlfield):
			self.xmlfield = xmlfield
		def __len__(self):
			return self.xmlfield.get_attr_count()
		def __getitem__(self, key):
			return self.xmlfield.get_attr(key)
		def __setitem__(self, key, value):
			self.xmlfield.set_attr(key, value)
		def __iter__(self):
			class Iter:
				def __init__(self, xmlfield):
					self.xmlfield = xmlfield
					self.pos = 0
					self.maxpos = self.xmlfield.get_attr_count()
				def __iter__(self):
					return self
				def next(self):
					if self.pos > self.maxpos:
						raise StopIteration
					name = self.xmlfield.get_nth_attr_name(self.pos)
					value = self.xmlfield.get_nth_attr_value(self.pos)
					self.pos += 1
					return (name, value)
			return Iter(self.xmlfield)

	class Keys:
		def __init__(self, xmlfield):
			self.xmlfield = xmlfield
		def __len__(self):
			return self.xmlfield.get_key_count()
		def __getitem__(self, key):
			return self.xmlfield.get_key_value(key)
		def __setitem__(self, key, value):
			self.xmlfield.set_key_value(key, value)
		def __iter__(self):
			class Iter:
				def __init__(self, xmlfield):
					self.xmlfield = xmlfield
					self.pos = 0
					self.maxpos = self.xmlfield.get_key_count()
				def __iter__(self):
					return self
				def next(self):
					if self.pos > self.maxpos:
						raise StopIteration
					name = self.xmlfield.get_nth_key_name(self.pos)
					value = self.xmlfield.get_nth_key_value(self.pos)
					self.pos += 1
					return (name, value)
			return Iter(self.xmlfield)

	attrs = property(Attrs)
	keys = property(Keys)
%}
}

