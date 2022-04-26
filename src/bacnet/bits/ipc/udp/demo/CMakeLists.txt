FILE(GLOB ipcudpdemosources *.c)

add_library(ipcudpdemo ${ipcudpdemosources})

# state that anybody linking to us needs to include the current source dir
# includes, while we don't.
target_include_directories(ipcudpdemo
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${CMAKE_BINARY_DIR}
          PRIVATE ${CMAKE_SOURCE_DIR}
          PRIVATE ${CMAKE_SOURCE_DIR}/libs/bacnet/src/
          PRIVATE ${CMAKE_SOURCE_DIR}/libs/bacnet/src/eLib/osLayer/linux
          )

