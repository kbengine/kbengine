/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(binascii_a2b_uu__doc__,
"a2b_uu($module, data, /)\n"
"--\n"
"\n"
"Decode a line of uuencoded data.");

#define BINASCII_A2B_UU_METHODDEF    \
    {"a2b_uu", (PyCFunction)binascii_a2b_uu, METH_O, binascii_a2b_uu__doc__},

static PyObject *
binascii_a2b_uu_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_a2b_uu(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "O&:a2b_uu", ascii_buffer_converter, &data)) {
        goto exit;
    }
    return_value = binascii_a2b_uu_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(binascii_b2a_uu__doc__,
"b2a_uu($module, data, /, *, backtick=False)\n"
"--\n"
"\n"
"Uuencode line of data.");

#define BINASCII_B2A_UU_METHODDEF    \
    {"b2a_uu", (PyCFunction)binascii_b2a_uu, METH_FASTCALL|METH_KEYWORDS, binascii_b2a_uu__doc__},

static PyObject *
binascii_b2a_uu_impl(PyObject *module, Py_buffer *data, int backtick);

static PyObject *
binascii_b2a_uu(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"", "backtick", NULL};
    static _PyArg_Parser _parser = {"y*|$i:b2a_uu", _keywords, 0};
    Py_buffer data = {NULL, NULL};
    int backtick = 0;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &data, &backtick)) {
        goto exit;
    }
    return_value = binascii_b2a_uu_impl(module, &data, backtick);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_a2b_base64__doc__,
"a2b_base64($module, data, /)\n"
"--\n"
"\n"
"Decode a line of base64 data.");

#define BINASCII_A2B_BASE64_METHODDEF    \
    {"a2b_base64", (PyCFunction)binascii_a2b_base64, METH_O, binascii_a2b_base64__doc__},

static PyObject *
binascii_a2b_base64_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_a2b_base64(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "O&:a2b_base64", ascii_buffer_converter, &data)) {
        goto exit;
    }
    return_value = binascii_a2b_base64_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(binascii_b2a_base64__doc__,
"b2a_base64($module, data, /, *, newline=True)\n"
"--\n"
"\n"
"Base64-code line of data.");

#define BINASCII_B2A_BASE64_METHODDEF    \
    {"b2a_base64", (PyCFunction)binascii_b2a_base64, METH_FASTCALL|METH_KEYWORDS, binascii_b2a_base64__doc__},

static PyObject *
binascii_b2a_base64_impl(PyObject *module, Py_buffer *data, int newline);

static PyObject *
binascii_b2a_base64(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"", "newline", NULL};
    static _PyArg_Parser _parser = {"y*|$i:b2a_base64", _keywords, 0};
    Py_buffer data = {NULL, NULL};
    int newline = 1;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &data, &newline)) {
        goto exit;
    }
    return_value = binascii_b2a_base64_impl(module, &data, newline);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_a2b_hqx__doc__,
"a2b_hqx($module, data, /)\n"
"--\n"
"\n"
"Decode .hqx coding.");

#define BINASCII_A2B_HQX_METHODDEF    \
    {"a2b_hqx", (PyCFunction)binascii_a2b_hqx, METH_O, binascii_a2b_hqx__doc__},

static PyObject *
binascii_a2b_hqx_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_a2b_hqx(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "O&:a2b_hqx", ascii_buffer_converter, &data)) {
        goto exit;
    }
    return_value = binascii_a2b_hqx_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(binascii_rlecode_hqx__doc__,
"rlecode_hqx($module, data, /)\n"
"--\n"
"\n"
"Binhex RLE-code binary data.");

#define BINASCII_RLECODE_HQX_METHODDEF    \
    {"rlecode_hqx", (PyCFunction)binascii_rlecode_hqx, METH_O, binascii_rlecode_hqx__doc__},

static PyObject *
binascii_rlecode_hqx_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_rlecode_hqx(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:rlecode_hqx", &data)) {
        goto exit;
    }
    return_value = binascii_rlecode_hqx_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_b2a_hqx__doc__,
"b2a_hqx($module, data, /)\n"
"--\n"
"\n"
"Encode .hqx data.");

#define BINASCII_B2A_HQX_METHODDEF    \
    {"b2a_hqx", (PyCFunction)binascii_b2a_hqx, METH_O, binascii_b2a_hqx__doc__},

static PyObject *
binascii_b2a_hqx_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_b2a_hqx(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:b2a_hqx", &data)) {
        goto exit;
    }
    return_value = binascii_b2a_hqx_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_rledecode_hqx__doc__,
"rledecode_hqx($module, data, /)\n"
"--\n"
"\n"
"Decode hexbin RLE-coded string.");

#define BINASCII_RLEDECODE_HQX_METHODDEF    \
    {"rledecode_hqx", (PyCFunction)binascii_rledecode_hqx, METH_O, binascii_rledecode_hqx__doc__},

static PyObject *
binascii_rledecode_hqx_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_rledecode_hqx(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:rledecode_hqx", &data)) {
        goto exit;
    }
    return_value = binascii_rledecode_hqx_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_crc_hqx__doc__,
"crc_hqx($module, data, crc, /)\n"
"--\n"
"\n"
"Compute CRC-CCITT incrementally.");

#define BINASCII_CRC_HQX_METHODDEF    \
    {"crc_hqx", (PyCFunction)binascii_crc_hqx, METH_FASTCALL, binascii_crc_hqx__doc__},

static unsigned int
binascii_crc_hqx_impl(PyObject *module, Py_buffer *data, unsigned int crc);

static PyObject *
binascii_crc_hqx(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};
    unsigned int crc;
    unsigned int _return_value;

    if (!_PyArg_ParseStack(args, nargs, "y*I:crc_hqx",
        &data, &crc)) {
        goto exit;
    }
    _return_value = binascii_crc_hqx_impl(module, &data, crc);
    if ((_return_value == (unsigned int)-1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromUnsignedLong((unsigned long)_return_value);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_crc32__doc__,
"crc32($module, data, crc=0, /)\n"
"--\n"
"\n"
"Compute CRC-32 incrementally.");

#define BINASCII_CRC32_METHODDEF    \
    {"crc32", (PyCFunction)binascii_crc32, METH_FASTCALL, binascii_crc32__doc__},

static unsigned int
binascii_crc32_impl(PyObject *module, Py_buffer *data, unsigned int crc);

static PyObject *
binascii_crc32(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};
    unsigned int crc = 0;
    unsigned int _return_value;

    if (!_PyArg_ParseStack(args, nargs, "y*|I:crc32",
        &data, &crc)) {
        goto exit;
    }
    _return_value = binascii_crc32_impl(module, &data, crc);
    if ((_return_value == (unsigned int)-1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromUnsignedLong((unsigned long)_return_value);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_b2a_hex__doc__,
"b2a_hex($module, data, /)\n"
"--\n"
"\n"
"Hexadecimal representation of binary data.\n"
"\n"
"The return value is a bytes object.  This function is also\n"
"available as \"hexlify()\".");

#define BINASCII_B2A_HEX_METHODDEF    \
    {"b2a_hex", (PyCFunction)binascii_b2a_hex, METH_O, binascii_b2a_hex__doc__},

static PyObject *
binascii_b2a_hex_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_b2a_hex(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:b2a_hex", &data)) {
        goto exit;
    }
    return_value = binascii_b2a_hex_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_hexlify__doc__,
"hexlify($module, data, /)\n"
"--\n"
"\n"
"Hexadecimal representation of binary data.\n"
"\n"
"The return value is a bytes object.");

#define BINASCII_HEXLIFY_METHODDEF    \
    {"hexlify", (PyCFunction)binascii_hexlify, METH_O, binascii_hexlify__doc__},

static PyObject *
binascii_hexlify_impl(PyObject *module, Py_buffer *data);

static PyObject *
binascii_hexlify(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:hexlify", &data)) {
        goto exit;
    }
    return_value = binascii_hexlify_impl(module, &data);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(binascii_a2b_hex__doc__,
"a2b_hex($module, hexstr, /)\n"
"--\n"
"\n"
"Binary data of hexadecimal representation.\n"
"\n"
"hexstr must contain an even number of hex digits (upper or lower case).\n"
"This function is also available as \"unhexlify()\".");

#define BINASCII_A2B_HEX_METHODDEF    \
    {"a2b_hex", (PyCFunction)binascii_a2b_hex, METH_O, binascii_a2b_hex__doc__},

static PyObject *
binascii_a2b_hex_impl(PyObject *module, Py_buffer *hexstr);

static PyObject *
binascii_a2b_hex(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer hexstr = {NULL, NULL};

    if (!PyArg_Parse(arg, "O&:a2b_hex", ascii_buffer_converter, &hexstr)) {
        goto exit;
    }
    return_value = binascii_a2b_hex_impl(module, &hexstr);

exit:
    /* Cleanup for hexstr */
    if (hexstr.obj)
       PyBuffer_Release(&hexstr);

    return return_value;
}

PyDoc_STRVAR(binascii_unhexlify__doc__,
"unhexlify($module, hexstr, /)\n"
"--\n"
"\n"
"Binary data of hexadecimal representation.\n"
"\n"
"hexstr must contain an even number of hex digits (upper or lower case).");

#define BINASCII_UNHEXLIFY_METHODDEF    \
    {"unhexlify", (PyCFunction)binascii_unhexlify, METH_O, binascii_unhexlify__doc__},

static PyObject *
binascii_unhexlify_impl(PyObject *module, Py_buffer *hexstr);

static PyObject *
binascii_unhexlify(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer hexstr = {NULL, NULL};

    if (!PyArg_Parse(arg, "O&:unhexlify", ascii_buffer_converter, &hexstr)) {
        goto exit;
    }
    return_value = binascii_unhexlify_impl(module, &hexstr);

exit:
    /* Cleanup for hexstr */
    if (hexstr.obj)
       PyBuffer_Release(&hexstr);

    return return_value;
}

PyDoc_STRVAR(binascii_a2b_qp__doc__,
"a2b_qp($module, /, data, header=False)\n"
"--\n"
"\n"
"Decode a string of qp-encoded data.");

#define BINASCII_A2B_QP_METHODDEF    \
    {"a2b_qp", (PyCFunction)binascii_a2b_qp, METH_FASTCALL|METH_KEYWORDS, binascii_a2b_qp__doc__},

static PyObject *
binascii_a2b_qp_impl(PyObject *module, Py_buffer *data, int header);

static PyObject *
binascii_a2b_qp(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"data", "header", NULL};
    static _PyArg_Parser _parser = {"O&|i:a2b_qp", _keywords, 0};
    Py_buffer data = {NULL, NULL};
    int header = 0;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        ascii_buffer_converter, &data, &header)) {
        goto exit;
    }
    return_value = binascii_a2b_qp_impl(module, &data, header);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(binascii_b2a_qp__doc__,
"b2a_qp($module, /, data, quotetabs=False, istext=True, header=False)\n"
"--\n"
"\n"
"Encode a string using quoted-printable encoding.\n"
"\n"
"On encoding, when istext is set, newlines are not encoded, and white\n"
"space at end of lines is.  When istext is not set, \\r and \\n (CR/LF)\n"
"are both encoded.  When quotetabs is set, space and tabs are encoded.");

#define BINASCII_B2A_QP_METHODDEF    \
    {"b2a_qp", (PyCFunction)binascii_b2a_qp, METH_FASTCALL|METH_KEYWORDS, binascii_b2a_qp__doc__},

static PyObject *
binascii_b2a_qp_impl(PyObject *module, Py_buffer *data, int quotetabs,
                     int istext, int header);

static PyObject *
binascii_b2a_qp(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"data", "quotetabs", "istext", "header", NULL};
    static _PyArg_Parser _parser = {"y*|iii:b2a_qp", _keywords, 0};
    Py_buffer data = {NULL, NULL};
    int quotetabs = 0;
    int istext = 1;
    int header = 0;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &data, &quotetabs, &istext, &header)) {
        goto exit;
    }
    return_value = binascii_b2a_qp_impl(module, &data, quotetabs, istext, header);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}
/*[clinic end generated code: output=815f1c453fd6568f input=a9049054013a1b77]*/
