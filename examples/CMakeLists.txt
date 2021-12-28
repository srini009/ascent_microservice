add_executable (example-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (example-server ams-server /home/users/sramesh/VIZ_SERVICE/spack/opt/spack/linux-ubuntu18.04-haswell/gcc-7.4.0/conduit-0.7.2-zsjo2lbxzvmq7awr3h663myd4gb7rn4i/lib/libconduit.so /home/users/sramesh/VIZ_SERVICE/spack/opt/spack/linux-ubuntu18.04-haswell/gcc-7.4.0/ascent-0.7.1-rqxfjvkfwtfr6twvfloaxl7kx6jjiqya/lib/libascent.so)

add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (example-admin ams-admin)

add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (example-client ams-client /home/users/sramesh/VIZ_SERVICE/spack/opt/spack/linux-ubuntu18.04-haswell/gcc-7.4.0/conduit-0.7.2-zsjo2lbxzvmq7awr3h663myd4gb7rn4i/lib/libconduit.so /home/users/sramesh/VIZ_SERVICE/spack/opt/spack/linux-ubuntu18.04-haswell/gcc-7.4.0/conduit-0.7.2-zsjo2lbxzvmq7awr3h663myd4gb7rn4i/lib/libconduit_blueprint.so /home/users/sramesh/VIZ_SERVICE/spack/opt/spack/linux-ubuntu18.04-haswell/gcc-7.4.0/ascent-0.7.1-rqxfjvkfwtfr6twvfloaxl7kx6jjiqya/lib/libascent.so)
