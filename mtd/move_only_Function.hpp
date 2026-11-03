#pragma once

#include <functional>
#include <memory>

namespace mtd
{
    template <typename FnSig>
    class move_only_Function
    {
        static_assert(!std::is_same_v<FnSig,FnSig> , "invalid function signature");
    };

    template <typename Ret , typename... Args>
    class move_only_Function<Ret(Args...)>
    {
        struct FuncBase
        {
            virtual Ret call(Args...) = 0;

            virtual ~FuncBase() = default;
        };

        template <typename Func>
        struct FuncImpl : FuncBase
        {
            Func m_func;

            template <typename... CArgs>
            explicit FuncImpl(std::in_place_t , CArgs&&... args) : m_func(std::forward<CArgs>(args)...) {}

            Ret call(Args... args) override
            {
                return std::invoke(m_func , std::forward<Args>(args)...);
            }
        };

        std::unique_ptr<FuncBase> m_base;

    public:
        move_only_Function() noexcept = default;

        move_only_Function(std::nullptr_t) noexcept : move_only_Function() {}

        template <typename Func>
            requires (std::is_invocable_r_v<Ret,Func&,Args...>)
        move_only_Function(Func func)
            : m_base(std::make_unique<FuncImpl<Func>>(std::in_place , std::move(func))) {}

        template <typename Func , typename... CArgs>
            requires(std::constructible_from<Func,CArgs...> && std::is_invocable_r_v<Ret,Func&,Args...>)
        move_only_Function(std::in_place_type_t<Func> , CArgs&&... args)
            : m_base(std::make_unique<FuncImpl<std::decay_t<Func>>>(std::in_place , std::forward<CArgs>(args)...)) {}

        move_only_Function(const move_only_Function& other) = delete;

        move_only_Function& operator=(std::nullptr_t) noexcept
        {
            m_base.reset();
            return *this;
        }

        move_only_Function& operator=(const move_only_Function& other) = delete;

        move_only_Function(move_only_Function&&) noexcept = default;

        move_only_Function& operator=(move_only_Function&&) noexcept = default;

        explicit operator bool() const noexcept
        {
            return m_base != nullptr;
        }

        bool operator==(std::nullptr_t) const noexcept
        {
            return m_base == nullptr;
        }

        bool operator!=(std::nullptr_t) const noexcept
        {
            return m_base != nullptr;
        }

        Ret operator()(Args... args) const
        {
            if (!m_base)[[unlikely]]
                    throw std::bad_function_call();
            return m_base->call(std::forward<Args>(args)...);
        }

        void swap(move_only_Function& other) noexcept
        {
            m_base.swap(other.m_base);
        }
    };
}
