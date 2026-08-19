# CMake generated Testfile for 
# Source directory: D:/Embedded/Project/Freelance
# Build directory: D:/Embedded/Project/Freelance/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[gom850_host]=] "D:/Embedded/Project/Freelance/build/Debug/gom850_host.exe")
  set_tests_properties([=[gom850_host]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Embedded/Project/Freelance/CMakeLists.txt;21;add_test;D:/Embedded/Project/Freelance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[gom850_host]=] "D:/Embedded/Project/Freelance/build/Release/gom850_host.exe")
  set_tests_properties([=[gom850_host]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Embedded/Project/Freelance/CMakeLists.txt;21;add_test;D:/Embedded/Project/Freelance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[gom850_host]=] "D:/Embedded/Project/Freelance/build/MinSizeRel/gom850_host.exe")
  set_tests_properties([=[gom850_host]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Embedded/Project/Freelance/CMakeLists.txt;21;add_test;D:/Embedded/Project/Freelance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[gom850_host]=] "D:/Embedded/Project/Freelance/build/RelWithDebInfo/gom850_host.exe")
  set_tests_properties([=[gom850_host]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Embedded/Project/Freelance/CMakeLists.txt;21;add_test;D:/Embedded/Project/Freelance/CMakeLists.txt;0;")
else()
  add_test([=[gom850_host]=] NOT_AVAILABLE)
endif()
