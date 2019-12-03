

macro(SETUP_AND_ADD_TEST TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE} ${PROJECT_SOURCE_DIR}/src/decs.h)
    #set_target_properties(${TEST_NAME} PROPERTIES CXX_EXTENSIONS OFF)
	set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 17)
    target_link_libraries(${TEST_NAME} PRIVATE Decs GTest::Main Threads::Threads)
	target_compile_options(${TEST_NAME} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic -Wall>)
    target_compile_options(${TEST_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

SETUP_AND_ADD_TEST(basic base_tests.cpp)