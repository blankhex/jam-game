#pragma once

#include "Common.h"

#include "Entity.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace NGame {

class TGridmap {
public:
    std::unordered_set<TEntity::TId> Query(const Vec2i& position, const Vec2i& size) const;
    void Remove(const Vec2i& position, const Vec2i& size, TEntity::TId id);
    void Add(const Vec2i& position, const Vec2i& size, TEntity::TId id);
    void Reset();

private:
    using TId = size_t;
    TId GridId(const Vec2i& point) const;

private:
    std::array<std::vector<TEntity::TId>, 32768> Grid_;
    const std::uint32_t CellSize_ = 128;
};

} // namespace NGame