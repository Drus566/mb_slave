#include "ModbusMaster.h"

#include "Time.h"

namespace mb {
namespace action {

ModbusMaster::ModbusMaster(ModbusConnection& connection) : m_connection(connection),
																			  m_ctx(NULL) {}

ModbusMaster::~ModbusMaster() {
	if (m_ctx) {
   	close();
   	free();
	}
}

bool ModbusMaster::setContext(const ModbusConnection& connection) {
	bool result = true;

	if (m_ctx) {
		close();
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

	if (!m_ctx || !setResponseTimeout()) result = false;

	return result;
}

bool ModbusMaster::setContext() { return setContext(m_connection); }

bool ModbusMaster::connect() {
	bool result = true;
   if (modbus_connect(m_ctx) == -1) result = false;
   return result;
}

bool ModbusMaster::setResponseTimeout() {
	bool result = true;
	int seconds, microseconds;
	mb::helpers::millisecondsToSecondsMicroseconds(m_connection.response_timeout, seconds, microseconds);
   if (modbus_set_response_timeout(m_ctx, seconds, microseconds) == -1) result = false; 
   return result;
}

bool ModbusMaster::setResponseTimeout(int tv_sec, int tv_usec) {
	bool result = true;
   if (modbus_set_response_timeout(m_ctx, tv_sec, tv_usec) == -1) result = false; 
   return result;
}

bool ModbusMaster::getResponseTimeout(uint32_t* tv_sec, uint32_t* tv_usec) {
	bool result = true;
   if (modbus_get_response_timeout(m_ctx, tv_sec, tv_usec) == -1) result = false; 
   return result;
}

bool ModbusMaster::setSlave(int slave_id) {
	bool result = true;
	if (modbus_set_slave(m_ctx, slave_id) == -1) result = false;
	return result;
}

bool ModbusMaster::getSlave(int& slave_id) {
	bool result = false;
	int ret = modbus_get_slave(m_ctx);
	if (ret != -1) {
		result = true;
		slave_id = ret;
	}
	return result;
}

bool ModbusMaster::readBits(int slave_id, int addr, int count, uint8_t* dest) {
   bool result = true;
	setSlave(slave_id);
   if (modbus_read_bits(m_ctx, addr, count, dest) == -1) result = false; 
   return result;
}

bool ModbusMaster::readInputBits(int slave_id, int addr, int count, uint8_t* dest) {
	bool result = true;
	setSlave(slave_id);
   if (modbus_read_input_bits(m_ctx, addr, count, dest) == -1) result = false; 
   return result;
}

bool ModbusMaster::readRegisters(int slave_id, int addr, int count, uint16_t* dest) {
	bool result = true;
	setSlave(slave_id);
   if (modbus_read_registers(m_ctx, addr, count, dest) == -1) result = false; 
   return result;
}

bool ModbusMaster::readInputRegisters(int slave_id, int addr, int count, uint16_t* dest) {
	bool result = true;
	setSlave(slave_id);
   if (modbus_read_input_registers(m_ctx, addr, count, dest) == -1) result = false; 
   return result;
}

bool ModbusMaster::writeBit(int slave_id, int addr, int bit) {
	bool result = true;
	setSlave(slave_id);
   if (modbus_write_bit(m_ctx, addr, bit) == -1) result = false; 
   return result;
}

bool ModbusMaster::writeBits(int slave_id, int addr, int count, const uint8_t* values) {
	bool result = true;
	setSlave(slave_id);
   if (modbus_write_bits(m_ctx, addr, count, values) == -1) result = false; 
   return result;
}

bool ModbusMaster::writeRegister(int slave_id, int addr, const uint16_t value) {
	bool result = true;
	setSlave(slave_id);
	if (modbus_write_register(m_ctx, addr, value) == -1) result = false; 
   return result;
}

bool ModbusMaster::writeRegisters(int slave_id, int addr, int count, const uint16_t* values) {
	bool result = true;
	setSlave(slave_id);
	if (modbus_write_registers(m_ctx, addr, count, values) == -1) result = false; 
   return result;
}

bool ModbusMaster::flush() {
	bool result = true;
   if (modbus_flush(m_ctx) == -1) result = false;
	return result;
}

void ModbusMaster::close() { if (m_ctx) modbus_close(m_ctx); }

void ModbusMaster::free() { if (m_ctx) modbus_free(m_ctx); }

} // action
} // mb
