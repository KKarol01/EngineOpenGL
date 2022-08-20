#pragma once

#include <iostream>
#include <cstdint>
#include <chrono>
#include <stack>
#include <queue>
#include <ratio>
#include <string>

template <typename TIMEFORMAT = std::nano> class Timer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point last_time;
    std::string name{""};
    std::queue<std::chrono::duration<long long, TIMEFORMAT>> durations;
    std::deque<std::string> duration_names;

  public:
    Timer() = default;
    Timer(std::string &&name) : name{std::move(name)} { last_time = clock::now(); }

    inline void start() { last_time = clock::now(); }
    inline auto step() {
        durations.emplace(clock::now() - last_time);
        duration_names.emplace_back("");
        last_time = clock::now();

        return [this](std::string &&name = "") {
            duration_names.pop_back();
            duration_names.emplace_back(std::move(name));
            last_time = clock::now();
        };
    }
    inline auto end() {
        step()("end");
        print();
    }

  private:
    void print() {
        std::cout << "=== " << name << " ===\n";
        while (durations.empty() == false) {
            std::cout << "\t" << duration_names.front() << ": " << durations.front().count() << '\n';
            durations.pop();
            duration_names.pop_front();
        }
    }
};
