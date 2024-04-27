#pragma once
#include <entt/entt.hpp>
#include <string_view>

using namespace entt::literals;
using namespace std::literals;

template <typename T>
void DefineMeta();

template <typename T>
void* AddComponent(entt::registry* registry, const entt::entity entity)
{
    if (registry == nullptr || entity == entt::null)
    {
        return nullptr;
    }

    if (registry->try_get<T>(entity) != nullptr)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(&registry->emplace<T>(entity));
}

template <typename T>
void RemoveComponent(entt::registry* registry, const entt::entity entity)
{
    if (registry == nullptr || entity == entt::null)
    {
        return;
    }

    registry->remove<T>(entity);
}

#define IG_DECLARE_TYPE_META(T)     \
    struct T##_DefineMeta           \
    {                               \
        T##_DefineMeta();           \
                                    \
    private:                        \
        static T##_DefineMeta _reg; \
    };

#define IG_DEFINE_TYPE_META(T)               \
    T##_DefineMeta::T##_DefineMeta()         \
    {                                        \
        entt::meta<T>().prop("Name"_hs, #T); \
        DefineMeta<T>();                     \
    }                                        \
    T##_DefineMeta T##_DefineMeta::_reg;

// 특정 형태의 타입에 대해서, 기본적으로 몇가지 자유 함수에 대한 정보 추가
#define IG_DEFINE_TYPE_META_AS_COMPONENT(T)                                       \
    T##_DefineMeta::T##_DefineMeta()                                              \
    {                                                                             \
        entt::meta<T>().prop("Name"_hs, #T);                                      \
        entt::meta<T>().prop("Component"_hs);                                     \
        entt::meta<T>().template func<&AddComponent<T>>("AddComponent"_hs);       \
        entt::meta<T>().template func<&RemoveComponent<T>>("RemoveComponent"_hs); \
        DefineMeta<T>();                                                          \
    }                                                                             \
    T##_DefineMeta T##_DefineMeta::_reg;

#define IG_DEFINE_META_TO_JSON(T)   entt::meta<T>().template func<&SerializeTypeless<nlohmann::json, T>>("ToJson"_hs)
#define IG_DEFINE_META_FROM_JSON(T) entt::meta<T>().template func<&DeserializeTypeless<nlohmann::json, T>>("FromJson"_hs)