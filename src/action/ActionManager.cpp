/** timing params */
#define DEFAULT_TCP_COUNT_CONNECTIONS 5

/** rtu default params */
#define DEFAULT_BAUDRATE 19200
#define DEFAULT_STOP_BIT 1
#define DEFAULT_DATA_BITS 8
#define DEFAULT_PARITY 'N'

/** ethernet default params */
#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_PORT 5555

#include "ActionManager.h"

#include "ModbusSlave.h"
#include "MemoryMapInfo.h"

#include <iostream>
#include <poll.h>

namespace mb {
namespace action {

ActionManager::ActionManager(Config* config, 
									  DataManager* data_manager) : m_config(config), 
									  						  			    m_data_manager(data_manager),
							   		  					  			    m_run(false),
																		    m_connect(false) {	
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

		int tcp_count_connections = m_config->tcpConnectionCount();
		if (tcp_count_connections == 0) tcp_count_connections = DEFAULT_TCP_COUNT_CONNECTIONS;
		m_modbus_connection.eth.tcp_count_connections = tcp_count_connections;
	}
	m_modbus_connection.slave_id = m_config->slaveId();

	m_modbus_slave = new ModbusSlave(m_modbus_connection);
} 

ActionManager::~ActionManager() {
	stop();
	if (m_modbus_slave) delete m_modbus_slave;
}

bool ActionManager::start(bool wait) {
	if (m_run.load()) return false;
	
	if (!m_modbus_slave->setContext()) return false;

	m_modbus_slave->setDebug();

	MemoryMapInfo mem_map_info;
	m_data_manager->getMemoryMapInfo(mem_map_info);

	if (!m_modbus_slave->createMemory(mem_map_info.start_bits, mem_map_info.nb_bits,
												 mem_map_info.start_input_bits, mem_map_info.nb_input_bits,
												 mem_map_info.start_registers, mem_map_info.nb_registers,
												 mem_map_info.start_input_registers, mem_map_info.nb_input_registers)) {
		return false;
	}

	m_modbus_slave->fillModbusMemory(m_data_manager->getRegs());

	if (m_modbus_connection.type == ModbusConnectionType::RTU && !m_modbus_slave->connect()) {
		return false;
	}
	else if (m_modbus_connection.type == ModbusConnectionType::ETH && !m_modbus_slave->modbusTcpListen(m_modbus_connection.eth.tcp_count_connections)) {
		return false;
	}

	m_thread = std::thread(&ActionManager::payload, this);
	if (!wait) m_thread.detach();
	else m_thread.join();
	
	return true;
}

bool ActionManager::stop() {
	if (!m_run.load()) return false;
	
	if (m_thread.joinable()) m_thread.join();

	m_modbus_slave->modbusClose();
	m_modbus_slave->freeMemory();

	if (m_modbus_connection.type == ModbusConnectionType::ETH) {
		m_modbus_slave->clientSocketsClose();
		m_modbus_slave->serverSocketClose();
	}

	m_run.store(false);
	return true;
}

bool ActionManager::isRun() { return m_run.load(); }

bool ActionManager::isConnect() { return m_connect.load(); };

void ActionManager::payload() {
	m_run.store(true);

	m_modbus_slave->flush();
	if (m_modbus_connection.type == ModbusConnectionType::RTU) rtuPayload();
	else if (m_modbus_connection.type == ModbusConnectionType::ETH) tcpPayload();
}

void ActionManager::tcpPayload() {
	int max_client_count = m_modbus_connection.eth.tcp_count_connections;
	struct pollfd fds[max_client_count + 1];
	int server_socket = m_modbus_slave->getServerSocket();
	int client_socket;

	for (int i = 0; i <= max_client_count; i++) {
		fds[i].fd = -1;			// Инициализируем fd
		fds[i].events = POLLIN; // Устанавливаем событие на чтение
	}

	fds[0].fd = server_socket; // Устанавливаем слушающий сокет
	fds[0].events = POLLIN;

	while (m_run.load()) {
		int poll_count = poll(fds, max_client_count + 1, -1); // Ожидаем события
		if (poll_count < 0) {
			std::cout << "poll error" << std::endl;
			break;
		}

		// Проверяем слушающий сокет на наличие новых подключений
		if (fds[0].revents & POLLIN) {
			if (!m_modbus_slave->modbusTcpAccept()) continue;
			client_socket = m_modbus_slave->getClientSocket();
			
			// Добавляем новый клиентский сокет в массив fds
			for (int i = 1; i <= max_client_count; i++) { 
				// Начинаем с 1, т.к. 0 — это слушающий сокет
				if (fds[i].fd == -1) {
					fds[i].fd = client_socket;
					break;
				}
			}
		}

		// Обрабатываем события для всех клиентских сокетов
		for (int i = 1; i <= max_client_count; i++) {
			if (fds[i].fd != -1) {
				if (fds[i].revents & POLLIN) {
					if (m_modbus_slave->modbusReceive()) {
						if (!m_modbus_slave->modbusReply()) {
							close(fds[i].fd);
							fds[i].fd = -1; // Освобождаем слот для клиента
						}
					}
					else {
						close(fds[i].fd);
						fds[i].fd = -1; // Освобождаем слот для клиента
					}
				}
				else if (fds[i].revents & POLLERR) {
					close(fds[i].fd);
					fds[i].fd = -1; // Освобождаем слот для клиента
				}
				else if (fds[i].revents & POLLHUP) {
					close(fds[i].fd);
					fds[i].fd = -1; // Освобождаем слот для клиента
				}
			}
		}
	}
}

void ActionManager::rtuPayload() {
	while (m_run.load()) m_modbus_slave->modbusReceive() && m_modbus_slave->modbusReply();
		// std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

} // action
} // mb
