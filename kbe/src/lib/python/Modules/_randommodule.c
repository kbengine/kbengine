/* Random objects */

/* ------------------------------------------------------------------
   The code in this module was based on a download from:
      http://www.math.keio.ac.jp/~matumoto/MT2002/emt19937ar.html

   It was modified in 2002 by Raymond Hettinger as follows:

    * the principal computational lines untouched.

    * renamed genrand_res53() to random_random() and wrapped
      in python calling/return code.

    * genrand_int32() and the helper functions, init_genrand()
      and init_by_array(), were declared static, wrapped in
      Python calling/return code.  also, their global data
      references were replaced with structure references.

    * unused functions from the original were deleted.
      new, original C python code was added to implement the
      Random() interface.

   The following are the verbatim comments from the original code:

   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* ---------------------------------------------------------------*/

#include "Python.h"
#include <time.h>               /* for seeding to current time */

/* Period parameters -- These are all magic.  Don't change. */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

typedef struct {
    PyObject_HEAD
    unsigned long state[N];
    int index;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)


/* Random methods */


/* generates a random number on [0,0xffffffff]-interval */
static unsigned long
genrand_int32(RandomObject *self)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    unsigned long *mt;

    mt = self->state;
    if (self->index >= N) { /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        self->index = 0;
    }

    y = mt[self->index++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}

/* random_random is the function named genrand_res53 in the original code;
 * generates a random number on [0,1) with 53-bit resolution; note that
 * 9007199254740992 == 2**53; I assume they're spelling "/2**53" as
 * multiply-by-reciprocal in the (likely vain) hope that the compiler will
 * optimize the division away at compile-time.  67108864 is 2**26.  In
 * effect, a contains 27 random bits shifted left 26, and b fills in the
 * lower 26 bits of the 53-bit numerator.
 * The orginal code credited Isaku Wada for this algorithm, 2002/01/09.
 */
static PyObject *
random_random(RandomObject *self)
{
    unsigned long a=genrand_int32(self)>>5, b=genrand_int32(self)>>6;
    return PyFloat_FromDouble((a*67108864.0+b)*(1.0/9007199254740992.0));
}

/* initializes mt[N] with a seed */
static void
init_genrand(RandomObject *self, unsigned long s)
{
    int mti;
    unsigned long *mt;

    mt = self->state;
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                                */
        /* 2002/01/09 modified by Makoto Matsumoto                     */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
    self->index = mti;
    return;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
static PyObject *
init_by_array(RandomObject *self, unsigned long init_key[], size_t key_length)
{
    size_t i, j, k;       /* was signed in the original code. RDH 12/16/2002 */
    unsigned long *mt;

    mt = self->state;
    init_genrand(self, 19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
                 + init_key[j] + (unsigned long)j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
                 - (unsigned long)i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
    Py_INCREF(Py_None);
    return Py_None;
}

/*
 * The rest is Python-specific code, neither part of, nor derived from, the
 * Twister download.
 */

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *result = NULL;            /* guilty until proved innocent */
    PyObject *n = NULL;
    unsigned long *key = NULL;
    unsigned char *key_as_bytes = NULL;
    size_t bits, keyused, i;
    int res;
    PyObject *arg = NULL;

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (arg == NULL || arg == Py_None) {
        time_t now;

        time(&now);
        init_genrand(self, (unsigned long)now);
        Py_INCREF(Py_None);
        return Py_None;
    }
    /* This algorithm relies on the number being unsigned.
     * So: if the arg is a PyLong, use its absolute value.
     * Otherwise use its hash value, cast to unsigned.
     */
    if (PyLong_Check(arg))
        n = PyNumber_Absolute(arg);
    else {
        Py_hash_t hash = PyObject_Hash(arg);
        if (hash == -1)
            goto Done;
        n = PyLong_FromSize_t((size_t)hash);
    }
    if (n == NULL)
        goto Done;

    /* Now split n into 32-bit chunks, from the right. */
    bits = _PyLong_NumBits(n);
    if (bits == (size_t)-1 && PyErr_Occurred())
        goto Done;

    /* Figure out how many 32-bit chunks this gives us. */
    keyused = bits == 0 ? 1 : (bits - 1) / 32 + 1;

    /* Convert seed to byte sequence. */
    key_as_bytes = (unsigned char *)PyMem_Malloc((size_t)4 * keyused);
    if (key_as_bytes == NULL) {
        PyErr_NoMemory();
        goto Done;
    }
    res = _PyLong_AsByteArray((PyLongObject *)n,
                              key_as_bytes, keyused * 4,
                              1,  /* little-endian */
                              0); /* unsigned */
    if (res == -1) {
        PyMem_Free(key_as_bytes);
        goto Done;
    }

    /* Fill array of unsigned longs from byte sequence. */
    key = (unsigned long *)PyMem_Malloc(sizeof(unsigned long) * keyused);
    if (key == NULL) {
        PyErr_NoMemory();
        PyMem_Free(key_as_bytes);
        goto Done;
    }
    for (i = 0; i < keyused; i++) {
        key[i] =
            ((unsigned long)key_as_bytes[4*i + 0] << 0) +
            ((unsigned long)key_as_bytes[4*i + 1] << 8) +
            ((unsigned long)key_as_bytes[4*i + 2] << 16) +
            ((unsigned long)key_as_bytes[4*i + 3] << 24);
    }
    PyMem_Free(key_as_bytes);
    result = init_by_array(self, key, keyused);
Done:
    Py_XDECREF(n);
    PyMem_Free(key);
    return result;
}

static PyObject *
random_getstate(RandomObject *self)
{
    PyObject *state;
    PyObject *element;
    int i;

    state = PyTuple_New(N+1);
    if (state == NULL)
        return NULL;
    for (i=0; i<N ; i++) {
        element = PyLong_FromUnsignedLong(self->state[i]);
        if (element == NULL)
            goto Fail;
        PyTuple_SET_ITEM(state, i, element);
    }
    element = PyLong_FromLong((long)(self->index));
    if (element == NULL)
        goto Fail;
    PyTuple_SET_ITEM(state, i, element);
    return state;

Fail:
    Py_DECREF(state);
    return NULL;
}

static PyObject *
random_setstate(RandomObject *self, PyObject *state)
{
    int i;
    unsigned long element;
    long index;

    if (!PyTuple_Check(state)) {
        PyErr_SetString(PyExc_TypeError,
            "state vector must be a tuple");
        return NULL;
    }
    if (PyTuple_Size(state) != N+1) {
        PyErr_SetString(PyExc_ValueError,
            "state vector is the wrong size");
        return NULL;
    }

    for (i=0; i<N ; i++) {
        element = PyLong_AsUnsignedLong(PyTuple_GET_ITEM(state, i));
        if (element == (unsigned long)-1 && PyErr_Occurred())
            return NULL;
        self->state[i] = element & 0xffffffffUL; /* Make sure we get sane state */
    }

    index = PyLong_AsLong(PyTuple_GET_ITEM(state, i));
    if (index == -1 && PyErr_Occurred())
        return NULL;
    self->index = (int)index;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
random_getrandbits(RandomObject *self, PyObject *args)
{
    int k, i, bytes;
    unsigned long r;
    unsigned char *bytearray;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "i:getrandbits", &k))
        return NULL;

    if (k <= 0) {
        PyErr_SetString(PyExc_ValueError,
                        "number of bits must be greater than zero");
        return NULL;
    }

    if (k <= 32)  /* Fast path */
        return PyLong_FromUnsignedLong(genrand_int32(self) >> (32 - k));

    bytes = ((k - 1) / 32 + 1) * 4;
    bytearray = (unsigned char *)PyMem_Malloc(bytes);
    if (bytearray == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    /* Fill-out whole words, byte-by-byte to avoid endianness issues */
    for (i=0 ; i<bytes ; i+=4, k-=32) {
        r = genrand_int32(self);
        if (k < 32)
            r >>= (32 - k);
        bytearray[i+0] = (unsigned char)r;
        bytearray[i+1] = (unsigned char)(r >> 8);
        bytearray[i+2] = (unsigned char)(r >> 16);
        bytearray[i+3] = (unsigned char)(r >> 24);
    }

    /* little endian order to match bytearray assignment order */
    result = _PyLong_FromByteArray(bytearray, bytes, 1, 0);
    PyMem_Free(bytearray);
    return result;
}

static PyObject *
random_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RandomObject *self;
    PyObject *tmp;

    if (type == &Random_Type && !_PyArg_NoKeywords("Random()", kwds))
        return NULL;

    self = (RandomObject *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;
    tmp = random_seed(self, args);
    if (tmp == NULL) {
        Py_DECREF(self);
        return NULL;
    }
    Py_DECREF(tmp);
    return (PyObject *)self;
}

static PyMethodDef random_methods[] = {
    {"random",          (PyCFunction)random_random,  METH_NOARGS,
        PyDoc_STR("random() -> x in the interval [0, 1).")},
    {"seed",            (PyCFunction)random_seed,  METH_VARARGS,
        PyDoc_STR("seed([n]) -> None.  Defaults to current time.")},
    {"getstate",        (PyCFunction)random_getstate,  METH_NOARGS,
        PyDoc_STR("getstate() -> tuple containing the current state.")},
    {"setstate",          (PyCFunction)random_setstate,  METH_O,
        PyDoc_STR("setstate(state) -> None.  Restores generator state.")},
    {"getrandbits",     (PyCFunction)random_getrandbits,  METH_VARARGS,
        PyDoc_STR("getrandbits(k) -> x.  Generates an int with "
                  "k random bits.")},
    {NULL,              NULL}           /* sentinel */
};

PyDoc_STRVAR(random_doc,
"Random() -> create a random number generator with its own internal state.");

static PyTypeObject Random_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_random.Random",                   /*tp_name*/
    sizeof(RandomObject),               /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    /* methods */
    0,                                  /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,                                  /*tp_reserved*/
    0,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash*/
    0,                                  /*tp_call*/
    0,                                  /*tp_str*/
    PyObject_GenericGetAttr,            /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /*tp_flags*/
    random_doc,                         /*tp_doc*/
    0,                                  /*tp_traverse*/
    0,                                  /*tp_clear*/
    0,                                  /*tp_richcompare*/
    0,                                  /*tp_weaklistoffset*/
    0,                                  /*tp_iter*/
    0,                                  /*tp_iternext*/
    random_methods,                     /*tp_methods*/
    0,                                  /*tp_members*/
    0,                                  /*tp_getset*/
    0,                                  /*tp_base*/
    0,                                  /*tp_dict*/
    0,                                  /*tp_descr_get*/
    0,                                  /*tp_descr_set*/
    0,                                  /*tp_dictoffset*/
    0,                                  /*tp_init*/
    0,                                  /*tp_alloc*/
    random_new,                         /*tp_new*/
    PyObject_Free,                      /*tp_free*/
    0,                                  /*tp_is_gc*/
};

PyDoc_STRVAR(module_doc,
"Module implements the Mersenne Twister random number generator.");


static struct PyModuleDef _randommodule = {
    PyModuleDef_HEAD_INIT,
    "_random",
    module_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__random(void)
{
    PyObject *m;

    if (PyType_Ready(&Random_Type) < 0)
        return NULL;
    m = PyModule_Create(&_randommodule);
    if (m == NULL)
        return NULL;
    Py_INCREF(&Random_Type);
    PyModule_AddObject(m, "Random", (PyObject *)&Random_Type);
    return m;
}
