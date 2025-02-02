add_library(data_manager OBJECT
    DataManager.cpp
    reg/RegManager.cpp
)

target_include_directories(data_manager PUBLIC .)

target_include_directories(data_manager PRIVATE reg)      

target_link_libraries(data_manager PUBLIC helpers 
                                          types
                                          config)

