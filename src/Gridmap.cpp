#include "Gridmap.h"

#include <algorithm>
#include <unordered_set>

namespace NGame {

std::unordered_set<TEntity::TId> TGridmap::Query(const Vec2i& position, const Vec2i& size) const {
    Vec2i cellPosition = position / CellSize_;
    Vec2i cellEnd = cellPosition + ((size + (CellSize_ - 1)) / CellSize_);

    std::unordered_set<TEntity::TId> result;
    for (Vec2i::TType i = cellPosition.X; i <= cellEnd.X; ++i) {
        for (Vec2i::TType j = cellPosition.Y; j <= cellEnd.Y; ++j) {
            TId gridId = GridId(Vec2i(i, j));
            const auto& cell = Grid_[gridId];
            for (auto id : cell) {
                result.insert(id);
            }
        }
    }

    return result;
}

void TGridmap::Remove(const Vec2i& position, const Vec2i& size, TEntity::TId id) {
    Vec2i cellPosition = position / CellSize_;
    Vec2i cellEnd = cellPosition + ((size + (CellSize_ - 1)) / CellSize_);

    std::unordered_set<TEntity::TId> result;
    for (Vec2i::TType i = cellPosition.X; i <= cellEnd.X; ++i) {
        for (Vec2i::TType j = cellPosition.Y; j <= cellEnd.Y; ++j) {
            TId gridId = GridId(Vec2i(i, j));
            auto& cell = Grid_[gridId];
            auto it = std::remove(cell.begin(), cell.end(), id);
            cell.erase(it, cell.end());
        }
    }
}

void TGridmap::Add(const Vec2i& position, const Vec2i& size, TEntity::TId id) {
    Vec2i cellPosition = position / CellSize_;
    Vec2i cellEnd = cellPosition + ((size + (CellSize_ - 1)) / CellSize_);

    std::unordered_set<TEntity::TId> result;
    for (Vec2i::TType i = cellPosition.X; i <= cellEnd.X; ++i) {
        for (Vec2i::TType j = cellPosition.Y; j <= cellEnd.Y; ++j) {
            TId gridId = GridId(Vec2i(i, j));
            Grid_[gridId].push_back(id);
        }
    }
}

void TGridmap::Reset() {
    for (auto& cell : Grid_) {
        cell.clear();
    }
}

TGridmap::TId TGridmap::GridId(const Vec2i& point) const {
    return std::hash<Vec2i>{}(point) % Grid_.size();
}

} // namespace NGame