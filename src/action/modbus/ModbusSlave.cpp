#include "ModbusSlave.h"

#include "ModbusEnums.h"
#include "ModbusTrans.h"

#include <unistd.h>
#include <iostream>

namespace mb {
namespace action {

ModbusSlave::ModbusSlave(ModbusConnection& connection) : m_connection(connection),
																			m_ctx(NULL) {}

ModbusSlave::~ModbusSlave() {
	if (m_ctx) {
		modbusClose();
		free();

		if (m_connection.type == ModbusConnectionType::ETH) {
			clientSocketsClose();
			serverSocketClose();
		}
	}
}

bool ModbusSlave::setContext(const ModbusConnection& connection) {
	bool result = true;

	if (m_ctx) {
		modbusClose();
		free();
	}

	if (connection.type == ModbusConnectionType::RTU) {
		m_ctx = modbus_new_rtu(connection.rtu.serial_port.c_str(), 
									  connection.rtu.baudrate, 
									  connection.rtu.parity, 
									  connection.rtu.data_bits, 
									  connection.rtu.stop_bit);
	}
	else if (connection.type == ModbusConnectionType::ETH) {
		m_ctx = modbus_new_tcp(connection.eth.ip.c_str(), connection.eth.port);
	}
	else result = false; 

	if (!m_ctx) result = false;
	else setSlave(connection.slave_id);

	return result;
}

bool ModbusSlave::setContext() { return setContext(m_connection); }

void ModbusSlave::setDebug() { modbus_set_debug(m_ctx, TRUE); }

bool ModbusSlave::connect() {
	bool result = true;
   if (modbus_connect(m_ctx) == -1) result = false;
   return result;
}

/** @brief create memory for process requests on current context */
bool ModbusSlave::createMemory(int nb_bits, int nb_input_bits, int nb_registers, int nb_input_registers) {
	bool result = true;
	modbus_mapping_t* mb_mapping = modbus_mapping_new(nb_bits, nb_input_bits, nb_registers, nb_input_registers);
	if (mb_mapping == NULL) result = false;
	else  {
		m_mem.memory = mb_mapping;

		m_mem.length_bits = nb_bits;
		m_mem.length_input_bits = nb_input_bits;
		m_mem.length_regs = nb_registers;
		m_mem.length_input_regs = nb_input_registers;
	}
	return result;
}

/** @brief create memory for process requests on current context */
bool ModbusSlave::createMemory(int start_bits, int nb_bits,
										 int start_input_bits, int nb_input_bits,
										 int start_registers, int nb_registers,
										 int start_input_registers, int nb_input_registers) {
	bool result = true;
	modbus_mapping_t* mb_mapping = modbus_mapping_new_start_address(
		start_bits, nb_bits,
		start_input_bits, nb_input_bits,
		start_registers, nb_registers,
		start_input_registers, nb_input_registers
	);

	if (mb_mapping == NULL) result = false;
	else {
		m_mem.memory = mb_mapping;

		m_mem.offset_bits = start_bits;
		m_mem.offset_input_bits = start_input_bits;
		m_mem.offset_regs = start_registers;
		m_mem.offset_input_regs = start_input_registers;

		m_mem.length_bits = nb_bits;
		m_mem.length_input_bits = nb_input_bits;
		m_mem.length_regs = nb_registers;
		m_mem.length_input_regs = nb_input_registers;
	}
	return result;
}

bool ModbusSlave::setSlave(int slave_id) {
	bool result = true;
	if (modbus_set_slave(m_ctx, slave_id) == -1) result = false;
	return result;
}

bool ModbusSlave::getSlave(int& slave_id) {
	bool result = false;
	int ret = modbus_get_slave(m_ctx);
	if (ret != -1) {
		result = true;
		slave_id = ret;
	}
	return result;
}

/** @brief create and listen a TCP Modbus socket */
bool ModbusSlave::modbusTcpListen(int nb_connection) {
	bool result = false;
	int socket = modbus_tcp_listen(m_ctx, nb_connection);
	if (socket != -1) {
		m_listen_socket = socket;
		result = true;
	}
	return result;
}

/** @brief get server socket */
int ModbusSlave::getServerSocket() { return m_listen_socket; }

/** @brief accept a new connection on a TCP Modbus socket */
bool ModbusSlave::modbusTcpAccept() {
	bool result = false;
	int client_socket = modbus_tcp_accept(m_ctx, &m_listen_socket);
	if (client_socket != -1) {
		m_client_socket = client_socket;
		m_client_sockets.push_back(client_socket);
		result = true;
	}
	return result;
}

/** @brief get client sockets */
std::vector<int>& ModbusSlave::getClientSockets() { return m_client_sockets; }

int ModbusSlave::getClientSocket() { return m_client_socket; }

/** @brief receive query */
bool ModbusSlave::modbusReceive() {
	bool result = false;
	int length = modbus_receive(m_ctx, m_query.query);
	if (length != -1) {
		m_query.length = length;
		result = true;
		for (int i = 0; i < m_query.length; i++)
		{
			printf("%02X ", m_query.query[i]);
		}
		printf("\n");
	}

	return result;
}

/** @brief reply for query */
bool ModbusSlave::modbusReply() {
	bool result = true;
	int length = modbus_reply(m_ctx, m_query.query, m_query.length, m_mem.memory);
	if (length == -1) result = false;
	return result;
}

/** @brief get last query message */
bool ModbusSlave::getQueryMessage(uint8_t*& message, int& length) { 
	bool result = false;
	if (m_query.length != -1) {
		message = m_query.query; 
		length = m_query.length;
		result = true;
	}
	return result; 
}

bool ModbusSlave::flush() {
	bool result = true;
   if (modbus_flush(m_ctx) == -1) result = false;
	return result;
}

void ModbusSlave::serverSocketClose() { close(m_listen_socket); }

void ModbusSlave::clientSocketsClose() { 
	for (auto& sock : m_client_sockets) {
		close(sock);
	}
}

void ModbusSlave::socketClose(int socket) { close(socket); }

void ModbusSlave::modbusClose() { if (m_ctx) modbus_close(m_ctx); }

void ModbusSlave::free() { if (m_ctx) modbus_free(m_ctx); }

void ModbusSlave::freeMemory() { if (m_mem.memory) modbus_mapping_free(m_mem.memory); }

void ModbusSlave::fillModbusMemory(std::forward_list<Register>& regs) {
	int index, i_val;
	float f_val;
	RegDataType type;
	RegDataOrder order;
	uint16_t dword[2];

	for (auto& r : regs) {
		if (r.function == FuncNumber::READ_COIL && m_mem.length_bits > 0) {
			index = r.address - m_mem.offset_bits - 1;
			if (index >= 0 && index < m_mem.length_bits) {
				if (r.reg_info.value.i_val != 0) i_val = 1;
				else i_val = 0;
				m_mem.memory->tab_bits[index] = static_cast<uint8_t>(i_val);
			}
		}
		else if (r.function == FuncNumber::READ_INPUT_COIL && m_mem.length_input_bits > 0) {
			index = r.address - m_mem.offset_input_bits - 1;
			if (index >= 0 && index < m_mem.length_input_bits) {
				if (r.reg_info.value.i_val != 0) i_val = 1;
				else i_val = 0;
				m_mem.memory->tab_input_bits[index] = static_cast<uint8_t>(i_val);
			}
		}
		else if (r.function == FuncNumber::READ_REGS && m_mem.length_regs > 0) {
			index = r.address - m_mem.offset_regs - 1;
			if (index >= 0 && index < m_mem.length_regs) {
				type = r.reg_info.data_type;
				if (isDwordDataType(type) && ((index + 1) < m_mem.length_regs)) {
					ModbusTrans::valToRawReg32(dword, r);
					m_mem.memory->tab_registers[index] = dword[0];
					m_mem.memory->tab_registers[index + 1] = dword[1];
				}
				else {
					if (type == RegDataType::FLOAT16) {
						int precision = std::pow(10, r.reg_info.precision);
						i_val = r.reg_info.value.f_val * precision;
						m_mem.memory->tab_registers[index] = static_cast<uint16_t>(i_val);
					}
					else {
						i_val = r.reg_info.value.i_val;
						m_mem.memory->tab_registers[index] = static_cast<uint16_t>(i_val);
					}
				}
			}
		}
		else if (r.function == FuncNumber::READ_INPUT_REGS && m_mem.length_input_regs > 0) {
			index = r.address - m_mem.offset_regs - 1;
			if (index >= 0 && index < m_mem.length_regs) {
				type = r.reg_info.data_type;
				if (isDwordDataType(type) && ((index + 1) < m_mem.length_input_regs)) {
					ModbusTrans::valToRawReg32(dword, r);
					m_mem.memory->tab_input_registers[index] = dword[0];
					m_mem.memory->tab_input_registers[index + 1] = dword[1];
				}
				else {
					if (type == RegDataType::FLOAT16) {
						int precision = std::pow(10, r.reg_info.precision);
						i_val = r.reg_info.value.f_val * precision;
						m_mem.memory->tab_input_registers[index] = static_cast<uint16_t>(i_val);
					}
					else {
						i_val = r.reg_info.value.i_val;
						m_mem.memory->tab_input_registers[index] = static_cast<uint16_t>(i_val);
					}
				}
			}
		}
	}
}

} // action
} // mb
