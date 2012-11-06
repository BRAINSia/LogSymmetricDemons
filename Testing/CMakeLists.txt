SD_UNIT_TEST(itkDisplacementFieldCompositionFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkVelocityFieldLieBracketFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkLogDomainDeformableRegistrationFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkExponentialDisplacementFieldImageFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkVelocityFieldBCHCompositionFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkLogDomainDemonsRegistrationFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkLogDomainDemonsRegistrationFilterTest2.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkSymmetricLogDomainDemonsRegistrationFilterTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkSymmetricLogDomainDemonsRegistrationFilterTest2.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkTransformToVelocityFieldSourceTest.cxx EXTLIBS ${Libraries})
SD_UNIT_TEST(itkDisplacementToVelocityFieldLogFilterTest.cxx EXTLIBS ${Libraries})

set_tests_properties( itkLogDomainDemonsRegistrationFilterTest
  itkLogDomainDemonsRegistrationFilterTest2
  PROPERTIES TIMEOUT 6000 )


if(MATLAB_FOUND)
  include_directories(${MATLAB_INCLUDE_DIR})
  SD_UNIT_TEST(vnl_sd_matrix_tools_test.cxx EXTLIBS ${Libraries} MATLAB_ENG MATLAB_MX)
  SD_APPEND_TARGET_PROPERTIES(vnl_sd_matrix_tools_test COMPILE_FLAGS "-DMATLAB_FOUND")
  if(APPLE)
    set_target_properties(vnl_sd_matrix_tools_test PROPERTIES LINK_FLAGS "-L${MATLAB_SYS}")
  endif(APPLE)
else(MATLAB_FOUND)
  SD_UNIT_TEST(vnl_sd_matrix_tools_test.cxx EXTLIBS ${Libraries})
endif(MATLAB_FOUND)


# Also do a unit test that checks whether the registration result is the same
# whether we use pixel spacing and image direction or not
add_test(RegisterImagesLD ${PROJECT_BINARY_DIR}/bin/LogDomainDemonsRegistration
  -f ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice1.mhd
  -m ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice2.mhd
  -o ${PROJECT_BINARY_DIR}/Testing/output-ld.mha
  -d -i 3x3x5 -s 1 -g 1 -e -a 0)

add_test(RegisterImagesLDs ${PROJECT_BINARY_DIR}/bin/LogDomainDemonsRegistration
  -f ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice1s.mhd
  -m ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice2s.mhd
  -o ${PROJECT_BINARY_DIR}/Testing/output-ld-s.mha
  -d -i 3x3x5 -s 1 -g 1 -e -a 0)

add_test(CompareImageLD ${PROJECT_BINARY_DIR}/bin/ImageCompare
   ${PROJECT_BINARY_DIR}/Testing/output-ld.mha ${PROJECT_BINARY_DIR}/Testing/output-ld-s.mha)


add_test(RegisterImagesSLD ${PROJECT_BINARY_DIR}/bin/LogDomainDemonsRegistration
  -f ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice1.mhd
  -m ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice2.mhd
  -o ${PROJECT_BINARY_DIR}/Testing/output-sld.mha
  -d -i 3x3x5 -s 1 -g 1 -e -a 1)

add_test(RegisterImagesSLDs ${PROJECT_BINARY_DIR}/bin/LogDomainDemonsRegistration
  -f ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice1s.mhd
  -m ${CMAKE_SOURCE_DIR}/TestingData/RatLungSlice2s.mhd
  -o ${PROJECT_BINARY_DIR}/Testing/output-sld-s.mha
  -d -i 3x3x5 -s 1 -g 1 -e -a 1)

add_test(CompareImageSLD ${PROJECT_BINARY_DIR}/bin/ImageCompare
  ${PROJECT_BINARY_DIR}/Testing/output-sld.mha ${PROJECT_BINARY_DIR}/Testing/output-sld-s.mha)


# Add a test to check the style of the source code
if ( 0 )
find_program(KWSTYLE_EXECUTABLE
  NAMES KWStyle
  PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware Inc.\\KWStyle 1.0.0]/bin"
  )
mark_as_advanced(KWSTYLE_EXECUTABLE)
if(KWSTYLE_EXECUTABLE)
  add_test(StyleTest ${CMAKE_COMMAND}
    -E chdir ${CMAKE_SOURCE_DIR}; ${KWSTYLE_EXECUTABLE}
    -xml ${CMAKE_SOURCE_DIR}/sd_kwstyle.kws.xml
    -D ${CMAKE_SOURCE_DIR}/sd_kwstyle.txt
    -gcc )
else(KWSTYLE_EXECUTABLE)
  message("Did not find KWStyle - the style test will not be run")
endif(KWSTYLE_EXECUTABLE)
endif()

add_executable(MakeTestImages2D MakeTestImages2D.cxx)
target_link_libraries(MakeTestImages2D ${ITK_LIBRARIES})

add_test(NAME MakeTestImages2D COMMAND  ${PROJECT_BINARY_DIR}/bin/MakeTestImages2D
  ${PROJECT_BINARY_DIR}/Testing/ellipse.png
  ${PROJECT_BINARY_DIR}/Testing/box.png
  ${PROJECT_BINARY_DIR}/Testing/box45.png
  ${PROJECT_BINARY_DIR}/Testing/star.png
  ${PROJECT_BINARY_DIR}/Testing/xorstar.png
  45.0
  )

add_executable(DistanceMaps2D DistanceMaps2D.cxx)
target_link_libraries(DistanceMaps2D ${ITK_LIBRARIES})

add_executable(ConvertSVFToDefAndWarp2D ConvertSVFToDefAndWarp2D.cxx)
target_link_libraries(ConvertSVFToDefAndWarp2D ${ITK_LIBRARIES})

add_executable(MakeTestImages3D MakeTestImages3D.cxx)
target_link_libraries(MakeTestImages3D ${ITK_LIBRARIES})
