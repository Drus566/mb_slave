#ifndef MB_CONFIG_H
#define MB_CONFIG_H

#include "IniFile.h"
#include "String.h"
#include "ConfigKeys.h"

#include <chrono>
#include <vector>
#include <unordered_map>

namespace mb {
namespace config {

using namespace mb::helpers;

using ParamsMap = std::unordered_map<std::string, std::string>;
using SectionsMap = std::unordered_map<std::string, std::vector<ParamsMap>>;

class Config {
  public:
    Config(const std::string& config_path) : m_config(config_path) {}

    std::string type() const { return getString(SETTINGS_SECTION, "Type"); }
    
    unsigned int slaveId() const { return get<unsigned int>(SETTINGS_SECTION, "SlaveId"); }
    
    std::string serialPort() const { return getString(SETTINGS_SECTION, "SerialPort"); }
    unsigned int baudrate() const { return get<unsigned int>(SETTINGS_SECTION, "Baudrate"); }
    unsigned int stopBit() const { return get<unsigned int>(SETTINGS_SECTION, "StopBit"); }
    unsigned int dataBits() const { return get<unsigned int>(SETTINGS_SECTION, "DataBits"); }
    std::string parity() const { return getString(SETTINGS_SECTION, "Parity"); }

    std::string ip() const { return getString(SETTINGS_SECTION, "Ip"); }
    unsigned int port() const { return get<unsigned int>(SETTINGS_SECTION, "Port"); }
    unsigned int tcpConnectionCount() const { return get<unsigned int>(SETTINGS_SECTION, "TcpConnectionCount"); }

    std::string getCoilsArea() const { return getString(AREA_SECTION, "Coils"); }
    std::string getDiscreteArea() const { return getString(AREA_SECTION, "DiscreteInputs"); }
    std::string getInputRegsArea() const { return getString(AREA_SECTION, "InputRegisters"); }
    std::string getHoldingRegsArea() const { return getString(AREA_SECTION, "HoldingRegisters"); }

    bool getRegs(SectionsMap& map) {
      bool result = false;
      std::vector<std::string> sections = m_config.sections();
      ParamsMap params_map;

      for (auto& s : sections) {
        s = trim(s);
        if (startsWith(s, COILS_SECTION)              || startsWith(s, F1_SECTION)
          || startsWith(s, DISCRETE_SECTION)          || startsWith(s, F2_SECTION)
          || startsWith(s, INPUT_REGS_SECTION)        || startsWith(s, F4_SECTION)
          || startsWith(s, HOLDING_REGS_SECTION)      || startsWith(s, F3_SECTION)) {
          std::vector<std::string> params = m_config[s];
          params_map.clear();
          for (auto& p : params) { params_map[p] = m_config.get(s, p); }
          map[s].push_back(params_map);
          result = true;
        }
      }
      return result;
    }
      
  private:
    mb::helpers::IniFile m_config;

    bool getBool(const std::string& section, const std::string& param) const { return m_config.get(section, param).toBool(); }
    std::string getString(const std::string& section, const std::string& param) const { return m_config.get(section, param); }
    template <typename T>
    T get(const std::string& section, const std::string& param) const { return T(m_config.get(section, param).toUInt()); }
};

} // config
} // mb

#endif // MB_CONFIG_H
