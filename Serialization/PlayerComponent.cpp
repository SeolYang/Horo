#include "PlayerComponent.h"
#include "Serialization.h"

template <>
void DefineMeta<Player>()
{
    IG_DEFINE_META_SERIALIZE_JSON(Player);
    IG_DEFINE_META_DESERIALIZE_JSON(Player);
}

IG_DEFINE_TYPE_META_AS_COMPONENT(Player);
