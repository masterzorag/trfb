CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(tinyrfb)

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

CHECK_C_COMPILER_FLAG("-W -Wall" CWARNS)
IF(CWARNS)
	ADD_COMPILE_OPTIONS(-W -Wall)
ENDIF()

CHECK_C_COMPILER_FLAG("-Wno-unused-paramete" CWARNS_E)
IF(CWARNS_E)
	ADD_COMPILE_OPTIONS("-Wno-unused-parameter")
ENDIF()

CHECK_INCLUDE_FILES(threads.h HAVE_THREADS_H)
IF(HAVE_THREADS_H)
	ADD_DEFINITIONS(-DHAVE_THREADS_H=1)
ELSE()
	CHECK_LIBRARY_EXISTS(pthread pthread_create "pthread.h" HAVE_LIBPTHREAD)
	IF(HAVE_LIBPTHREAD)
		SET(RFBLIBS "${RFBLIBS} pthread")
	ENDIF()
ENDIF()

CHECK_LIBRARY_EXISTS(v4l2 v4l2_open "libv4l2.h" HAVE_LIBV4L2)
IF(HAVE_LIBV4L2)
	ADD_DEFINITIONS(-DHAVE_LIBV4L2=1)
	SET(LIBWEBCAM_LIBS v4l2)
ELSE()
	SET(LIBWEBCAM_LIBS ${CMAKE_DL_LIBS})
ENDIF()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(libwebcam)
ADD_SUBDIRECTORY(rfbcam)

