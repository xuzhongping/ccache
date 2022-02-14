# CMake generated Testfile for 
# Source directory: /Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest
# Build directory: /Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(unittest "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/Debug/unittest")
  set_tests_properties(unittest PROPERTIES  _BACKTRACE_TRIPLES "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;51;add_test;/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(unittest "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/Release/unittest")
  set_tests_properties(unittest PROPERTIES  _BACKTRACE_TRIPLES "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;51;add_test;/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(unittest "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/MinSizeRel/unittest")
  set_tests_properties(unittest PROPERTIES  _BACKTRACE_TRIPLES "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;51;add_test;/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(unittest "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/RelWithDebInfo/unittest")
  set_tests_properties(unittest PROPERTIES  _BACKTRACE_TRIPLES "/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;51;add_test;/Users/xuzhongping/Desktop/dev/myGithub/ccache/unittest/CMakeLists.txt;0;")
else()
  add_test(unittest NOT_AVAILABLE)
endif()
