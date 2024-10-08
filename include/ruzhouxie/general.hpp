#ifndef RUZHOUXIE_GENERAL_H
#define RUZHOUXIE_GENERAL_H

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "macro_define.hpp"

#if !defined(__cpp_size_t_suffix) || __cpp_size_t_suffix <= 202006L
constexpr size_t operator""uz(unsigned long long x)
{
    return static_cast<size_t>(x);
}
#endif

namespace ruzhouxie
{
    namespace rzx = ::ruzhouxie;

    inline constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

    struct custom_t{};

	//type without cvref.
    template<typename T>
    concept pure = std::same_as<T, std::remove_cvref_t<T>>;

    template<typename T>
    using purified = std::remove_cvref_t<T>;

    template<auto fn>
    using tag_t = purified<decltype(fn)>;
	
	//completiable type.
    template<typename T>
    concept concrete = !std::same_as<T, void> && !std::is_abstract_v<T> && !std::is_unbounded_array_v<T>;

	//const object or ref to const object
    template<typename T>
    concept readonly = std::is_const_v<std::remove_reference_t<T>>;

    template<typename T>
    concept aggregated = std::is_aggregate_v<purified<T>>;

    template<typename T>
    concept empty = std::is_empty_v<T>;

    template<typename T>
    concept empty_aggregated = std::is_empty_v<T> && std::is_aggregate_v<T>;

    template<typename T>
    concept inheritable = std::is_class_v<T> && !std::is_final_v<T>;

    template<auto...>
    concept constevaluable = true;

    template<typename T, typename U>
    concept is = std::same_as<U, T> || std::derived_from<T, U>;

	//T is type of arbitrarily adding &, &&, const, volatile to TOriginal.
    template<typename T, typename TOriginal>
    concept specified = is<purified<T>, purified<TOriginal>> &&
		(!std::is_reference_v<TOriginal> || std::is_reference_v<T>) &&
		(!std::is_lvalue_reference_v<TOriginal> || std::is_lvalue_reference_v<T>) &&
		(!readonly<TOriginal> || readonly<T>) &&
		(!std::is_volatile_v<std::remove_reference_t<TOriginal>> || std::is_volatile_v<std::remove_reference_t<T>>);

    template<typename T>
    concept expiringed = std::is_rvalue_reference_v<T>;

	// T is type of non-const r-value reference.
    template<typename T>
    concept proprietary =
	    std::is_rvalue_reference_v<T>
		&&
		!std::is_const_v<std::remove_reference_t<T>>;

    template<typename T>
    T declvalue()noexcept;

    template<size_t I, typename...T>
    using type_at = std::tuple_element_t<I, std::tuple<T...>>;

    template<typename...T>
    RUZHOUXIE_INTRINSIC constexpr decltype(auto) fwd(auto&& arg) noexcept
	{
	    if constexpr (readonly<decltype(arg)>)
		{
		    using type = std::conditional_t<(... && std::is_rvalue_reference_v<T&&>),
			    const std::remove_reference_t<type_at<sizeof...(T) - 1, T...>>&&,
			    const std::remove_reference_t<type_at<sizeof...(T) - 1, T...>>&
			>;
		    return static_cast<type>(arg);
		}
	    else
		{
		    using type = std::conditional_t<(... && std::is_rvalue_reference_v<T&&>),
			    std::remove_reference_t<type_at<sizeof...(T) - 1, T...>>&&,
			    type_at<sizeof...(T) - 1, T...>&
			>;
		    return static_cast<type>(arg);
		}
	}

    template<typename TBase>
    RUZHOUXIE_INTRINSIC constexpr TBase& as_base(TBase& arg) noexcept
	{
	    return arg;
	}

    template<typename TBase>
    RUZHOUXIE_INTRINSIC constexpr const TBase& as_base(const TBase& arg) noexcept
	{
	    return arg;
	}

    template<typename TBase>
    RUZHOUXIE_INTRINSIC constexpr TBase&& as_base(TBase&& arg) noexcept
	{
	    return std::move(arg);
	}

    template<typename TBase>
    RUZHOUXIE_INTRINSIC constexpr const TBase&& as_base(const TBase&& arg) noexcept
	{
	    return std::move(arg);
	}

    template <typename T = bool>
    struct choice_t
	{
	    T strategy{};
	    bool nothrow = false;
	};

	template<typename T, std::convertible_to<bool> B>
	choice_t(T&&, B&&) -> choice_t<std::decay<T>>;

    template<typename T>
    struct wrapper
	{
	    T raw_value;

	    constexpr auto&& value(this auto&& self)noexcept
		{
		    return FWD(self, raw_value);
		}
	};

	template<typename T>
	wrapper(T&&) -> wrapper<T>;
}

namespace ruzhouxie
{

    template<size_t I>
    RUZHOUXIE_INLINE constexpr auto&& arg_at(auto&&...args)noexcept
	{
	    return std::get<I>(std::forward_as_tuple(FWD(args)...));
	}

    RUZHOUXIE_INLINE constexpr bool equal(auto&& x, auto&& y) 
	    noexcept(not requires{ FWD(x) == FWD(y); } || requires{ requires noexcept(FWD(x) == FWD(y)); })
	{
	    if constexpr(requires{ FWD(x) == FWD(y); })
		{
		    return FWD(x) == FWD(y);
		}
	    else
		{
		    return false;
		}
	}
}

#include "macro_undef.hpp"
#endif