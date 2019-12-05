project(pfs-json-tests CXX)

add_executable(json json.cpp)
add_test(NAME json COMMAND json)

add_executable(parser parser.cpp)
add_test(NAME parser COMMAND parser)
