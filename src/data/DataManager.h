#ifndef MB_DATA_MANAGER_H
#define MB_DATA_MANAGER_H

#include "Config.h"
#include "ModbusEnums.h"
#include "ModbusTrans.h"
#include "ModbusRegister.h"
#include "MemoryMapInfo.h"

#include <forward_list>
#include <string>

namespace mb {
namespace data {

using namespace mb::config;
using namespace mb::types;

class RegManager;
class RangeManager;
class RequestManager;

class DataManager {    
public:
    DataManager(Config* config);
    ~DataManager();
    
    bool start();

    Register* findReadRegOnlyByName(const std::string& name);
    Register* findReadReg(const std::string& name, const int func, const int slave_id);
    Register* findReadReg(const int addr, const int func, const int slave_id);

    void getMemoryMapInfo(MemoryMapInfo& mem_map_info);
    std::forward_list<Register>& getRegs();

private:
    bool getMemInfo(std::string& range_str, int& adr, int& count);
    bool parseRegs(int slave_id, SectionsMap& map);
    void completeRanges();

    Config* m_config;

    RegManager* m_reg_manager;
};

} // data
} // mb

#endif // MB_DATA_MANAGER_H
