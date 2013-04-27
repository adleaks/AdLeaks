# taken from http://code.google.com/p/openjpeg/source/browse/branches/openjpeg-1.5/CMake/FindFCGI.cmake?r=865
# Look for the header file.
FIND_PATH(FCGI_INCLUDE_DIR NAMES fastcgi.h)


# Look for the library.
FIND_LIBRARY(FCGI_LIBRARY NAMES fcgi)


# Handle the QUIETLY and REQUIRED arguments and set FCGI_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FCGI DEFAULT_MSG FCGI_LIBRARY FCGI_INCLUDE_DIR)


# Copy the results to the output variables.
IF(FCGI_FOUND)
	SET( ${FCGI_LIBRARY})
	SET(FCGI_INCLUDE_DIRS ${FCGI_INCLUDE_DIR})
ELSE(FCGI_FOUND)
	SET(FCGI_LIBRARIES)
	SET(FCGI_INCLUDE_DIRS)
ENDIF(FCGI_FOUND)


MARK_AS_ADVANCED(FCGI_INCLUDE_DIRS FCGI_LIBRARIES)
