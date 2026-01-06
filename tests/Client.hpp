/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/01 15:02:48 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/28 14:10:19 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>

#include "io/Socket.hpp"

class Client
{
  public:
    Client(void) = default;
    ~Client(void);

    Client(const Client& other) = delete;
    Client(Client&& other) noexcept = delete;
    Client& operator=(const Client& rhs) = delete;
    Client& operator=(Client&& rhs) noexcept = delete;

    void Connect(const char* host, const char* port);
    bool Ping(const std::string& message);
    bool Pong(void);

  private:
    Socket socket_;
};

#endif  // CLIENT_H_
