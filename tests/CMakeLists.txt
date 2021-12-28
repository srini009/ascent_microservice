include_directories(../include)

add_library(ams-test Main.cpp)
target_link_libraries(ams-test PkgConfig::cppunit ams-server ams-admin ams-client)

add_executable(AdminTest AdminTest.cpp)
target_link_libraries(AdminTest ams-test)

add_executable(ClientTest AdminTest.cpp)
target_link_libraries(ClientTest ams-test)

add_executable(NodeTest NodeTest.cpp)
target_link_libraries(NodeTest ams-test)

add_test(NAME AdminTest COMMAND ./AdminTest AdminTest.xml)
add_test(NAME ClientTest COMMAND ./ClientTest ClientTest.xml)
add_test(NAME NodeTest COMMAND ./NodeTest NodeTest.xml)
