#include "DataManager.h"

#include "RegManager.h"

#include <iostream>

namespace mb {
namespace data {

DataManager::DataManager(Config* config) : m_config(config) {}

DataManager::~DataManager() { if (m_reg_manager) delete m_reg_manager; }

bool DataManager::start() {
    bool result = true;

    m_reg_manager = new RegManager;
    
    SectionsMap map;
    int slave_id = m_config->slaveId();
    if (m_config->getRegs(map)) {
        result = parseRegs(slave_id, map);
    }

    return result;
}

bool DataManager::parseRegs(int slave_id, SectionsMap& map) {
    bool result = false;
    bool is_describe = false;
    FuncNumber function;
        
    for (const auto& pair : map) {
        const std::string& section = pair.first;

        /** Get function */
        if (startsWith(section, COILS_SECTION)             || startsWith(section, F1_SECTION))  function = FuncNumber::READ_COIL;
        else if (startsWith(section, DISCRETE_SECTION)     || startsWith(section, F2_SECTION))  function = FuncNumber::READ_INPUT_COIL;
        else if (startsWith(section, HOLDING_REGS_SECTION) || startsWith(section, F3_SECTION))  function = FuncNumber::READ_REGS;
        else if (startsWith(section, INPUT_REGS_SECTION)   || startsWith(section, F4_SECTION))  function = FuncNumber::READ_INPUT_REGS;

        std::cout << section << std::endl;
        
        /* Get params */
        const auto& params_list = map[section];
        for (const auto& params: params_list) {
            for (const auto& param : params) {
                std::string val = param.second;
                m_reg_manager->addReg(slave_id, function, param.first, val);
                std::cout << param.first << ": " << param.second << std::endl;
            }
        }
        result = true;
    }
    
    return result;
}

Register* DataManager::findReadRegOnlyByName(const std::string& name) {
    Register* result = nullptr;
    
    std::forward_list<Register>& read_regs = m_reg_manager->getReadRegs();
    for (auto& read_reg : read_regs) {
        if (read_reg.name == name) {
            result = &read_reg;
            break;
        }
    }
    return result;
}

Register* DataManager::findReadReg(const std::string& name, const int func, const int slave_id) {
    Register* result = nullptr;
    
    std::forward_list<Register>& read_regs = m_reg_manager->getReadRegs();
    for (auto& read_reg : read_regs) {
        if (read_reg.slave_id == slave_id && read_reg.function == func && read_reg.name == name) {
            result = &read_reg;
            break;
        }
    }
    return result;
}

Register* DataManager::findReadReg(const int addr, const int func, const int slave_id) {
    Register* result = nullptr;
    
    std::forward_list<Register>& read_regs = m_reg_manager->getReadRegs();
    for (auto& read_reg : read_regs) {
        if (read_reg.slave_id == slave_id && read_reg.function == func && read_reg.address == addr) {
            result = &read_reg;
            break;
        }
    }
    return result;
}

bool DataManager::getMemInfo(std::string& range_str, int& adr, int& count) {
    bool result = false;
    int start, end;

    range_str = trim(range_str);
    std::vector<std::string> tokens = split(range_str, '-');

    std::string start_str;
    std::string end_str;

    if (tokens.size() >= 2) {
        start_str = trim(tokens[0]);
        end_str = trim(tokens[1]);
    }
    else if (!range_str.empty() && isUnsignedInteger(range_str)) {
        start_str = range_str;
        end_str = range_str;
    }

    // Проверяем, что обе подстроки являются числами
    if (isUnsignedInteger(start_str) || isUnsignedInteger(end_str)) {
        // Преобразуем строки в целые числа
        start = std::stoi(start_str);
        end = std::stoi(end_str);

        // Убедимся, что start меньше или равно end
        if (start > end) {
            int temp = start;
            start = end;
            end = temp;
        }
        adr = start;
        count = end + 1;
        result = true;
    }
    return result;
}

void DataManager::getMemoryMapInfo(MemoryMapInfo& mem_map_info) {
    std::string coils_range = m_config->getCoilsArea();
    std::string discrete_range = m_config->getDiscreteArea();
    std::string reg_range = m_config->getHoldingRegsArea();
    std::string reg_input_range = m_config->getInputRegsArea();

    mem_map_info.start_bits = 0;
    mem_map_info.nb_bits = 0;
    mem_map_info.start_input_bits = 0;
    mem_map_info.nb_input_bits = 0;
    mem_map_info.start_registers = 0;
    mem_map_info.nb_registers = 0;
    mem_map_info.start_input_registers = 0;
    mem_map_info.nb_input_registers = 0;

    if (!coils_range.empty()) {
        getMemInfo(coils_range, mem_map_info.start_bits, mem_map_info.nb_bits);
    }

    if (!discrete_range.empty()) {
        getMemInfo(discrete_range, mem_map_info.start_input_bits, mem_map_info.nb_input_bits);
    }

    if (!reg_range.empty()) {
        getMemInfo(reg_range, mem_map_info.start_registers, mem_map_info.nb_registers);
    }

    if (!reg_input_range.empty()) {
        getMemInfo(reg_input_range, mem_map_info.start_input_registers, mem_map_info.nb_input_registers);
    }
}


std::forward_list<Register>& DataManager::getRegs() { return m_reg_manager->getReadRegs(); }

} // mb
} // data
