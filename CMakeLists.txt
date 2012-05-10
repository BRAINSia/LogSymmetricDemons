#-----------------------------------------------------------------------------
# Name of the project
project(LogDomainDemonsRegistration)


#-----------------------------------------------------------------------------
# Requires a recent version of CMake
cmake_minimum_required(VERSION 2.6)


#-----------------------------------------------------------------------------
# Include some useful macros
include(${CMAKE_SOURCE_DIR}/IJMacros.txt)
include(${CMAKE_SOURCE_DIR}/SDCMakeMacros.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})


#-----------------------------------------------------------------------------
# Set up some default paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)


#-----------------------------------------------------------------------------
#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
set(Required_Packages
  ITK
  )


#-----------------------------------------------------------------------------
#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
foreach(Package ${Required_Packages})
  LOAD_REQUIRED_PACKAGE(${Package})
endforeach(Package)


#-----------------------------------------------------------------------------
#Declare any optional external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
set(Optional_Packages
  Matlab
  )


#-----------------------------------------------------------------------------
#this foreach loads all of the packages that you specified as optional.
#It shouldn't need to be modified.
foreach(Package ${Optional_Packages})
  LOAD_OPTIONAL_PACKAGE(${Package})
endforeach(Package)


#-----------------------------------------------------------------------------
#Some packages options are required for this project. Check them now
set(_ITKVersionString "${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}.${ITK_VERSION_PATCH}" )
math(EXPR _ITKVersionNum "${ITK_VERSION_MAJOR}*100*100 + ${ITK_VERSION_MINOR}*100 + ${ITK_VERSION_PATCH}")
if( _ITKVersionNum LESS 30800 )
  message(SEND_ERROR "The ITK version you want to use (${_ITKVersionString}) is not supported by this project. Please use a more recent version of ITK. The minimum required version is 3.8.0. The recommended version is 3.14.0.")
endif( _ITKVersionNum LESS 30800 )


if(ITK_VERSION_MAJOR LESS 4)
  if(NOT ITK_USE_REVIEW)
    message(SEND_ERROR "ITK_USE_REVIEW has not been set to on when compiling ITK. This option is mandatory for this project to build. Please recompile your ITK installation.")
  endif(NOT ITK_USE_REVIEW)
endif()

#-----------------------------------------------------------------------------
#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
set(Libraries ${ITK_LIBRARIES})


#-----------------------------------------------------------------------------
#Set any extra compilation flags here
if(CMAKE_COMPILER_IS_GNUCXX)
  # Add some useful warning flags
  # Note: -Wall and -Wno-deprecated are already pulled in by
  # UseITK.cmake
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wcast-qual")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wcast-qual")
endif(CMAKE_COMPILER_IS_GNUCXX)


#-----------------------------------------------------------------------------
#Build the ITK code if necessary or simply add the corresponding include dirs
include_directories(Code)
add_subdirectory(Code)


#-----------------------------------------------------------------------------
#Build the matlab bindings if we have some
if(MATLAB_FOUND)
  message("Found Matlab. Matlab wrappers (mex files) and unit tests will be built if there are some in the project.")
  if(EXISTS ${CMAKE_SOURCE_DIR}/MatlabCode)
    add_subdirectory(MatlabCode)
  endif(EXISTS ${CMAKE_SOURCE_DIR}/MatlabCode)
endif(MATLAB_FOUND)


#-----------------------------------------------------------------------------
#Build the unit tests / configure dashboard
include(${CMAKE_ROOT}/Modules/Dart.cmake)
mark_as_advanced(TCL_TCLSH DART_ROOT)

set(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
mark_as_advanced(BUILDNAME)

enable_testing()
add_subdirectory(Testing)


#-----------------------------------------------------------------------------
#Build the applications
add_subdirectory(Applications)


#-----------------------------------------------------------------------------
# CPack is only used to generate the source package
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "5")

set(CPACK_SOURCE_IGNORE_FILES
  ${CPACK_SOURCE_IGNORE_FILES}
  "CTestConfig.cmake"
  "/Documents/.*/"
  "/Documents/.*/"
  "/MatlabCode/"
  "README-Matlab\\\\.txt"
  "/TestingData/[^R][^a][^t].*[^mr][^ha][^daw]"
  "/TestingData/.*\\\\.base\\\\.png$"
  "/TestingData/.*\\\\.test\\\\.png$"
  "/TestingData/.*\\\\.diff\\\\.png$"
  "\\\\.nii$"
  "~$"
  )

# reinclude cpack to take into account CPACK_SOURCE_IGNORE_FILES
include(CPack)

