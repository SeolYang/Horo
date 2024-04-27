#include <catch2/catch_test_macros.hpp>
#include "PlayerComponent.h"

TEST_CASE("Meta")
{
    REQUIRE(entt::resolve<Player>());
    REQUIRE(entt::resolve<Player>().prop("JsonSerializable"_hs));
    REQUIRE(entt::resolve<Player>().prop("BebopSerializable"_hs));
    REQUIRE(entt::resolve<Player>().func("ToJson"_hs));
    REQUIRE(entt::resolve<Player>().func("FromJson"_hs));

    entt::registry srcRegistry;
    entt::entity dummy = srcRegistry.create();
    srcRegistry.destroy(dummy);
    entt::entity playerEntity = srcRegistry.create();
    Player testData = srcRegistry.emplace<Player>(playerEntity, Player{ .HP = 100, .MP = 50 });

    nlohmann::json root;
    /* Registry Serialization */
    for (const auto [entity] : srcRegistry.storage<entt::entity>().each())
    {
        nlohmann::json entityRoot;
        for (auto&& [typeID, type] : entt::resolve())
        {
            entt::sparse_set* storage = srcRegistry.storage(typeID);
            const void* value = storage->value(entity);
            if (value != nullptr)
            {
                auto toJson = type.func("ToJson"_hs);
                if (toJson)
                {
                    nlohmann::json typeRoot;
                    typeRoot["ID"] = static_cast<uint64_t>(typeID);
                    toJson.invoke(type, &typeRoot, value);
                    entityRoot[type.prop("Name"_hs).value().cast<const char*>()] = typeRoot;
                }
            }
        }
        root[std::format("{}", entt::to_integral(entity))] = entityRoot;
    }
    /**************************************************************************************/

    nlohmann::json testEntityJson = root[std::format("{}", entt::to_integral(playerEntity))];
    REQUIRE(testEntityJson.is_object());
    nlohmann::json testPlayerCompJson = testEntityJson["Player"];
    REQUIRE(entt::type_hash<Player>::value() == (uint64_t)testPlayerCompJson["ID"]);
    REQUIRE(testData.HP == testPlayerCompJson["HP"]);
    REQUIRE(testData.MP == testPlayerCompJson["MP"]);

    /* Registry Deserialization */
    entt::registry dstRegistry;
    for (auto rootItr = root.begin(); rootItr != root.end(); ++rootItr)
    {
        const auto oldEntity = static_cast<entt::entity>(std::stoul(rootItr.key()));
        // 원래 사용되던 id로 entity 생성 못하는 경우.., 필요하다면 old<->new mapping 할 것!
        const auto newEntity = dstRegistry.create(oldEntity);
        nlohmann::json& entityRoot = rootItr.value();
        for (const nlohmann::json& componentRoot : entityRoot)
        {
            const auto componentTypeID = static_cast<entt::id_type >((uint64_t)componentRoot["ID"]);
            auto type = entt::resolve(componentTypeID);
            if (!type)
            {
                continue;
            }

            auto addComponent = type.func("AddComponent"_hs);
            if (!addComponent)
            {
                continue;
            }

            auto* component = addComponent.invoke(type, &dstRegistry, newEntity).cast<void*>();
            if (component == nullptr)
            {
                continue;
            }

            auto fromJson = type.func("FromJson"_hs);
            if (!fromJson)
            {
                continue;
            }

            // 기존 signature랑 완벽하게 일치하지 않으면 호출이 아에 안됨!
            fromJson.invoke(type, &componentRoot, component);
        }
    }

    Player& deserializedData = dstRegistry.get<Player>(playerEntity);
    REQUIRE(testData.HP == deserializedData.HP);
    REQUIRE(testData.MP == deserializedData.MP);
}