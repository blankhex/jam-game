#include "Alarm.h"

namespace NGame {

void TAlarm::Set(TId id, std::uint32_t millis) {
    auto& data = Alarms_[id];
    data.Target = millis;
    data.Current = 0;
}

void TAlarm::Unset(TId id) {
    auto& data = Alarms_[id];
    data.Target = 0;
    data.Current = 0;
}

void TAlarm::Update(std::uint32_t delta) {
    for (auto it = Alarms_.begin(); it != Alarms_.end(); ++it) {
        auto& alarm = it->second;
        if (alarm.Target) {
            alarm.Current += delta;
        }

        if (alarm.Current >= alarm.Target) {
            DueAlarms_.push(it->first);
        }
    }
}

bool TAlarm::Next(TId& id) {
    bool repeat;
    
    do {
        repeat = false;

        if (DueAlarms_.empty())
            return false;

        id = DueAlarms_.front();
        DueAlarms_.pop();

        auto& alarm = Alarms_[id];
        if (alarm.Target) {
            if (alarm.Current >= alarm.Target) {
                alarm.Current -= alarm.Target;
            }
            
            if (alarm.Current >= alarm.Target) {
                DueAlarms_.push(id);
            }
        } else {
            alarm.Current = 0;
            repeat = true;
        }
    } while (repeat);

    return true;
}

bool TAlarm::IsSet(TId id) const {
    auto it = Alarms_.find(id);

    if (it == Alarms_.end()) {
        return false;
    }

    if (!it->second.Target) {
        return false;
    }

    return true;
}

} // namespace NGame
