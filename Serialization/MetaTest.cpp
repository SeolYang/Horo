#include <catch2/catch_test_macros.hpp>
#include "PlayerComponent.h"

TEST_CASE("Meta")
{
    REQUIRE(entt::resolve<Player>());
    REQUIRE(entt::resolve<Player>().prop("TestProp"_hs).value().cast<std::string_view>() == "Yahh");
    const std::vector<std::string_view>& serializableProp = *entt::resolve<Player>().prop("Serializable"_hs).value().try_cast<std::vector<std::string_view>>();
    REQUIRE(serializableProp.size() == 2);
    REQUIRE(serializableProp[0] == "Json");
    REQUIRE(serializableProp[1] == "Bebop");
}