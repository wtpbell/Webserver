/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   helper.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/25 10:59:19 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/13 19:07:53 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstddef>
#include <string>
#include <vector>

std::size_t NextPOT(std::size_t n)
{
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return (++n);
}

///@note: https://stackoverflow.com/a/26032303
std::vector<char*> ConvertToCstrVector(const std::vector<std::string>& v)
{
  std::vector<char*> c_vec;
  c_vec.reserve(v.size());
  for (const std::string& s : v)
    c_vec.emplace_back(const_cast<char*>(s.c_str()));
  c_vec.emplace_back(nullptr);
  return c_vec;
}
