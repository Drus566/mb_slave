#ifndef MB_MODBUS_SLAVE_H
#define MB_MODBUS_SLAVE_H

#include "ModbusConnection.h"
#include "ModbusRegister.h"
#include "modbus.h"

#define MB_QUERY_LENGTH MODBUS_TCP_MAX_ADU_LENGTH

#include <cstdint>
#include <vector>
#include <forward_list>

namespace mb {
namespace action {

using namespace mb::types;

class ModbusSlave  {
public:
    /** @brief Constructor */
    ModbusSlave(ModbusConnection& connection);
    ~ModbusSlave();

    bool setContext();
    bool setContext(const ModbusConnection& connection);

    /** @brief connect on current context */
    bool connect();

    /** @brief create memory for process requests on current context */
    bool createMemory(int nb_bits, int nb_input_bits, int nb_registers, int nb_input_registers);

    /** @brief create memory for process requests on current context */
    bool createMemory(int start_bits, int nb_bits,
                      int start_input_bits, int nb_input_bits,
                      int start_registers, int nb_registers,
                      int start_input_registers, int nb_input_registers);

    /** @brief set response timeoute */
    bool setResponseTimeout();

    /** @brief set response timeoute */
    bool setResponseTimeout(int tv_sec, int tv_usec);

    /** @brief get response timeoute */
    bool getResponseTimeout(uint32_t* tv_sec, uint32_t* tv_usec);

    /** @brief set slave id */
    bool setSlave(int slave_id);

    /** @brief get slave id */
    bool getSlave(int& slave_id);

    /** @brief create and listen a TCP Modbus socket */
    bool modbusTcpListen(int nb_connection);

    /** @brief get server socket */
    int getServerSocket();

    /** @brief accept a new connection on a TCP Modbus socket */
    bool modbusTcpAccept();

    /** @brief get client sockets */
    std::vector<int>& getClientSockets();
    
    /** @brief get client socket */
    int getClientSocket();

    /** @brief receive query */
    bool modbusReceive();

    /** @brief reply for query */
    bool modbusReply();

    /** @brief get last query message */
    bool getQueryMessage(uint8_t*& message, int& length);

    /** @brief flush modbus buffer */
    bool flush();

    /** @brief close modbus server socket */
    void serverSocketClose();

    /** @brief close modbus clients socket */
    void clientSocketsClose();

    /** @brief close modbus socket */
    void socketClose(int socket);

    /** @brief close modbus connection on current context */
    void modbusClose();

    /** @brief free modbus current context */
    void free();

    /** @brief free modbus memory */
    void freeMemory();

    /** @brief fill map with values */
    void fillModbusMemory(std::forward_list<Register>& regs);
    
private:
    struct ModbusMemory {
        int offset_bits = 0;
        int offset_input_bits = 0;
        int offset_regs = 0;
        int offset_input_regs = 0;
        int length_bits = 0;
        int length_input_bits = 0;
        int length_regs = 0;
        int length_input_regs = 0;
        modbus_mapping_t* memory;
    };

    struct Query {
        uint8_t query[MB_QUERY_LENGTH];
        int length;
    };

    ModbusConnection& m_connection;
    modbus_t* m_ctx;
    ModbusMemory m_mem;
    Query m_query;
    int m_listen_socket;
    int m_client_socket;
    std::vector<int> m_client_sockets;
};

} // action
} // mb

#endif // MB_MODBUS_SLAVE_H
