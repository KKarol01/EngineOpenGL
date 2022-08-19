#include <gtest/gtest.h>
#include "../src/engine/ecs/sorted_vec.hpp"

#include <functional>

static int *data;

struct TEST_STRUCT {
    float x{0.f};
    bool moved = false;

    TEST_STRUCT() = default;
    TEST_STRUCT(float a) : x{a} {}
    TEST_STRUCT &operator=(const TEST_STRUCT &other) noexcept {
        x = other.x;
        return *this;
    }
    TEST_STRUCT &operator=(TEST_STRUCT &&other) noexcept {
        x           = other.x;
        other.x     = -1.f;
        other.moved = true;
        return *this;
    }

    TEST_STRUCT(const TEST_STRUCT &other) { *this = other; }
    TEST_STRUCT(TEST_STRUCT &&other) { *this = std::move(other); }
};

bool operator==(const TEST_STRUCT &a, const TEST_STRUCT &b) { return a.x == b.x; }
bool operator<(const TEST_STRUCT &a, const TEST_STRUCT &b) { return a.x < b.x; }
bool operator>(const TEST_STRUCT &a, const TEST_STRUCT &b) { return !(a == b) && !(a < b); }

// Demonstrate some basic assertions.
TEST(HelloTest, insertion) {
    data = new int{4};

    SortedVector<TEST_STRUCT> structs;
    TEST_STRUCT a{1.f};
    TEST_STRUCT b{2.f};

    EXPECT_EQ(structs.insert(a), TEST_STRUCT{1.f}) << "insert(&) should return value that was given";
    EXPECT_EQ(structs.insert(std::move(b)), TEST_STRUCT{2.f}) << "insert(&&) should return value that was given";
    EXPECT_TRUE(b.moved == true && b.x == -1.f)
        << "value that was moved in insert(&&) should be affected by the move constructor";
    EXPECT_EQ(structs.emplace(3.f), TEST_STRUCT{3.f})
        << "value that was emplaced with emplace() should be properly constructed and returned";

    ASSERT_TRUE(structs.data().size() == 3)
        << "3 values were inserted into vector and that's what the underlying vector should contain";

    EXPECT_TRUE(structs.data()[0].x == 1.f)
        << "values should be ordered in ascending order with std::less comparator function";
    EXPECT_TRUE(structs.data()[1].x == 2.f)
        << "values should be ordered in ascending order with std::less comparator function";
    EXPECT_TRUE(structs.data()[2].x == 3.f)
        << "values should be ordered in ascending order with std::less comparator function";

    SortedVector<TEST_STRUCT, std::greater<void>> structs_descending;
    structs_descending = std::vector<TEST_STRUCT>{3.f, 1.f, 2.f};
    EXPECT_TRUE(structs_descending.data()[0].x == 3.f)
        << "values should be ordered in descending order with std::greater comparator function";
    EXPECT_TRUE(structs_descending.data()[1].x == 2.f)
        << "values should be ordered in descending order with std::greater comparator function";
    EXPECT_TRUE(structs_descending.data()[2].x == 1.f)
        << "values should be ordered in descending order with std::greater comparator function";

    delete data;
}