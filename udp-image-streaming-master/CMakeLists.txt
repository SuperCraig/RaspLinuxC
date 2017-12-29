cmake_minimum_required(VERSION 2.8)
project( lan_vid_pseudostream )
find_package( OpenCV REQUIRED )
find_package( Threads ) #Added by Craig for using thread
add_executable( server Server.cpp PracticalSocket.cpp )
target_link_libraries( server ${OpenCV_LIBS} )
add_executable( client Client.cpp PracticalSocket.cpp )
#target_link_libraries( client ${OpenCV_LIBS} )
target_link_libraries( client ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
