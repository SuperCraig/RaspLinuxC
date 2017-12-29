cmake_minimum_required(VERSION 2.8)
project( lan_vid_pseudostream )
find_package( OpenCV REQUIRED )
find_package( Threads ) #Added by Craig for using thread
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK2_LIBRARY_DIRS})

# Add other flags to the compiler
ADD_DEFINITIONS(${GTK2_CFLAGS_OTHER})


add_executable( server Server.cpp PracticalSocket.cpp )
target_link_libraries( server ${OpenCV_LIBS} )
add_executable( client Client.cpp PracticalSocket.cpp )
#target_link_libraries( client ${OpenCV_LIBS} )
target_link_libraries( client   ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${GTK2_LIBRARIES})
