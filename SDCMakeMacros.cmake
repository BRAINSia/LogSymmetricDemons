macro(LOAD_REQUIRED_PACKAGE Package)
  LOADPACKAGE(${Package})
  if(NOT ${Package}_FOUND)
    message(FATAL_ERROR "Required package ${Package} was not found.\n
    Look at Find${Package}.cmake in the CMake module directory for clues
    on what you're supposed to do to help find this package.  Good luck.\n")
  endif(NOT ${Package}_FOUND)
endmacro(LOAD_REQUIRED_PACKAGE)

macro(LOAD_OPTIONAL_PACKAGE Package)
  LOADPACKAGE(${Package} QUIET)
endmacro(LOAD_OPTIONAL_PACKAGE)

macro(ADD_MEX_FILE Target)
  include_directories(${MATLAB_INCLUDE_DIR})
  add_library(${Target} SHARED ${ARGN})
  target_link_libraries(${Target}
    ${MATLAB_MX_LIBRARY}
    ${MATLAB_MEX_LIBRARY}
    ${MATLAB_MAT_LIBRARY}
    m
    )

  set_target_properties(${Target} PROPERTIES PREFIX "")

  # Determine mex suffix
  if(UNIX)
    if(APPLE)
      if(CMAKE_OSX_ARCHITECTURES MATCHES i386)
        set_target_properties(${Target} PROPERTIES SUFFIX ".mexmaci")
      else(CMAKE_OSX_ARCHITECTURES MATCHES i386)
        set_target_properties(${Target} PROPERTIES SUFFIX ".mexmac")
      endif(CMAKE_OSX_ARCHITECTURES MATCHES i386)
    else(APPLE)
      if(CMAKE_SIZEOF_VOID_P MATCHES "4")
        set_target_properties(${Target} PROPERTIES SUFFIX ".mexglx")
      elseif(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set_target_properties(${Target} PROPERTIES SUFFIX ".mexa64")
      else(CMAKE_SIZEOF_VOID_P MATCHES "4")
        message(FATAL_ERROR
          "CMAKE_SIZEOF_VOID_P (${CMAKE_SIZEOF_VOID_P}) doesn't indicate a valid platform")
      endif(CMAKE_SIZEOF_VOID_P MATCHES "4")
    endif(APPLE)
  elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P MATCHES "4")
      set_target_properties(${Target} PROPERTIES SUFFIX ".mexw32")
    elseif(CMAKE_SIZEOF_VOID_P MATCHES "8")
      set_target_properties(${Target} PROPERTIES SUFFIX ".mexw64")
    else(CMAKE_SIZEOF_VOID_P MATCHES "4")
      message(FATAL_ERROR
        "CMAKE_SIZEOF_VOID_P (${CMAKE_SIZEOF_VOID_P}) doesn't indicate a valid platform")
    endif(CMAKE_SIZEOF_VOID_P MATCHES "4")
  endif(UNIX)

  if(MSVC)
    set(MATLAB_FLAGS "-DMATLAB_MEX_FILE")
    SD_APPEND_TARGET_PROPERTIES(${Target} COMPILE_FLAGS ${MATLAB_FLAGS})
    set_target_properties(${Target} PROPERTIES LINK_FLAGS "/export:mexFunction")
  else(MSVC)
    set(MATLAB_FLAGS "-fPIC" "-D_GNU_SOURCE" "-pthread" "-D_FILE_OFFSET_BITS=64" "-DMX_COMPAT_32")
    SD_APPEND_TARGET_PROPERTIES(${Target} COMPILE_FLAGS ${MATLAB_FLAGS})

    if(APPLE)
      if(CMAKE_OSX_ARCHITECTURES MATCHES i386)
        # mac intel
        set_target_properties(${Target} PROPERTIES
          LINK_FLAGS "-L${MATLAB_SYS} -Wl,-flat_namespace -undefined suppress")
      else(CMAKE_OSX_ARCHITECTURES MATCHES i386)
        # mac powerpc?
        set_target_properties(${Target} PROPERTIES
          LINK_FLAGS "-L${MATLAB_SYS} -Wl,-flat_namespace -undefined suppress")
      endif(CMAKE_OSX_ARCHITECTURES MATCHES i386)
    else(APPLE)
      if(CMAKE_SIZEOF_VOID_P MATCHES "4")
        set_target_properties(${Target} PROPERTIES
          LINK_FLAGS "-Wl,-E -Wl,--no-undefined")
      elseif(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set_target_properties(${Target} PROPERTIES
          LINK_FLAGS "-Wl,-E -Wl,--no-undefined")
      else(CMAKE_SIZEOF_VOID_P MATCHES "4")
        message(FATAL_ERROR
          "CMAKE_SIZEOF_VOID_P (${CMAKE_SIZEOF_VOID_P}) doesn't indicate a valid platform")
      endif(CMAKE_SIZEOF_VOID_P MATCHES "4")
    endif(APPLE)
  endif(MSVC)
endmacro(ADD_MEX_FILE)


macro(SD_APPEND_TARGET_PROPERTIES TARGET_TO_CHANGE PROP_TO_CHANGE)
  foreach(_newProp ${ARGN})
    get_target_property(_oldProps ${TARGET_TO_CHANGE} ${PROP_TO_CHANGE})
    if(_oldProps)
      if(NOT "${_oldProps}" MATCHES "^.*${_newProp}.*$")
        set_target_properties(${TARGET_TO_CHANGE} PROPERTIES ${PROP_TO_CHANGE} "${_newProp} ${_oldProps}")
      endif(NOT "${_oldProps}" MATCHES "^.*${_newProp}.*$")
    else(_oldProps)
      set_target_properties(${TARGET_TO_CHANGE} PROPERTIES ${PROP_TO_CHANGE} ${_newProp})
    endif(_oldProps)
  endforeach(_newProp ${ARGN})
endmacro(SD_APPEND_TARGET_PROPERTIES TARGET_TO_CHANGE PROP_TO_CHANGE)


macro(SD_ADD_LINK_LIBRARIES Target)
  foreach(currentLib ${ARGN})
    if(${currentLib}_LIBRARIES)
      target_link_libraries(${Target} ${${currentLib}_LIBRARIES})
    elseif(${currentLib}_LIBRARY)
      target_link_libraries(${Target} ${${currentLib}_LIBRARY})
    else(${currentLib}_LIBRARIES)
      #message("WARNING: ${currentLib}_LIBRARY and ${currentLib}_LIBRARIES are undefined. Using ${currentLib} in linker")
      target_link_libraries(${Target} ${currentLib})
    endif(${currentLib}_LIBRARIES)

    if(${currentLib}_INCLUDE_DIRS)
      include_directories(${${currentLib}_INCLUDE_DIRS})
    elseif(${currentLib}_INCLUDE_DIR)
      include_directories(${${currentLib}_INCLUDE_DIR})
    else(${currentLib}_INCLUDE_DIRS)
      #message("WARNING: ${currentLib}_INCLUDE_DIR and ${currentLib}_INCLUDE_DIR are undefined. No specific include dir will be used for ${currentLib}")
    endif(${currentLib}_INCLUDE_DIRS)
  endforeach(currentLib)
endmacro(SD_ADD_LINK_LIBRARIES)


macro(SD_UNIT_TEST Src)
  # remove extension
  string(REGEX REPLACE "[.][^.]*$" "" Target ${Src})

  # parse arguments
  set(currentPos "")
  set(testLibs "")
  set(testExtLibs "")
  set(testArgs "")

  foreach(arg ${ARGN})
    if(arg STREQUAL "LIBS")
      set(currentPos "LIBS")
    elseif(arg STREQUAL "EXTLIBS")
      set(currentPos "EXTLIBS")
    elseif(arg STREQUAL "ARGS")
      set(currentPos "ARGS")
    else(arg STREQUAL "LIBS")
      if(currentPos STREQUAL "LIBS")
        set(testLibs ${testLibs} ${arg})
      elseif(currentPos STREQUAL "EXTLIBS")
        set(testExtLibs ${testExtLibs} ${arg})
      elseif(currentPos STREQUAL "ARGS")
        set(testArgs ${testArgs} ${arg})
      else(currentPos STREQUAL "ARGS")
         message(FATAL_ERROR "Unknown argument")
      endif(currentPos STREQUAL "LIBS")
    endif(arg STREQUAL "LIBS")
  endforeach(arg ${ARGN})

  # setup target
  add_executable(${Target} ${Src})
  SD_ADD_LINK_LIBRARIES(${Target} ${testExtLibs})
  target_link_libraries(${Target} ${testLibs})
  set_target_properties(${Target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
  add_test(${Target} ${PROJECT_BINARY_DIR}/tests/${Target} ${testArgs})
endmacro(SD_UNIT_TEST)


macro(SD_EXECUTABLE Src)
  # remove extension
  string(REGEX REPLACE "[.][^.]*$" "" Target ${Src})

  # parse arguments
  set(currentPos "")
  set(appLibs "")
  set(appExtLibs "")

  foreach(arg ${ARGN})
    if(arg STREQUAL "LIBS")
      set(currentPos "LIBS")
    elseif(arg STREQUAL "EXTLIBS")
      set(currentPos "EXTLIBS")
    else(arg STREQUAL "LIBS")
      if(currentPos STREQUAL "LIBS")
        set(appLibs ${appLibs} ${arg})
      elseif(currentPos STREQUAL "EXTLIBS")
        set(appExtLibs ${appExtLibs} ${arg})
      else(currentPos STREQUAL "LIBS")
         message(FATAL_ERROR "Unknown argument")
      endif(currentPos STREQUAL "LIBS")
    endif(arg STREQUAL "LIBS")
  endforeach(arg ${ARGN})

  # setup target
  add_executable(${Target} ${Src})
  SD_ADD_LINK_LIBRARIES(${Target} ${appExtLibs})
  target_link_libraries(${Target} ${appLibs})
  set_target_properties(${Target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endmacro(SD_EXECUTABLE)
