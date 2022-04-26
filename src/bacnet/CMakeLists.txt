FILE(GLOB bacnetsources *.c)

add_library(bacnet ${bacnetsources})

# state that anybody linking to us needs to include the current source dir
# includes, while we don't.
target_include_directories(bacnet
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${PROJECT_BINARY_DIR}                    
          PRIVATE ${CMAKE_BINARY_DIR}
          PRIVATE ${CMAKE_SOURCE_DIR}
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/bacnet/src/
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/bacnet/src/eLib/osLayer/linux
          )

add_subdirectory(basic)
add_subdirectory(bits)
add_subdirectory(datalink)