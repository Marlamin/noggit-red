# CMake generated Testfile for 
# Source directory: /Volumes/APFS/NoggitQT
# Build directory: /Volumes/APFS/NoggitQT/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(math-vector_2d "/Volumes/APFS/NoggitQT/cmake-build-debug/bin/math-vector_2d.test")
set_tests_properties(math-vector_2d PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/APFS/NoggitQT/CMakeLists.txt;556;add_test;/Volumes/APFS/NoggitQT/CMakeLists.txt;0;")
add_test(math-trig "/Volumes/APFS/NoggitQT/cmake-build-debug/bin/math-trig.test")
set_tests_properties(math-trig PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/APFS/NoggitQT/CMakeLists.txt;561;add_test;/Volumes/APFS/NoggitQT/CMakeLists.txt;0;")
add_test(math-matrix_4x4 "/Volumes/APFS/NoggitQT/cmake-build-debug/bin/math-matrix_4x4.test")
set_tests_properties(math-matrix_4x4 PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/APFS/NoggitQT/CMakeLists.txt;566;add_test;/Volumes/APFS/NoggitQT/CMakeLists.txt;0;")
subdirs("src/external/qt-color-widgets")
