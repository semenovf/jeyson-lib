project(pfs-json-tests CXX)

add_executable(factorial factorial.cpp)
add_test(NAME factorial COMMAND factorial)

add_executable(parser parser.cpp)
add_test(NAME parser COMMAND parser)
