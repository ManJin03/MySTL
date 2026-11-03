#pragma once

#include <functional>
#include <memory>

namespace mtd
{
    template <typename FnSig>
    class Function
    {
        static_assert(!std::is_same_v<FnSig,FnSig> , "invalid function signature");
    };

    template <typename Ret , typename... Args>
    class Function<Ret(Args...)>
    {
        struct FuncBase
        {
            virtual Ret call(Args...) = 0;

            [[nodiscard]] virtual std::unique_ptr<FuncBase> clone() const =0;

            [[nodiscard]] virtual std::type_info const& type() const =0;

            virtual ~FuncBase() = default;
        };

        template <typename Func>
        struct FuncImpl : FuncBase
        {
            Func m_func;

            template <typename... CArgs>
            explicit FuncImpl(std::in_place_t , CArgs... args) : m_func(std::forward<CArgs>(args)...) {}

            Ret call(Args... args) override
            {
                return std::invoke(m_func , std::forward<Args>(args)...);
            }

            [[nodiscard]] std::unique_ptr<FuncBase> clone() const override
            {
                return std::make_unique<FuncImpl>(std::in_place , m_func);
            }

            [[nodiscard]] std::type_info const& type() const override
            {
                return typeid(m_func);
            }
        };

        std::unique_ptr<FuncBase> m_base;

    public:
        Function() noexcept = default;

        Function(std::nullptr_t) noexcept : Function() {}

        template <typename Func>
            requires (std::is_invocable_r_v<Ret,Func,Args...>
                      && std::is_copy_constructible_v<Func>
                      && !std::is_same_v<std::decay_t<Func>,Function>)
        // ReSharper disable once CppNonExplicitConvertingConstructor
        Function(Func&& func) : m_base(std::make_unique<FuncImpl<std::decay_t<Func>>>(std::in_place , func)) {}

        Function(const Function& other) : m_base{other.m_base ? other.m_base->clone() : nullptr} {};

        Function& operator=(const Function& other)
        {
            if (other.m_base) {
                m_base = other.m_base->clone();
            }
            else {
                m_base = nullptr;
            }
            return *this;
        }

        Function(Function&&) = default;

        Function& operator=(Function&&) = default;

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

        [[nodiscard]] std::type_info const& target_type() const
        {
            return m_base ? m_base->type() : typeid(void);
        }

        template <class Fn>
        Fn* target() noexcept
        {
            return m_base && typeid(Fn) == m_base->type()
                       ? std::addressof(static_cast<FuncImpl<Fn>*>(m_base.get())->m_func)
                       : nullptr;
        }

        template <class Fn>
        const Fn* target() const noexcept
        {
            return m_base && typeid(Fn) == m_base->type()
                       ? std::addressof(static_cast<const FuncImpl<Fn>*>(m_base.get())->m_func)
                       : nullptr;
        }

        void swap(Function& other) noexcept
        {
            m_base.swap(other.m_base);
        }
    };
}
