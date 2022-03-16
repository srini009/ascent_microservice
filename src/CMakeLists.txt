# set source files
set (server-src-files
     Provider.cpp
     Backend.cpp)

set (client-src-files
     Client.cpp
     NodeHandle.cpp
     AsyncRequest.cpp)

set (admin-src-files
     Admin.cpp)

set (dummy-src-files
     dummy/DummyBackend.cpp)

set (module-src-files
     BedrockModule.cpp)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (ams-pkg "share/cmake/ams")

# set version fo shared library
set (ams-vers "${AMS_VERSION_MAJOR}.${AMS_VERSION_MINOR}")

# server library
add_library (ams-server ${server-src-files} ${dummy-src-files})
target_link_libraries (ams-server
    thallium
    PkgConfig::ABTIO
    PkgConfig::UUID
    nlohmann_json::nlohmann_json
    PkgConfig::SYMBIOMON-CLIENT
    PkgConfig::SYMBIOMON-SERVER)
target_include_directories (ams-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (ams-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (ams-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (ams-server
    PROPERTIES VERSION ${AMS_VERSION}
    SOVERSION ${AMS_VERSION_MAJOR})

# client library
add_library (ams-client ${client-src-files})
target_link_libraries (ams-client thallium PkgConfig::UUID)
target_include_directories (ams-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (ams-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (ams-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (ams-client
    PROPERTIES VERSION ${AMS_VERSION}
    SOVERSION ${AMS_VERSION_MAJOR})

# admin library
add_library (ams-admin ${admin-src-files})
target_link_libraries (ams-admin thallium PkgConfig::UUID nlohmann_json::nlohmann_json)
target_include_directories (ams-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (ams-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (ams-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (ams-admin
    PROPERTIES VERSION ${AMS_VERSION}
    SOVERSION ${AMS_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock module library
add_library (ams-bedrock-module ${module-src-files})
target_link_libraries (ams-bedrock-module ams-server ams-admin ams-client)
target_include_directories (ams-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (ams-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (ams-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (ams-bedrock-module
    PROPERTIES VERSION ${AMS_VERSION}
    SOVERSION ${AMS_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "ams-config-version.cmake"
    VERSION ${AMS_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(ams-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ams-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/ams/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lams-server")
set (CLIENT_PRIVATE_LIBS "-lams-client")
set (ADMIN_PRIVATE_LIBS  "-lams-admin")
configure_file ("ams-server.pc.in" "ams-server.pc" @ONLY)
configure_file ("ams-client.pc.in" "ams-client.pc" @ONLY)
configure_file ("ams-admin.pc.in"  "ams-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY) 

# "make install" rules
install (TARGETS ams-admin ams-server ams-client
         EXPORT ams-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS ams-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT ams-targets
         DESTINATION ${ams-pkg}
         FILE "ams-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ams-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/ams-config-version.cmake"
         DESTINATION ${ams-pkg} )
install (DIRECTORY ../include/ams
         DESTINATION include
         FILES_MATCHING PATTERN "*.hpp")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ams-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/ams-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/ams-admin.pc"
         DESTINATION "lib/pkgconfig/")
