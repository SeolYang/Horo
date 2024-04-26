#include "PlayerComponent.h"

template <>
void DefineMeta<Player>()
{
    entt::meta<Player>().type("Test"_hs).prop("TestProp"_hs, "Yahh"sv);
    entt::meta<Player>().prop("Serializable"_hs, std::vector<std::string_view>{"Json"sv, "Bebop"sv});
}

IG_DEFINE_META(Player);