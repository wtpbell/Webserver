/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/01 15:02:48 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/01 15:17:37 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H_
#define CLIENT_H_

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
    void Ping(const char* msg);
    void Shutdown(void);

  private:
    int clientfd_ = -1;
};

#endif  // CLIENT_H_
