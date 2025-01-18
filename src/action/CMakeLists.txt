add_library(action_manager OBJECT
    ActionManager.cpp
    modbus/ModbusSlave.cpp
)

target_include_directories(action_manager PUBLIC .)

target_include_directories(action_manager PRIVATE modbus)

target_link_libraries(action_manager PUBLIC helpers 
                                            types
                                            config
                                            data_manager
                                            modbus-static)

