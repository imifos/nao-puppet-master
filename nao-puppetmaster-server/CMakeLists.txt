cmake_minimum_required(VERSION 2.8)
project(nao-puppetmaster-server)

find_package(qibuild)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# List of source files
set(_SRC
    main.cpp
   )

# Main application
qi_create_bin(nao-puppetmaster-server ${_SRC})

# Testing application
enable_testing()
qi_create_test(test_nao-puppetmaster-server "test.cpp")

# Tell CMake that the project depends on ALCOMMON etc
# This will set the libraries to link the project with,
# the include paths, etc.
qi_use_lib(nao-puppetmaster-server ALCOMMON)

#target_link_libraries(nao-puppetmaster-server mylibs)



