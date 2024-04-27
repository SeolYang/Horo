#pragma once
#include "Meta.h"
#include <nlohmann/json.hpp>

struct Player
{
public:
    nlohmann::json& Serialize(nlohmann::json& root) const
    {
        root["HP"] = HP;
        root["MP"] = MP;
        return root;
    }

    const nlohmann::json& Deserialize(const nlohmann::json& root)
    {
        HP = root["HP"];
        MP = root["MP"];
        return root;
    }

public:
    uint32_t HP{ 0 };
    uint32_t MP{ 0 };
};

IG_DECLARE_TYPE_META(Player);

// entt::meta 에 정보 등록-> Json/Bebop 등 다른 형태의 Archive로 entt::meta의 정보를 순회해서 serialization
// entt::meta에 Serialize 자유 함수 추가?
// JsonSerialize, BebopSerialize...
// 특정 타입들은 그대로 Serialize 되거나 하면 안된다 (ex. 에셋; Serialize->Guid+Type->Deserialize->Instance)
// 굳이 외부 함수 아니더라도, 내부 static 함수로 Serialize 구현하는 방식도 괜찮을 것 같음