add_subdirectory(data)
add_subdirectory(types)
add_subdirectory(config)
add_subdirectory(action)
add_subdirectory(helpers)
add_subdirectory(version)

add_library(mb_slave OBJECT MbSlave.cpp)

# Exported includes
target_include_directories(mb_slave PUBLIC .)

target_link_libraries(mb_slave PRIVATE helpers 
                                       data_manager
                                       action_manager
                                       config)
