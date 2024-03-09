#include <iostream>

#include "NetClient.hpp"
#include "Logger/ILogger.hpp"

using namespace avlComms;

class ConsoleLogger : public cutils::ILogger
{
public:
    void Log(const std::string &message, const std::string &module, const std::string &method, int line, LogLevel level) override
    {
        std::cout << "[" << module << ":" << method << ":" << line << "] " << message << std::endl;
    }
};

int main()
{
    ConsoleLogger logger;
    NetConfiguration netConfig(999, "127.0.0.1", 40941);
    NetClient client(netConfig, &logger);
    client.Init();
    client.RequestConnection();

    while (true)
    {
        /* code */
    }

    std::cout << "GPS pkg size: " << sizeof(NetPkgGps) << std::endl;
    // avl.UpdatePosition(pos);
    return 0;
}