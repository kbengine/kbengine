import os

__revision__ = "$Id: debug.py 57699 2007-08-30 03:52:21Z collin.winter $"

# If DISTUTILS_DEBUG is anything other than the empty string, we run in
# debug mode.
DEBUG = os.environ.get('DISTUTILS_DEBUG')
