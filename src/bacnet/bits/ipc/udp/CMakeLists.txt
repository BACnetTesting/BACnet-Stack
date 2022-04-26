FILE(GLOB ipcudpsources *.c)

add_library(ipcudp ${ipcudpsources})

# state that anybody linking to us needs to include the current source dir
# includes, while we don't.
target_include_directories(ipcudp
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${PROJECT_BINARY_DIR}                    
          PRIVATE ${CMAKE_BINARY_DIR}
          PRIVATE ${CMAKE_SOURCE_DIR}
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../libs/bacnet/src/
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../libs/bacnet/src/eLib/osLayer/linux
          )

add_subdirectory(demo)