set(TEST_TYPE "INTEGRATION")

set(tests
  audio.cc
  cfm_damping_implicit_spring_damper.cc
  disable_fixed_joint_reduction.cc
  fixed_joint_reduction.cc
  force_torque_sensor.cc
  joint_axis_frame.cc
  locale_fix.cc
  parser_error_detection.cc
  plugin_attribute.cc
  plugin_bool.cc
  provide_feedback.cc
  urdf_joint_parameters.cc
)

find_program(XMLLINT_EXE xmllint)
if (EXISTS ${XMLLINT_EXE})
  set (tests ${tests} schema_test.cc)
else()
  BUILD_WARNING("xmllint not found. schema_test won't be run")
endif()

link_directories(${PROJECT_BINARY_DIR}/test)

sdf_build_tests(${tests})

if (EXISTS ${XMLLINT_EXE})
  # Need to run schema target (build .xsd files) before running schema_test
  add_dependencies(${TEST_TYPE}_schema_test schema)
endif()

