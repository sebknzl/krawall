# http://www.cmake.org/Wiki/CMake_Cross_Compiling

SET(CMAKE_SYSTEM_NAME GNU)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   $ENV{DEVKITARM}/bin/arm-none-eabi-gcc)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH $ENV{DEVKITARM}/arm-none-eabi)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY LAST)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE LAST)
