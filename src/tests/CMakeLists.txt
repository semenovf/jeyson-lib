project(pfs-json-tests CXX)

add_executable(parser parser.cpp)
add_test(NAME parser COMMAND parser)
