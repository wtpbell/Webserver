#include <chrono>
#include <iostream>
#include <thread>

#include "Client.hpp"
#include "utils/Logger.hpp"

int main(int argc, char* argv[])
{
  using namespace std::chrono_literals;
  if (argc != 3)
    return (1);

  Client client;

  try
  {
    client.Connect(argv[1], argv[2]);
    Logger::Log(LogLevel::INFO, "Connection established with {}:{}", argv[1], argv[2]);
  }
  catch (const std::exception& e)
  {
    Logger::Log(LogLevel::ERROR, "{}", e.what());
    return (1);
  }

  std::string line;
  while (true)
  {
    std::cout << "type message to send to the server: " << std::endl;
    if (!std::getline(std::cin, line) || line.compare("\\q") == 0)
      break;

    try
    {
      line.append("\n");
      if (!client.Ping(line))
        break;

      std::this_thread::sleep_for(500ms);

      std::cout << "\nserver response: " << std::endl;
      if (!client.Pong())
        break;
    }
    catch (const std::exception& e)
    {
      Logger::Log(LogLevel::ERROR, "{}", e.what());
      return (1);
    }
  }
  return (0);
}
