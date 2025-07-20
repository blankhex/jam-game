#pragma once

#include "Common.h"

#include <cstdint>
#include <queue>
#include <unordered_map>

namespace NGame {

class TAlarm {
public:
    using TId = int;

    void Set(TId id, std::uint32_t millis);
    void Unset(TId id);
    void Update(std::uint32_t delta);
    bool Next(TId& id);
    bool IsSet(TId id) const;

private:
    struct TData {
        std::uint32_t Target;
        std::uint32_t Current;
    };

    std::unordered_map<TId, TData> Alarms_;
    std::queue<TId> DueAlarms_;
};

}