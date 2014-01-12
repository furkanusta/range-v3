// Boost.Range library
//
//  Copyright Eric Niebler 2013.
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef RANGES_V3_UTILITY_SAFE_INT_HPP
#define RANGES_V3_UTILITY_SAFE_INT_HPP

#include <limits>
#include <type_traits>
#include <range/v3/utility/concepts.hpp>

namespace ranges
{
    inline namespace v3
    {
        template<typename SignedInteger>
        struct safe_int
        {
        private:
            CONCEPT_ASSERT(SignedIntegral<SignedInteger>());
            static constexpr SignedInteger pos_inf_ =
                std::numeric_limits<SignedInteger>::max();
            // Use -max instead of min for 2's complement weirdness.
            static constexpr SignedInteger neg_inf_ =
                -std::numeric_limits<SignedInteger>::max();
            static constexpr SignedInteger NaN_ =
                std::numeric_limits<SignedInteger>::min();
            static_assert(NaN_ != neg_inf_,
                "Are you really running on a machine that doesn't do 2's complement? "
                "Like, really?");

            SignedInteger i_ = 0;
        public:
            constexpr safe_int(SignedInteger i) noexcept
              : i_(i < neg_inf_ ? neg_inf_ : i)
            {}
            constexpr SignedInteger get() const noexcept
            {
                return i_;
            }
            static constexpr safe_int inf() noexcept
            {
                return pos_inf_;
            }
            static constexpr safe_int NaN() noexcept
            {
                return NaN_;
            }
            constexpr bool is_finite() const noexcept
            {
                return i_ < pos_inf_ && i_ > neg_inf_;
            }
            constexpr bool is_NaN() const noexcept
            {
                return i_ == NaN_;
            }
            constexpr safe_int operator-() const noexcept
            {
                return i_ == NaN_ ? NaN_ : -i_;
            }
            safe_int & operator++() noexcept
            {
                switch(i_)
                {
                case pos_inf_:  break;
                case neg_inf_:  break;
                case NaN_:      break;
                default:        ++i_;
                }
                return *this;
            }
            safe_int & operator++(int) noexcept
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }
            safe_int & operator--() noexcept
            {
                switch(i_)
                {
                case pos_inf_:  break;
                case neg_inf_:  break;
                case NaN_:      break;
                default:        --i_;
                }
                return *this;
            }
            safe_int & operator--(int) noexcept
            {
                auto tmp = *this;
                --*this;
                return tmp;
            }
            friend constexpr bool operator==(safe_int left, safe_int right) noexcept
            {
                return left.i_ != NaN_ && right.i_ != NaN_ && left.i_ == right.i_;
            }
            friend constexpr bool operator!=(safe_int left, safe_int right) noexcept
            {
                return left.i_ == NaN_ || right.i_ == NaN_ || left.i_ != right.i_;
            }
            friend constexpr bool operator<(safe_int left, safe_int right) noexcept
            {
                return left != NaN() && right != NaN() && left.i_ < right.i_;
            }
            friend constexpr bool operator<=(safe_int left, safe_int right) noexcept
            {
                return left != NaN() && right != NaN() && left.i_ <= right.i_;
            }
            friend constexpr bool operator>(safe_int left, safe_int right) noexcept
            {
                return left != NaN() && right != NaN() && left.i_ > right.i_;
            }
            friend constexpr bool operator>=(safe_int left, safe_int right) noexcept
            {
                return left != NaN() && right != NaN() && left.i_ >= right.i_;
            }
            friend constexpr safe_int operator+(safe_int left, safe_int right) noexcept
            {
                return (right == NaN() || left == NaN()) ? NaN() :
                       (!left.is_finite() && !right.is_finite()) ? (left == right ? left : NaN()) :
                       (!left.is_finite() && right.is_finite()) ? left :
                       (left.is_finite() && !right.is_finite()) ? right :
                       (right.i_ >= 0 && left.i_ > pos_inf_ - right.i_) ? inf() :
                       (right.i_ < 0 && left.i_ < neg_inf_ - right.i_) ? -inf() :
                       left.i_ + right.i_;
            }
            friend constexpr safe_int operator-(safe_int left, safe_int right) noexcept
            {
                return left + -right;
            }
            safe_int & operator+=(safe_int that)
            {
                *this = *this + that;
                return *this;
            }
            safe_int & operator-=(safe_int that)
            {
                *this = *this - that;
                return *this;
            }
            friend constexpr safe_int operator/(safe_int left, safe_int right)
            {
                return (left.is_NaN() || right.is_NaN()) ? NaN() :
                       (left == 0 && right == 0) ? NaN() :
                       (left.is_finite() && right == 0) ? (left < 0 ? -inf() : inf()) :
                       (left.is_finite() && !right.is_finite()) ? (left < 0 ? -right : right) :
                       (!left.is_finite() && right == 0) ? left :
                       (!left.is_finite() && right.is_finite()) ? (right < 0 ? -left : left) :
                       left.i_ / right.i_;
            }
            safe_int & operator/=(safe_int that)
            {
                *this = *this / that;
                return *this;
            }
            // TODO multiplication
        };

        template<typename SignedInteger>
        constexpr SignedInteger safe_int<SignedInteger>::pos_inf_;

        template<typename SignedInteger>
        constexpr SignedInteger safe_int<SignedInteger>::neg_inf_;

        template<typename SignedInteger>
        constexpr SignedInteger safe_int<SignedInteger>::NaN_;
    }
}

namespace std
{
    template<typename T>
    struct is_integral<ranges::v3::safe_int<T>>
      : is_integral<T>
    {};

    template<typename T>
    class numeric_limits<ranges::v3::safe_int<T>>
      : public numeric_limits<T>
    {
        using safe_int = ranges::v3::safe_int<T>;
    public:
        static constexpr bool is_specialized = true;
        static constexpr safe_int min() noexcept { return safe_int{-numeric_limits<T>::max() + 1}; }
        static constexpr safe_int max() noexcept { return safe_int{numeric_limits<T>::max() - 1}; }
        static constexpr safe_int lowest() noexcept { return min(); }
        static constexpr bool has_infinity = true;
        static constexpr safe_int infinity() noexcept { return safe_int::inf(); }
        static constexpr bool has_quiet_NaN = true;
        static constexpr safe_int quiet_NaN() noexcept { return safe_int::NaN(); }
    };

    template<typename T>
    constexpr bool numeric_limits<ranges::v3::safe_int<T>>::is_specialized;

    template<typename T>
    constexpr bool numeric_limits<ranges::v3::safe_int<T>>::has_infinity;

    template<typename T>
    constexpr bool numeric_limits<ranges::v3::safe_int<T>>::has_quiet_NaN;
}

#endif
