#ifndef MB_ACTION_MANAGER_H
#define MB_ACTION_MANAGER_H

#include "Config.h"
#include "DataManager.h"
#include "ModbusConnection.h"

#include <chrono>
#include <thread>
#include <atomic>

namespace mb {
namespace action {

class ModbusSlave;

using namespace mb::config;
using namespace mb::types;
using namespace mb::data;

class ActionManager {    
public:
   ActionManager(Config* config, DataManager* data_manager);
   ~ActionManager();
   
   bool start(bool wait = false);
	bool stop();
	bool isRun();
	bool isConnect();

private:
	DataManager* m_data_manager;

	ModbusConnection m_modbus_connection;
	ModbusSlave *m_modbus_slave;

	Config* m_config;

	std::atomic<bool> m_run;
	std::atomic<bool> m_connect;
	std::thread m_thread;

	void payload();
	void tcpPayload();
	void rtuPayload();
};

} // action
} // mb

#endif // MB_ACTION_MANAGER