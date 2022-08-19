#pragma once

#include <iostream>
#include <cstdint>
#include <chrono>
#include <stack>
#include <ratio>
#include <string>

class Timer {
    using clock = std::chrono::high_resolution_clock;

    struct Snapshot {
        Snapshot() { time = clock::now(); }
        std::chrono::steady_clock::time_point time;
    };

    inline static std::stack<Snapshot> snaps;
    inline static std::stack<std::string> names;

  public:
    struct Time {
        std::string name;
        std::chrono::duration<long long, std::micro> time;
        std::uint32_t indent{0u};
        Time(std::string &&name, std::chrono::duration<long long, std::micro> t, std::uint32_t indent = 0)
            : name{std::move(name)}, time{t}, indent{indent} {}
    };

    static void start(const std::string &name) {
        snaps.emplace();
        names.push(name);
    }
    static auto step() {
        return Time{std::string{names.top()},
                    std::chrono::duration_cast<std::chrono::microseconds>(Snapshot{}.time - snaps.top().time),
                    (uint32_t)(snaps.size() - 1)};
    }
    static auto end() {
        auto res = step();
        snaps.pop();
        names.pop();
        return res;
    }
};

std::ostream &operator<<(std::ostream &s, const Timer::Time &time) {
    for (auto i = 0u; i < time.indent; ++i) s << '\t';
    s << time.name << ": " << time.time.count() << '\n';
    return s;
}
