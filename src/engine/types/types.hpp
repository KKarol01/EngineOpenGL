#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <algorithm>
#include <utility>
namespace eng {
    template <typename T> using OptionalReference = std::optional<std::reference_wrapper<T>>;

    struct UNIQUE_INSERT;
    struct NON_UNIQUE_INSERT;
    template <typename INSERT_BHV, typename DATA_TYPE, typename COMP = std::less<>> class SortedVector_TMPL {
        std::vector<DATA_TYPE> vec;

      public:
        SortedVector_TMPL() = default;
        SortedVector_TMPL(const std::vector<DATA_TYPE> &d) { *this = d; }
        SortedVector_TMPL(std::vector<DATA_TYPE> &&d) { *this = std::move(d); }
        SortedVector_TMPL(const SortedVector_TMPL &d) { *this = d; }
        SortedVector_TMPL(SortedVector_TMPL &&d) { *this = std::move(d); }
        SortedVector_TMPL &operator=(const std::vector<DATA_TYPE> &d) {
            vec = d;
            std::sort(vec.begin(), vec.end(), COMP{});
            return *this;
        }
        SortedVector_TMPL &operator=(std::vector<DATA_TYPE> &&d) {
            vec = std::move(d);
            std::sort(vec.begin(), vec.end(), COMP{});
            return *this;
        }
        SortedVector_TMPL &operator=(const SortedVector_TMPL &other) {
            vec = other.vec;
            return *this;
        }
        SortedVector_TMPL &operator=(SortedVector_TMPL &&other) {
            vec = std::move(other.vec);
            return *this;
        }
        DATA_TYPE &operator[](std::size_t id) { return vec[id]; }
        const DATA_TYPE &operator[](std::size_t id) const { return vec.at(id); }

        constexpr DATA_TYPE &insert(const DATA_TYPE &d) {
            if constexpr (std::is_same_v<INSERT_BHV, UNIQUE_INSERT>) {
                auto f_it = find_element_idx(d);
                if (f_it != vec.end() && d == (*f_it)) return *f_it;
            }
            auto it = get_insertion_idx(d);
            return *vec.insert(it, d);
        }

        constexpr DATA_TYPE &insert(DATA_TYPE &&d) {
            if constexpr (std::is_same_v<INSERT_BHV, UNIQUE_INSERT>) {
                auto f_it = find_element_idx(d);
                if (f_it != vec.end() && d == (*f_it)) return *f_it;
            }
            auto it = get_insertion_idx(d);
            return *vec.emplace(it, std::move(d));
        }

        constexpr void remove(const DATA_TYPE &d) {
            auto it = find_element_idx(d);
            if (it != vec.end() && *it == d) vec.erase(it);
        }

        template <typename VAL, typename PRED = COMP> constexpr decltype(auto) find(VAL &&v, PRED p = COMP{}) {
            return std::lower_bound(vec.begin(), vec.end(), v, [&p](auto &&e, auto &&v) { return p(e, v); });
        }
        template <typename VAL, typename PRED = COMP> constexpr bool contains(VAL &&v, PRED p = COMP{}) {
            return find(v, p) != vec.end();
        }

        template <typename VAL, typename PRED> constexpr void remove(VAL &&v, PRED p = PRED{}) {
            auto it = find(v, p);
            if (it != vec.end() && v == *it) vec.erase(it);
        }

        template <typename... ARGS> constexpr DATA_TYPE &emplace(ARGS &&...args) {
            return insert(DATA_TYPE{std::forward<ARGS>(args)...});
        }

        constexpr auto &data() { return vec; }
        constexpr const auto &data() const { return vec; }
        constexpr auto begin() { return vec.begin(); }
        constexpr auto end() { return vec.end(); }
        constexpr auto cbegin() const { return vec.cbegin(); }
        constexpr auto cend() const { return vec.cend(); }

      private:
        constexpr auto find_element_idx(const DATA_TYPE &d, COMP comp = COMP{}) {
            return std::lower_bound(vec.begin(), vec.end(), d, comp);
        }
        constexpr auto get_insertion_idx(const DATA_TYPE &d, COMP comp = COMP{}) {
            return std::upper_bound(vec.begin(), vec.end(), d, comp);
        }
    };

    template <typename DATA_TYPE, typename COMP = std::less<>>
    using SortedVector = SortedVector_TMPL<NON_UNIQUE_INSERT, DATA_TYPE, COMP>;

    template <typename DATA_TYPE, typename COMP = std::less<>>
    using SortedVectorUnique = SortedVector_TMPL<UNIQUE_INSERT, DATA_TYPE, COMP>;
} // namespace eng
