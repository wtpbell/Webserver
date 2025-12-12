/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/25 12:08:23 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/05 11:52:15 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_H_
#define CONNECTION_H_

// TODO: uncomment and remove line 19 once EpollManager is merged
// #include "EpollManager.hpp"

class EpollManager;

class Connection
{
  public:
    Connection(void) = default;
    ~Connection(void) = default;

    Connection(const Connection& other) = default;
    Connection(Connection&& other) noexcept = default;
    Connection& operator=(const Connection& rhs) = delete;
    Connection& operator=(Connection&& rhs) noexcept = delete;

    void Clear() {};
};

#endif  // CONNECTION_H_
