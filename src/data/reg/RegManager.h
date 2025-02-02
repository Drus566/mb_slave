#ifndef MB_REG_MANAGER_H
#define MB_REG_MANAGER_H

#include "ModbusEnums.h"
#include "ModbusTrans.h"
#include "ModbusRegister.h"

#include <forward_list>
#include <string>

namespace mb {
namespace data {
    
using namespace mb::types;

class RegManager {    
    public:
        RegManager() {}

        std::forward_list<Register>& getReadRegs();
        bool addReg(const int slave_id, const FuncNumber func, const std::string& name, std::string& reg_str);
        Register* addReadReg(const int address, const int slave_id, const FuncNumber func);
    
    private:
        bool parseReadReg(const std::string& reg_str, int& address, RegisterInfo& reg_info);
        std::forward_list<Register> m_regs;
};

} // data
} // mb

#endif // MB_REG_MANAGER_H