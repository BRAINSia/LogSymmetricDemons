# - this module looks for Matlab
# Defines:
#  MATLAB_INCLUDE_DIR: include path for mex.h, engine.h
#  MATLAB_LIBRARIES:   required libraries: libmex, etc
#  MATLAB_MAT_LIBRARY: path to libmat.lib
#  MATLAB_MEX_LIBRARY: path to libmex.lib
#  MATLAB_MX_LIBRARY:  path to libmx.lib
#  MATLAB_ENG_LIBRARY: path to libeng.lib

set(MATLAB_ROOT "" CACHE PATH "Directory containing matlab.")

if(NOT MATLAB_ROOT)
  if(WIN32)
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio .*" OR ${CMAKE_GENERATOR} MATCHES "NMake Makefiles")
      set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/")
    else(${CMAKE_GENERATOR} MATCHES "Visual Studio .*" OR ${CMAKE_GENERATOR} MATCHES "NMake Makefiles")
        if(${CMAKE_GENERATOR} MATCHES "Borland")
          # Same here, there are also: bcc50 and bcc51 directories
          set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/bcc54")
        else(${CMAKE_GENERATOR} MATCHES "Borland")
          message(FATAL_ERROR "Generator not compatible: ${CMAKE_GENERATOR}")
        endif(${CMAKE_GENERATOR} MATCHES "Borland")
    endif(${CMAKE_GENERATOR} MATCHES "Visual Studio .*" OR ${CMAKE_GENERATOR} MATCHES "NMake Makefiles")
  else( WIN32 )
    if(NOT MATLAB_ROOT)
      if($ENV{MATLAB_ROOT})
        set(MATLAB_ROOT $ENV{MATLAB_ROOT})
      else($ENV{MATLAB_ROOT})
        set(MATLAB_ROOT /opt/matlab)
      endif($ENV{MATLAB_ROOT})
    endif(NOT MATLAB_ROOT)
  endif(WIN32)
endif(NOT MATLAB_ROOT)



set(MATLAB_FOUND 0)
if(WIN32)
  find_library(MATLAB_MEX_LIBRARY
    libmex
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_MX_LIBRARY
    libmx
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_ENG_LIBRARY
    libeng
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_MAT_LIBRARY
    libmat
    ${MATLAB_ROOT}
    )

  find_path(MATLAB_INCLUDE_DIR
    "mex.h"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/include"
    )
else( WIN32 )
  if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES MATCHES i386)
      # mac intel
      set(MATLAB_SYS
        ${MATLAB_ROOT}/bin/maci
        )
    else(CMAKE_OSX_ARCHITECTURES MATCHES i386)
      # mac powerpc
      set(MATLAB_SYS
        ${MATLAB_ROOT}/bin/mac
        )
    endif(CMAKE_OSX_ARCHITECTURES MATCHES i386)
  else(APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # Regular x86
      set(MATLAB_SYS
        ${MATLAB_ROOT}/bin/glnx86
        )
    else(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # AMD64:
      set(MATLAB_SYS
        ${MATLAB_ROOT}/bin/glnxa64
        )
    endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  endif(APPLE)

  find_library(MATLAB_MEX_LIBRARY
    mex
    PATHS
    ${MATLAB_SYS}
    NO_DEFAULT_PATH
    )
  find_library(MATLAB_MX_LIBRARY
    mx
    PATHS
    ${MATLAB_SYS}
    NO_DEFAULT_PATH
    )
  find_library(MATLAB_MAT_LIBRARY
    mat
    PATHS
    ${MATLAB_SYS}
    NO_DEFAULT_PATH
    )
  find_library(MATLAB_ENG_LIBRARY
    eng
    PATHS
    ${MATLAB_SYS}
    NO_DEFAULT_PATH
    )
  find_path(MATLAB_INCLUDE_DIR
    "mex.h"
    ${MATLAB_ROOT}/extern/include
    )

endif(WIN32)

# This is common to UNIX and Win32:
set(MATLAB_LIBRARIES
  ${MATLAB_MEX_LIBRARY}
  ${MATLAB_MX_LIBRARY}
  ${MATLAB_ENG_LIBRARY}
)

if(MATLAB_INCLUDE_DIR
    AND MATLAB_MEX_LIBRARY
    AND MATLAB_MAT_LIBRARY
    AND MATLAB_ENG_LIBRARY
    AND MATLAB_MX_LIBRARY)
  set(MATLAB_LIBRARIES ${MATLAB_MX_LIBRARY} ${MATLAB_MEX_LIBRARY} ${MATLAB_ENG_LIBRARY} ${MATLAB_MAT_LIBRARY})
endif(MATLAB_INCLUDE_DIR
    AND MATLAB_MEX_LIBRARY
    AND MATLAB_MAT_LIBRARY
    AND MATLAB_ENG_LIBRARY
    AND MATLAB_MX_LIBRARY)

mark_as_advanced(
  MATLAB_MAT_LIBRARY
  MATLAB_MEX_LIBRARY
  MATLAB_MX_LIBRARY
  MATLAB_ENG_LIBRARY
  MATLAB_INCLUDE_DIR
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Matlab
    MATLAB_INCLUDE_DIR
    MATLAB_MEX_LIBRARY
    MATLAB_MAT_LIBRARY
    MATLAB_ENG_LIBRARY
    MATLAB_MX_LIBRARY )

