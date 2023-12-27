set (BASE_SRC_PREFIX ${CMAKE_SOURCE_DIR}/base)
set (BASE_SRC 
  ${BASE_SRC_PREFIX}/timer-wheel.cc
  ${BASE_SRC_PREFIX}/error.cc
  ${BASE_SRC_PREFIX}/json/basic.cc
  ${BASE_SRC_PREFIX}/json/value.cc
)

if(BUILD_TESTS)
  set(ld_libs base fmt)

  add_tc(NAME "${BASE_SRC_PREFIX}/error-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/strings-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/result-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/utf8-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/json/value-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/json/reflection-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/json/json-writer-test.cc" LIBS ${ld_libs})
  add_tc(NAME "${BASE_SRC_PREFIX}/json/json-reader-test.cc" LIBS ${ld_libs})
endif()
