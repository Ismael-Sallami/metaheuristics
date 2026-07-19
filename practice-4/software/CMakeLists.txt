cmake_minimum_required(VERSION 3.0)
PROJECT(cec17)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Las fuentes C/C++ del benchmark y de los algoritmos viven en src/
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

ADD_EXECUTABLE(test          "src/test.cc")
ADD_EXECUTABLE(testrandom    "src/testrandom.cc")
ADD_EXECUTABLE(testsolis     "src/testsolis.cc")
ADD_EXECUTABLE(testlca       "src/testlca.cc")
ADD_EXECUTABLE(testlcasw     "src/testlcasw.cc")
ADD_EXECUTABLE(testlcachc    "src/testlcachc.cc")
ADD_EXECUTABLE(testlcamulti  "src/testlcamulti.cc")
ADD_EXECUTABLE(testde        "src/testde.cc")
ADD_EXECUTABLE(testpso       "src/testpso.cc")

ADD_LIBRARY("cec17_test_func" SHARED "src/cec17_test_func.c" "src/cec17.c")

TARGET_LINK_LIBRARIES(test          "cec17_test_func")
TARGET_LINK_LIBRARIES(testrandom    "cec17_test_func")
TARGET_LINK_LIBRARIES(testsolis     "cec17_test_func")
TARGET_LINK_LIBRARIES(testlca       "cec17_test_func")
TARGET_LINK_LIBRARIES(testlcasw     "cec17_test_func")
TARGET_LINK_LIBRARIES(testlcachc    "cec17_test_func")
TARGET_LINK_LIBRARIES(testlcamulti  "cec17_test_func")
TARGET_LINK_LIBRARIES(testde        "cec17_test_func")
TARGET_LINK_LIBRARIES(testpso       "cec17_test_func")
