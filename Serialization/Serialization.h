#pragma once
#include <concepts>
#include <iostream>

template <typename Archive, typename Ty>
concept Serializable = requires(Archive& archive, const Ty& value) {
    {
        value.Serialize(archive)
    } -> std::same_as<Archive&>;
};

template <typename Archive, typename Ty>
concept Deserializable = requires(const Archive& archive, Ty& value) {
    {
        value.Deserialize(archive)
    } -> std::same_as<const Archive&>;
};

template <typename Archive, typename Ty>
    requires Serializable<Archive, Ty>
Archive& Serialize(Archive& archive, const Ty& value)
{
    return value.Serialize(archive);
}

template <typename Archive, typename Ty>
    requires Deserializable<Archive, Ty>
const Archive& Deserialize(const Archive& archive, Ty& value)
{
    return value.Deserialize(archive);
}

template <typename Archive, typename Ty>
    requires Serializable<Archive, Ty>
Archive& operator<<(Archive& archive, const Ty& value)
{
    return value.Serialize(archive);
}

template <typename Archive, typename Ty>
    requires Deserializable<Archive, Ty>
const Archive& operator>>(const Archive& archive, Ty& value)
{
    return value.Deserialize(archive);
}

template <typename Archive, typename Ty>
    requires Serializable<Archive, Ty>
void SerializeTypeless(Archive* archive, const void* ptr)
{
    if (archive == nullptr || ptr == nullptr)
    {
        return;
    }

    reinterpret_cast<const Ty*>(ptr)->Serialize(*archive);
}

template <typename Archive, typename Ty>
    requires Deserializable<Archive, Ty>
void DeserializeTypeless(const Archive* archive, void* ptr)
{
    if (archive == nullptr || ptr == nullptr)
    {
        return;
    }

    reinterpret_cast<Ty*>(ptr)->Deserialize(*archive);
}