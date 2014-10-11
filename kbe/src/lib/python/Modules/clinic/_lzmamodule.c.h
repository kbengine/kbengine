/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(_lzma_LZMACompressor_compress__doc__,
"compress($self, data, /)\n"
"--\n"
"\n"
"Provide data to the compressor object.\n"
"\n"
"Returns a chunk of compressed data if possible, or b\'\' otherwise.\n"
"\n"
"When you have finished providing data to the compressor, call the\n"
"flush() method to finish the compression process.");

#define _LZMA_LZMACOMPRESSOR_COMPRESS_METHODDEF    \
    {"compress", (PyCFunction)_lzma_LZMACompressor_compress, METH_VARARGS, _lzma_LZMACompressor_compress__doc__},

static PyObject *
_lzma_LZMACompressor_compress_impl(Compressor *self, Py_buffer *data);

static PyObject *
_lzma_LZMACompressor_compress(Compressor *self, PyObject *args)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTuple(args,
        "y*:compress",
        &data))
        goto exit;
    return_value = _lzma_LZMACompressor_compress_impl(self, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(_lzma_LZMACompressor_flush__doc__,
"flush($self, /)\n"
"--\n"
"\n"
"Finish the compression process.\n"
"\n"
"Returns the compressed data left in internal buffers.\n"
"\n"
"The compressor object may not be used after this method is called.");

#define _LZMA_LZMACOMPRESSOR_FLUSH_METHODDEF    \
    {"flush", (PyCFunction)_lzma_LZMACompressor_flush, METH_NOARGS, _lzma_LZMACompressor_flush__doc__},

static PyObject *
_lzma_LZMACompressor_flush_impl(Compressor *self);

static PyObject *
_lzma_LZMACompressor_flush(Compressor *self, PyObject *Py_UNUSED(ignored))
{
    return _lzma_LZMACompressor_flush_impl(self);
}

PyDoc_STRVAR(_lzma_LZMADecompressor_decompress__doc__,
"decompress($self, data, /)\n"
"--\n"
"\n"
"Provide data to the decompressor object.\n"
"\n"
"Returns a chunk of decompressed data if possible, or b\'\' otherwise.\n"
"\n"
"Attempting to decompress data after the end of stream is reached\n"
"raises an EOFError.  Any data found after the end of the stream\n"
"is ignored and saved in the unused_data attribute.");

#define _LZMA_LZMADECOMPRESSOR_DECOMPRESS_METHODDEF    \
    {"decompress", (PyCFunction)_lzma_LZMADecompressor_decompress, METH_VARARGS, _lzma_LZMADecompressor_decompress__doc__},

static PyObject *
_lzma_LZMADecompressor_decompress_impl(Decompressor *self, Py_buffer *data);

static PyObject *
_lzma_LZMADecompressor_decompress(Decompressor *self, PyObject *args)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTuple(args,
        "y*:decompress",
        &data))
        goto exit;
    return_value = _lzma_LZMADecompressor_decompress_impl(self, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(_lzma_LZMADecompressor___init____doc__,
"LZMADecompressor(format=FORMAT_AUTO, memlimit=None, filters=None)\n"
"--\n"
"\n"
"Create a decompressor object for decompressing data incrementally.\n"
"\n"
"  format\n"
"    Specifies the container format of the input stream.  If this is\n"
"    FORMAT_AUTO (the default), the decompressor will automatically detect\n"
"    whether the input is FORMAT_XZ or FORMAT_ALONE.  Streams created with\n"
"    FORMAT_RAW cannot be autodetected.\n"
"  memlimit\n"
"    Limit the amount of memory used by the decompressor.  This will cause\n"
"    decompression to fail if the input cannot be decompressed within the\n"
"    given limit.\n"
"  filters\n"
"    A custom filter chain.  This argument is required for FORMAT_RAW, and\n"
"    not accepted with any other format.  When provided, this should be a\n"
"    sequence of dicts, each indicating the ID and options for a single\n"
"    filter.\n"
"\n"
"For one-shot decompression, use the decompress() function instead.");

static int
_lzma_LZMADecompressor___init___impl(Decompressor *self, int format, PyObject *memlimit, PyObject *filters);

static int
_lzma_LZMADecompressor___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    static char *_keywords[] = {"format", "memlimit", "filters", NULL};
    int format = FORMAT_AUTO;
    PyObject *memlimit = Py_None;
    PyObject *filters = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
        "|iOO:LZMADecompressor", _keywords,
        &format, &memlimit, &filters))
        goto exit;
    return_value = _lzma_LZMADecompressor___init___impl((Decompressor *)self, format, memlimit, filters);

exit:
    return return_value;
}

PyDoc_STRVAR(_lzma_is_check_supported__doc__,
"is_check_supported($module, check_id, /)\n"
"--\n"
"\n"
"Test whether the given integrity check is supported.\n"
"\n"
"Always returns True for CHECK_NONE and CHECK_CRC32.");

#define _LZMA_IS_CHECK_SUPPORTED_METHODDEF    \
    {"is_check_supported", (PyCFunction)_lzma_is_check_supported, METH_VARARGS, _lzma_is_check_supported__doc__},

static PyObject *
_lzma_is_check_supported_impl(PyModuleDef *module, int check_id);

static PyObject *
_lzma_is_check_supported(PyModuleDef *module, PyObject *args)
{
    PyObject *return_value = NULL;
    int check_id;

    if (!PyArg_ParseTuple(args,
        "i:is_check_supported",
        &check_id))
        goto exit;
    return_value = _lzma_is_check_supported_impl(module, check_id);

exit:
    return return_value;
}

PyDoc_STRVAR(_lzma__encode_filter_properties__doc__,
"_encode_filter_properties($module, filter, /)\n"
"--\n"
"\n"
"Return a bytes object encoding the options (properties) of the filter specified by *filter* (a dict).\n"
"\n"
"The result does not include the filter ID itself, only the options.");

#define _LZMA__ENCODE_FILTER_PROPERTIES_METHODDEF    \
    {"_encode_filter_properties", (PyCFunction)_lzma__encode_filter_properties, METH_VARARGS, _lzma__encode_filter_properties__doc__},

static PyObject *
_lzma__encode_filter_properties_impl(PyModuleDef *module, lzma_filter filter);

static PyObject *
_lzma__encode_filter_properties(PyModuleDef *module, PyObject *args)
{
    PyObject *return_value = NULL;
    lzma_filter filter = {LZMA_VLI_UNKNOWN, NULL};

    if (!PyArg_ParseTuple(args,
        "O&:_encode_filter_properties",
        lzma_filter_converter, &filter))
        goto exit;
    return_value = _lzma__encode_filter_properties_impl(module, filter);

exit:
    /* Cleanup for filter */
    if (filter.id != LZMA_VLI_UNKNOWN)
       PyMem_Free(filter.options);

    return return_value;
}

PyDoc_STRVAR(_lzma__decode_filter_properties__doc__,
"_decode_filter_properties($module, filter_id, encoded_props, /)\n"
"--\n"
"\n"
"Return a bytes object encoding the options (properties) of the filter specified by *filter* (a dict).\n"
"\n"
"The result does not include the filter ID itself, only the options.");

#define _LZMA__DECODE_FILTER_PROPERTIES_METHODDEF    \
    {"_decode_filter_properties", (PyCFunction)_lzma__decode_filter_properties, METH_VARARGS, _lzma__decode_filter_properties__doc__},

static PyObject *
_lzma__decode_filter_properties_impl(PyModuleDef *module, lzma_vli filter_id, Py_buffer *encoded_props);

static PyObject *
_lzma__decode_filter_properties(PyModuleDef *module, PyObject *args)
{
    PyObject *return_value = NULL;
    lzma_vli filter_id;
    Py_buffer encoded_props = {NULL, NULL};

    if (!PyArg_ParseTuple(args,
        "O&y*:_decode_filter_properties",
        lzma_vli_converter, &filter_id, &encoded_props))
        goto exit;
    return_value = _lzma__decode_filter_properties_impl(module, filter_id, &encoded_props);

exit:
    /* Cleanup for encoded_props */
    if (encoded_props.obj)
       PyBuffer_Release(&encoded_props);

    return return_value;
}
/*[clinic end generated code: output=808fec8216ac712b input=a9049054013a1b77]*/
