add_executable (example-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (example-server ams-server ams-admin -lconduit -lascent_mpi)

add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (example-admin ams-admin)

add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (example-client ams-client -lconduit -lconduit_blueprint -lascent_mpi)
