#ifndef MODBUS_MEMORY_MAP_INFO_H
#define MODBUS_MEMORY_MAP_INFO_H

namespace mb {
namespace types {

struct MemoryMapInfo {
	int start_bits; 
	int nb_bits;
	int start_input_bits; 
	int nb_input_bits;
	int start_registers; 
	int nb_registers;
	int start_input_registers; 
	int nb_input_registers;
};

} // types
} // mb

#endif // MODBUS_MEMORY_MAP_INFO_H