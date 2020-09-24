#include <optional>

struct QueueFamilyIndices
{
public:
    const bool IsComplete() const
    {
        return graphicsFamily.has_value();
    }
public:
    std::optional<uint32_t> graphicsFamily;
};