#ifndef MODBUS_CONNECTION_H
#define MODBUS_CONNECTION_H

#include <string>

namespace mb {
namespace types {

/** @brief типы данных регистров */
enum class ModbusConnectionType {
	NONE,
   RTU,
   ETH
};

struct ModbusConnection {
	ModbusConnectionType type;
	int slave_id;
	
	struct {
		std::string serial_port;
		int baudrate;
		int stop_bit;
		int data_bits;
		char parity;
	} rtu;

	struct {
		std::string ip;
		int port;
		int tcp_count_connections;
	} eth;
};

} // types
} // mb

#endif // MODBUS_CONNECTION_H