FILE(GLOB basictsmsources *.c)

add_library(basictsm ${basictsmsources})

# state that anybody linking to us needs to include the current source dir
# includes, while we don't.
target_include_directories(basictsm
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${PROJECT_BINARY_DIR}                    
          PRIVATE ${CMAKE_BINARY_DIR}
          PRIVATE ${CMAKE_SOURCE_DIR}
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libs/bacnet/src/
          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libs/bacnet/src/eLib/osLayer/linux
          )
