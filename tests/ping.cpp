#include <chrono>
#include <iostream>
#include <thread>

#include "Client.hpp"

int main(int argc, char* argv[])
{
  using namespace std::chrono_literals;
  if (argc != 3)
    return (1);

  Client client;

  try
  {
    client.Connect(argv[1], argv[2]);
    std::cout << "Connection established with " << argv[1] << ":" << argv[2] << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
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
      client.Ping(line.data());
      std::this_thread::sleep_for(500ms);

      std::cout << "\nserver response: " << std::endl;
      client.Pong();
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << '\n';
      return (1);
    }
  }
  return (0);
}
