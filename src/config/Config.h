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

    std::string baudrateStr() const { return getString(SETTINGS_SECTION, "Baudrate"); }
    std::string stopBitStr() const { return getString(SETTINGS_SECTION, "StopBit"); }
    std::string dataBitsStr() const { return getString(SETTINGS_SECTION, "DataBits"); }

    unsigned int maxCountRegsRead() const { return get<unsigned int>(SETTINGS_SECTION, "MaxCountRegsRead"); }
    std::chrono::milliseconds timeBetweenRequests() const { return get<std::chrono::milliseconds>(SETTINGS_SECTION, "TimeBetweenRequests"); }
    std::chrono::milliseconds responseTimeout() const { return get<std::chrono::milliseconds>(SETTINGS_SECTION, "ResponseTimeout"); }
    std::chrono::milliseconds pollDelay() const { return get<std::chrono::milliseconds>(SETTINGS_SECTION, "PollDelay"); }

    SectionsMap& getRegs(SectionsMap& map) {
      std::vector<std::string> sections = m_config.sections();
      ParamsMap params_map;

      for (auto& s : sections) {
        s = trim(s);
        if (startsWith(s, COILS_SECTION)              || startsWith(s, F1_SECTION)
          || startsWith(s, DISCRETE_SECTION)          || startsWith(s, F2_SECTION)
          || startsWith(s, INPUT_REGS_SECTION)        || startsWith(s, F4_SECTION)
          || startsWith(s, HOLDING_REGS_SECTION)      || startsWith(s, F3_SECTION)
          || startsWith(s, WRITE_WORD_SECTION)        || startsWith(s, WRITE_COIL_SECTION)) {
          std::vector<std::string> params = m_config[s];
          params_map.clear();
          for (auto& p : params) { params_map[p] = m_config.get(s, p); }
          map[s].push_back(params_map);
        }
      } 
      return map;
    }

    SectionsMap& getAreas(SectionsMap& map) {
      std::vector<std::string> sections = m_config.sections();
      ParamsMap params_map;

      for (auto& s : sections) {
        s = trim(s);
        if (startsWith(s, AREA_FUNCS_SECTION)
          || startsWith(s, COILS_AREA_SECTION)        || startsWith(s, F1_AREA_SECTION)
          || startsWith(s, DISCRETE_AREA_SECTION)     || startsWith(s, F2_AREA_SECTION)
          || startsWith(s, INPUT_REGS_AREA_SECTION)   || startsWith(s, F4_AREA_SECTION)
          || startsWith(s, HOLDING_REGS_AREA_SECTION) || startsWith(s, F3_AREA_SECTION)) {
          std::vector<std::string> params = m_config[s];
          params_map.clear();
          for (auto& p : params) { params_map[p] = m_config.get(s, p); }
          map[s].push_back(params_map);
        }
      } 
      return map;
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
