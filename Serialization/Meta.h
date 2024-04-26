#pragma once
#include <entt/entt.hpp>
#include <string_view>

using namespace entt::literals;
using namespace std::literals;

template <typename T>
void DefineMeta();

#define IG_DECLARE_META(T)          \
    struct T##_DefineMeta           \
    {                               \
        T##_DefineMeta();           \
                                    \
    private:                        \
        static T##_DefineMeta _reg; \
    };

#define IG_DEFINE_META(T)            \
    T##_DefineMeta::T##_DefineMeta() \
    {                                \
        DefineMeta<T>();             \
    }                                \
    T##_DefineMeta T##_DefineMeta::_reg;