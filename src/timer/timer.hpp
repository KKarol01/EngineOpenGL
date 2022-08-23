#pragma once

#include <iostream>
#include <cstdint>
#include <chrono>
#include <stack>
#include <queue>
#include <map>
#include <vector>
#include <ratio>
#include <string>
#include <numeric>

template <typename TIMEFORMAT = std::nano> class Timer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point last_time;
    std::string name{""};
    std::queue<std::chrono::duration<long long, TIMEFORMAT>> durations;
    std::deque<std::string> duration_names;
    std::map<std::string, std::vector<std::chrono::duration<long long, TIMEFORMAT>>> duration_avgs;

  public:
    Timer() = default;
    Timer(std::string &&name) : name{std::move(name)} { last_time = clock::now(); }

    inline void start() { last_time = clock::now(); }
    inline auto step() {
        durations.emplace(clock::now() - last_time);
        duration_names.emplace_back("");
        last_time = clock::now();

        return [this, &top_time = durations.back()](std::string &&name = "") {
            duration_names.pop_back();
            duration_avgs[name].push_back(top_time);
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
        std::cout << '\n';
        for (auto &[k, v] : duration_avgs) {
            float sum = std::reduce(v.begin(), v.end(), 0, [](auto &&a, auto &&b) { return a + b.count(); });
            sum /= static_cast<float>(v.size());

            std::cout << k << ": " << sum << '\n';
        }
    }
};
