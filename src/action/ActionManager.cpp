/** timing params */
#define DEFAULT_TIME_BETWEEN_REQUESTS 10
#define DEFAULT_RESPONSE_TIMEOUT 500

/** rtu default params */
#define DEFAULT_BAUDRATE 19200
#define DEFAULT_STOP_BIT 1
#define DEFAULT_DATA_BITS 8
#define DEFAULT_PARITY 'N'

/** ethernet default params */
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 502

#include "ActionManager.h"

#include "ModbusMaster.h"

#include <iostream>

namespace mb {
namespace action {

ActionManager::ActionManager(Config* config, 
									  DataManager* data_manager,
									  MemManager* mem_manager) : m_config(config), 
									  						  			  m_data_manager(data_manager),
																		  m_mem_manager(mem_manager),
							   		  					  			  m_run(false),
																		  m_connect(false) {
	m_time_between_requests = m_config->timeBetweenRequests();
	m_response_timeout = m_config->responseTimeout();
	m_poll_delay = m_config->pollDelay();

	if (m_time_between_requests.count() == 0) m_time_between_requests = std::chrono::milliseconds(DEFAULT_TIME_BETWEEN_REQUESTS);
	if (m_response_timeout.count() == 0) m_response_timeout = std::chrono::milliseconds(DEFAULT_RESPONSE_TIMEOUT);
	
	m_modbus_connection.response_timeout = m_response_timeout.count();

	std::string type = m_config->type();
	type = toUpperCase(type);

	if (type == "RTU") {
		m_modbus_connection.type = ModbusConnectionType::RTU;

		m_modbus_connection.rtu.serial_port = m_config->serialPort();

		int baudrate = m_config->baudrate();
		if (baudrate == 0) m_modbus_connection.rtu.baudrate = DEFAULT_BAUDRATE;
		m_modbus_connection.rtu.baudrate = baudrate;

		int stop_bit = m_config->stopBit();
		if (stop_bit == 0) m_modbus_connection.rtu.stop_bit = DEFAULT_STOP_BIT;
		m_modbus_connection.rtu.stop_bit = stop_bit;

		int data_bits = m_config->dataBits();
		if (data_bits == 0) m_modbus_connection.rtu.data_bits = DEFAULT_DATA_BITS;
		m_modbus_connection.rtu.data_bits = data_bits;

		std::string parity = m_config->parity();
		if (parity.empty()) m_modbus_connection.rtu.parity = DEFAULT_PARITY;
		else m_modbus_connection.rtu.parity = parity.c_str()[0];
	}
	else if (type == "TCP") {
		m_modbus_connection.type = ModbusConnectionType::ETH;
		
		std::string ip = m_config->ip();
		if (ip.empty()) m_modbus_connection.eth.ip = DEFAULT_IP;
		else m_modbus_connection.eth.ip = ip;
		
		int port = m_config->port();
		if (port == 0) port = DEFAULT_PORT;
		m_modbus_connection.eth.port = port;
	}

	m_modbus_master = new ModbusMaster(m_modbus_connection);
} 

ActionManager::~ActionManager() {
	stop();
	if (m_modbus_master) delete m_modbus_master;
}

bool ActionManager::start() {
	if (m_run.load()) return false;
	
	if (!m_modbus_master->setContext()) return false;
	if (!m_modbus_master->connect()) return false;
	
	m_thread = std::thread(&ActionManager::payload, 
									this, 
									m_data_manager, 
									m_mem_manager);
	m_thread.detach();
	
	return true;
}

bool ActionManager::stop() {
	if (!m_run.load()) return false;
	
	if (m_thread.joinable()) m_thread.join();

	m_modbus_master->close();

	m_run.store(false);
	return true;
}

bool ActionManager::isRun() { return m_run.load(); }

bool ActionManager::isConnect() { return m_connect.load(); };

void ActionManager::payload(DataManager* data_manager, MemManager* mem_manager) {
	m_run.store(true);

	const std::vector<Request>& read_requests = data_manager->getReadRequests();
	const int max_count_read_regs = data_manager->getMaxCountReadRegs();
	
	uint16_t buffer[max_count_read_regs];
	uint8_t* u8_ptr = reinterpret_cast<uint8_t*>(buffer);
	uint16_t* u16_ptr = buffer;
	int error, max_error = 30;
	bool response;

	while (m_run.load()) {
		for (const auto& request : read_requests) {
			// std::cout << "Request: " 
			// 			 << request.slave_id 
			// 			 << ", " << request.function 
			// 			 << ", " << request.address 
			// 			 << std::endl;
			switch (request.function) {
				case FuncNumber::READ_COIL:
					response = m_modbus_master->readBits(request.slave_id, request.address, request.quantity, u8_ptr); 
					if (response) mem_manager->writeMem(u8_ptr, request.mem_chunk->u8_ptr, request.quantity, request.offset_mem_chunk);
					break;
				
				case FuncNumber::READ_INPUT_COIL: 
					response = m_modbus_master->readInputBits(request.slave_id, request.address, request.quantity, u8_ptr); 
					if (response) mem_manager->writeMem(u8_ptr, request.mem_chunk->u8_ptr, request.quantity, request.offset_mem_chunk);
					break;

				case FuncNumber::READ_REGS: 
					response = m_modbus_master->readRegisters(request.slave_id, request.address, request.quantity, u16_ptr); 
					if (response) mem_manager->writeMem(u16_ptr, request.mem_chunk->u16_ptr, request.quantity, request.offset_mem_chunk);
					break;

				case FuncNumber::READ_INPUT_REGS:
					response = m_modbus_master->readRegisters(request.slave_id, request.address, request.quantity, u16_ptr); 
					if (response) mem_manager->writeMem(u16_ptr, request.mem_chunk->u16_ptr, request.quantity, request.offset_mem_chunk);
					break;

			}			

			if (!response) ++error; 
			else { 
				error = 0;
				m_connect.store(true);
			}

			if (error > max_error) { 
				error = max_error;
				m_connect.store(false);
			}
			std::this_thread::sleep_for(m_time_between_requests);
			// TODO: check out requests
		}
		std::this_thread::sleep_for(m_poll_delay);
	}
}

} // action
} // mb
