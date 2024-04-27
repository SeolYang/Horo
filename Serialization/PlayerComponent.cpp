#include "PlayerComponent.h"
#include "Serialization.h"

template <>
void DefineMeta<Player>()
{
    entt::meta<Player>().prop("JsonSerializable"_hs);
    entt::meta<Player>().prop("BebopSerializable"_hs);
    IG_DEFINE_META_TO_JSON(Player);
    IG_DEFINE_META_FROM_JSON(Player);
}

IG_DEFINE_TYPE_META_AS_COMPONENT(Player);
