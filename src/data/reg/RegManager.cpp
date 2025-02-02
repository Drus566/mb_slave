#include "RegManager.h"

#include <iostream>

namespace mb {
namespace data {

bool RegManager::parseReadReg(const std::string& reg_str, int& address, RegisterInfo& reg_info) {
	bool result = false;
	char delimeter = ',';
	char precision_delimeter = '_';

	std::vector<std::string> tokens = split(reg_str, delimeter);
	if (!tokens.empty()) {
		const int ADR_POS = 0;
		const int VAL_POS = 1;
		const int TYPE_POS = 2;
		const int ORDER_POS = 3;

		bool already_use_int = false;

		std::string addr = tokens.at(ADR_POS);
		if (isUnsignedInteger(addr)) {
			address = std::stoi(addr);
			result = true;
		}

		if (result && tokens.size() > 1) {
			std::string val = tokens.at(VAL_POS);
			if (isInteger(val))  {
				reg_info.value.i_val = std::stoi(val);
				already_use_int = true;
			}
			else if (isFloat(val)) {
				reg_info.value.f_val = std::stof(val);
				reg_info.data_type = RegDataType::FLOAT16;
			}
			else result = false;
		}

		if (result && tokens.size() > 2) {
			std::string data_type = tokens.at(TYPE_POS);
			toUpperCase(data_type);
			if (getRegDataTypeFromStr(reg_info.data_type, data_type)) {
				if (isFloatDataType(reg_info.data_type)) {
					if (already_use_int) reg_info.value.f_val = reg_info.value.i_val;
					std::vector<std::string> float_tokens = split(data_type, precision_delimeter);
					if (float_tokens.size() > 1) {
						std::string precision_str = float_tokens.at(1);
						if (isUnsignedInteger(precision_str))reg_info.precision = std::stoi(precision_str);
					}
				}
			}
			else result = false;
		}

		if (result && tokens.size() > 3) {
			std::string order = tokens.at(ORDER_POS);
			toUpperCase(order);
			if (!getRegDataOrderFromStr(reg_info.order, order)) result = false;
		}
	}
	return result;
}

bool RegManager::addReg(const int slave_id, const FuncNumber func, const std::string& name, std::string& reg_str) {
	bool result = false;
	
	char delimeter = ',';
	char precision_delimeter = '_';

	int address;
	reg_str = trim(reg_str);

	RegisterInfo reg_info;

	// parse read func
	if (isReadFunc(func)) result = parseReadReg(reg_str, address, reg_info);
	if (result) m_regs.emplace_front(address, reg_info, name, slave_id, func);

	return result;
}

Register* RegManager::addReadReg(const int address, const int slave_id, const FuncNumber func) {
	RegisterInfo reg_info; 
	m_regs.emplace_front(address, reg_info, "", slave_id, func);
	auto it = m_regs.begin();
	Register* reg = &(*it);
	return reg;
}

std::forward_list<Register>& RegManager::getReadRegs() { return m_regs; }

} // data
} // mb