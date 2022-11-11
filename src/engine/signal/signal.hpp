#pragma once

#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdint>

template <typename... ARGS> class Signal;
template <typename... ARGS> class Connection;

template <typename... ARGS> class Connection {
    static inline std::uint32_t gid{0ull};
    const std::uint32_t id{0};
    Signal<ARGS...> *signal{nullptr};

  public:
    Connection(Signal<ARGS...> *s) : id{gid++}, signal{s} {}

    void disconnect() { signal->disconnect(*this); }

    operator bool() const { return !!signal; }

  public:
    friend class Signal<ARGS...>;
};

template <typename... ARGS> class Signal {
  public:
    typedef Connection<ARGS...> ConnectionType;

  private:
    using func_t = std::function<void(ARGS...)>;

    struct Slot {
        Slot(std::uint32_t id, func_t &&f) : conn_id(id), func(std::move(f)) {}
        Slot(Slot &&s) noexcept { *this = std::move(s); }
        Slot &operator=(Slot &&s) noexcept {
            conn_id = s.conn_id;
            func    = std::move(s.func);
            return *this;
        }
        Slot(const Slot &s) noexcept { *this = s; }
        Slot &operator=(const Slot &s) noexcept {
            conn_id = s.conn_id;
            func    = s.func;
            return *this;
        }

        std::uint32_t conn_id{0u};
        func_t func;
    };

  public:
    Connection<ARGS...> connect(func_t &&f) {
        Connection<ARGS...> c{this};
        slots.emplace_back(c.id, std::move(f));
        return c;
    }
    void disconnect(const Connection<ARGS...> &c) {
        slots.erase(std::remove_if(slots.begin(), slots.end(), [&c](const Slot &s) { return s.conn_id == c.id; }),
                    slots.end());
    }

    template <typename... EMIT_ARGS> void emit(EMIT_ARGS &&...args) {
        for (const auto &s : slots) { s.func(std::forward<EMIT_ARGS>(args)...); }
    }

  private:
    std::vector<Slot> slots;

    friend class Connection<ARGS...>;
};