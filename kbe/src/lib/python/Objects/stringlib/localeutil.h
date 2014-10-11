/* stringlib: locale related helpers implementation */

#include <locale.h>

#ifndef STRINGLIB_IS_UNICODE
#   error "localeutil is specific to Unicode"
#endif

typedef struct {
    const char *grouping;
    char previous;
    Py_ssize_t i; /* Where we're currently pointing in grouping. */
} STRINGLIB(GroupGenerator);

static void
STRINGLIB(GroupGenerator_init)(STRINGLIB(GroupGenerator) *self, const char *grouping)
{
    self->grouping = grouping;
    self->i = 0;
    self->previous = 0;
}

/* Returns the next grouping, or 0 to signify end. */
static Py_ssize_t
STRINGLIB(GroupGenerator_next)(STRINGLIB(GroupGenerator) *self)
{
    /* Note that we don't really do much error checking here. If a
       grouping string contains just CHAR_MAX, for example, then just
       terminate the generator. That shouldn't happen, but at least we
       fail gracefully. */
    switch (self->grouping[self->i]) {
    case 0:
        return self->previous;
    case CHAR_MAX:
        /* Stop the generator. */
        return 0;
    default: {
        char ch = self->grouping[self->i];
        self->previous = ch;
        self->i++;
        return (Py_ssize_t)ch;
    }
    }
}

/* Fill in some digits, leading zeros, and thousands separator. All
   are optional, depending on when we're called. */
static void
STRINGLIB(fill)(STRINGLIB_CHAR **digits_end, STRINGLIB_CHAR **buffer_end,
     Py_ssize_t n_chars, Py_ssize_t n_zeros, STRINGLIB_CHAR* thousands_sep,
     Py_ssize_t thousands_sep_len)
{
    Py_ssize_t i;

    if (thousands_sep) {
        *buffer_end -= thousands_sep_len;

        /* Copy the thousands_sep chars into the buffer. */
        memcpy(*buffer_end, thousands_sep,
               thousands_sep_len * STRINGLIB_SIZEOF_CHAR);
    }

    *buffer_end -= n_chars;
    *digits_end -= n_chars;
    memcpy(*buffer_end, *digits_end, n_chars * sizeof(STRINGLIB_CHAR));

    *buffer_end -= n_zeros;
    for (i = 0; i < n_zeros; i++)
        (*buffer_end)[i] = '0';
}

/**
 * InsertThousandsGrouping:
 * @buffer: A pointer to the start of a string.
 * @n_buffer: Number of characters in @buffer.
 * @digits: A pointer to the digits we're reading from. If count
 *          is non-NULL, this is unused.
 * @n_digits: The number of digits in the string, in which we want
 *            to put the grouping chars.
 * @min_width: The minimum width of the digits in the output string.
 *             Output will be zero-padded on the left to fill.
 * @grouping: see definition in localeconv().
 * @thousands_sep: see definition in localeconv().
 *
 * There are 2 modes: counting and filling. If @buffer is NULL,
 *  we are in counting mode, else filling mode.
 * If counting, the required buffer size is returned.
 * If filling, we know the buffer will be large enough, so we don't
 *  need to pass in the buffer size.
 * Inserts thousand grouping characters (as defined by grouping and
 *  thousands_sep) into the string between buffer and buffer+n_digits.
 *
 * Return value: 0 on error, else 1.  Note that no error can occur if
 *  count is non-NULL.
 *
 * This name won't be used, the includer of this file should define
 *  it to be the actual function name, based on unicode or string.
 *
 * As closely as possible, this code mimics the logic in decimal.py's
    _insert_thousands_sep().
 **/
static Py_ssize_t
STRINGLIB(InsertThousandsGrouping)(
    STRINGLIB_CHAR *buffer,
    Py_ssize_t n_buffer,
    STRINGLIB_CHAR *digits,
    Py_ssize_t n_digits,
    Py_ssize_t min_width,
    const char *grouping,
    STRINGLIB_CHAR *thousands_sep,
    Py_ssize_t thousands_sep_len)
{
    Py_ssize_t count = 0;
    Py_ssize_t n_zeros;
    int loop_broken = 0;
    int use_separator = 0; /* First time through, don't append the
                              separator. They only go between
                              groups. */
    STRINGLIB_CHAR *buffer_end = NULL;
    STRINGLIB_CHAR *digits_end = NULL;
    Py_ssize_t l;
    Py_ssize_t n_chars;
    Py_ssize_t remaining = n_digits; /* Number of chars remaining to
                                        be looked at */
    /* A generator that returns all of the grouping widths, until it
       returns 0. */
    STRINGLIB(GroupGenerator) groupgen;
    STRINGLIB(GroupGenerator_init)(&groupgen, grouping);

    if (buffer) {
        buffer_end = buffer + n_buffer;
        digits_end = digits + n_digits;
    }

    while ((l = STRINGLIB(GroupGenerator_next)(&groupgen)) > 0) {
        l = Py_MIN(l, Py_MAX(Py_MAX(remaining, min_width), 1));
        n_zeros = Py_MAX(0, l - remaining);
        n_chars = Py_MAX(0, Py_MIN(remaining, l));

        /* Use n_zero zero's and n_chars chars */

        /* Count only, don't do anything. */
        count += (use_separator ? thousands_sep_len : 0) + n_zeros + n_chars;

        if (buffer) {
            /* Copy into the output buffer. */
            STRINGLIB(fill)(&digits_end, &buffer_end, n_chars, n_zeros,
                 use_separator ? thousands_sep : NULL, thousands_sep_len);
        }

        /* Use a separator next time. */
        use_separator = 1;

        remaining -= n_chars;
        min_width -= l;

        if (remaining <= 0 && min_width <= 0) {
            loop_broken = 1;
            break;
        }
        min_width -= thousands_sep_len;
    }
    if (!loop_broken) {
        /* We left the loop without using a break statement. */

        l = Py_MAX(Py_MAX(remaining, min_width), 1);
        n_zeros = Py_MAX(0, l - remaining);
        n_chars = Py_MAX(0, Py_MIN(remaining, l));

        /* Use n_zero zero's and n_chars chars */
        count += (use_separator ? thousands_sep_len : 0) + n_zeros + n_chars;
        if (buffer) {
            /* Copy into the output buffer. */
            STRINGLIB(fill)(&digits_end, &buffer_end, n_chars, n_zeros,
                 use_separator ? thousands_sep : NULL, thousands_sep_len);
        }
    }
    return count;
}

