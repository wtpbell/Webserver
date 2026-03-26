/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Expected.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/12 13:48:44 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/12 13:48:45 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPECTED_H_
#define EXPECTED_H_

#include <utility>

/*
 * Class inspired by:
 *   - https://en.cppreference.com/w/cpp/utility/expected.html (c++23)
 *   - https://en.cppreference.com/w/cpp/utility/optional.html (c++17)
 *
 * Additional source:
 *   - https://www.club.cc.cmu.edu/~ajo/disseminate/2017-02-15-Optional-From-Scratch.pdf
 */

template <typename Val, typename Unex>
class Expected
{
  public:
    Expected(void) = delete;
    Expected(Val&& value) noexcept : value_(std::move(value)), has_value_(true) {}
    Expected(Unex&& error) noexcept : error_(std::move(error)), has_value_(false) {}
    ~Expected(void)
    {
      if (has_value_)
        value_.~Val();
      else
        error_.~Unex();
    };

    Expected(const Expected& other)
    {
      if (other.has_value_)
        value_ = other.value_;
      else
        error_ = other.error_;
      has_value_ = other.has_value_;
    }

    Expected(Expected&& other) noexcept
    {
      if (other.has_value_)
        value_ = std::move(other.value_);
      else
        error_ = std::move(other.error_);
      has_value_ = other.has_value_;
    }

    Expected& operator=(const Expected& rhs)
    {
      if (this == &rhs)
        return *this;

      if (has_value_)
        value_.~Val();
      else
        error_.~Unex();

      if (rhs.has_value_)
        value_ = rhs.value_;
      else
        error_ = rhs.error_;

      has_value_ = rhs.has_value_;
      return *this;
    }

    Expected& operator=(Expected&& rhs) noexcept
    {
      if (this == &rhs)
        return *this;

      if (has_value_)
        value_.~Val();
      else
        error_.~Unex();

      if (rhs.has_value_)
        value_ = std::move(rhs.value_);
      else
        error_ = std::move(rhs.error_);

      has_value_ = rhs.has_value_;
      return *this;
    }

    Val* operator->(void)
    {
      return &value_;
    }

    const Val* operator->(void) const
    {
      return &value_;
    }

    Val* operator*(void)
    {
      return &value_;
    }

    const Val* operator*(void) const
    {
      return &value_;
    }

    Val& GetValue(void)
    {
      return value_;
    }

    const Val& GetValue(void) const
    {
      return value_;
    }

    Unex& GetError(void)
    {
      return error_;
    }

    const Unex& GetError(void) const
    {
      return error_;
    }

    Val&& ExtractValue(void)
    {
      return std::move(value_);
    }

    bool HasValue(void)
    {
      return has_value_;
    }

  private:
    union
    {
        Val value_;
        Unex error_;
    };
    bool has_value_;
};

#endif
