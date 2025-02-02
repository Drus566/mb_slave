#ifndef MB_SLAVE_H
#define MB_SLAVE_H

#include "MbSlave.h"

#include "Config.h"
#include "DataManager.h"
#include "ActionManager.h"

namespace mb {

using namespace mb::data;
using namespace mb::config;
using namespace mb::action;

class MbSlave {

public:
   MbSlave(const std::string &path_config);
   ~MbSlave();

   bool start(bool wait = false);
   bool isConnect();
      
private:
	Config* m_config;
	DataManager* m_data_manager;
   ActionManager* m_action_manager;

	bool m_start;
}; // MbSlave

} // mb

#endif // MB_SLAVE_H