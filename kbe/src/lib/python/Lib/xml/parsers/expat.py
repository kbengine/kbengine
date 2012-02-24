"""Interface to the Expat non-validating XML parser."""
__version__ = '$Revision: 85528 $'

import sys

from pyexpat import *

# provide pyexpat submodules as xml.parsers.expat submodules
sys.modules['xml.parsers.expat.model'] = model
sys.modules['xml.parsers.expat.errors'] = errors
