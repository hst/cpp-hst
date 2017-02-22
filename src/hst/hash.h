/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_HASH_H
#define HST_HASH_H

#include <functional>

namespace hst {

// You can't do anything with these; they just exist, and give you a nice basis
// for forming distinct hash values.
class hash_scope {
};

class hasher {
  public:
    explicit hasher(const hash_scope& scope)
        : hash_(std::hash<const void*>()(&scope))
    {
    }

    template <typename T>
    hasher& add(const T& value)
    {
        hash_ ^= std::hash<T>()(value) + 0x9e3779b9 + (hash_ << 6) +
                 (hash_ >> 2);
        return *this;
    }

    template <typename T>
    hasher& add_unordered(const T& value)
    {
        hash_ ^= std::hash<T>()(value);
        return *this;
    }

    std::size_t value() const { return hash_; }

  private:
    std::size_t hash_;
};

}  // namespace hst
#endif  // HST_HASH_H
