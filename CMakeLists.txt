project ( gaxb )
cmake_minimum_required ( VERSION 2.8 )

add_subdirectory(lua)

add_definitions(-std=c99)

find_package ( libxml2 )
if ( LIBXML2_FOUND )
  include_directories(${LIBXML2_INCLUDE_DIR})
endif( LIBXML2_FOUND )

add_executable(gaxb main.c)
include_directories(${lua_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/lua)
target_link_libraries(gaxb liblua)
target_link_libraries(gaxb ${LIBXML2_LIBRARIES})