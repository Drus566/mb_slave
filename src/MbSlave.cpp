#include "MbSlave.h"

namespace mb {

MbSlave::MbSlave(const std::string& config_path) : m_start(false) {
	m_config = new Config(config_path);
	m_data_manager = new DataManager(m_config);
	m_action_manager = new ActionManager(m_config, m_data_manager);
}

MbSlave::~MbSlave() {
	if (m_config) delete m_config;
	if (m_data_manager) delete m_data_manager;
	if (m_action_manager) delete m_action_manager;
}

bool MbSlave::start(bool wait) {
	if (m_start) return false;

	if (!m_data_manager->start()) return false;
	if (!m_action_manager->start(wait)) return false;
	
	m_start = true;
	return true;
}

bool MbSlave::isConnect() { return m_action_manager->isConnect(); }

} // mb

