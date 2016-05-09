find_path(PYTHON_INCLUDE_DIRS NAMES Python.h PATHS /usr/include/python3.2)
find_library(PYTHON_LIBRARY NAMES Python3.2)
#find_library(SNDFILECPP_LIBRARY NAMES sndfilecpp)

set(PYTHON_LIBRARIES ${PYTHON_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(python DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_DIRS)
