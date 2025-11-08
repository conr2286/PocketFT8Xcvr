#include <vector>

#include <unity.h>
#include <string>

#include "TinySet.h"

TinySet<unsigned> mySet(6);

/**
 * @brief This is the unity setup method executed prior to each test
 */
void setUp(void) {
}

/**
 * @brief Test insert of single element
 */
void test_insert(void) {
    TEST_ASSERT_EQUAL_UINT32(0, mySet.getSize());
    TEST_ASSERT_TRUE(mySet.insert(42));
    TEST_ASSERT_EQUAL_UINT32(1, mySet.getSize());
}

/**
 * @brief Test erase of single element
 * @note Must execute following test_insert()
 */
void test_erase(void) {
    TEST_ASSERT_EQUAL_UINT32(1, mySet.getSize());
    TEST_ASSERT_TRUE(mySet.erase(42));
    TEST_ASSERT_EQUAL_UINT32(0, mySet.getSize());
}

/**
 * @brief Test set of string members
 */
void test_string(void) {
    TinySet<std::string> foo;
    TEST_ASSERT_EQUAL_UINT32(0, foo.getSize());
    TEST_ASSERT_TRUE(foo.insert("FOO"));
    TEST_ASSERT_EQUAL_UINT32(1, foo.getSize());
    TEST_ASSERT_TRUE(foo.erase("FOO"));
    TEST_ASSERT_EQUAL_UINT32(0, foo.getSize());
}

/**
 * @brief Test an iterator
 */
void test_iterator(void) {
    TinySet<unsigned> collection;

    // Insert 5 elements in collection
    for (unsigned i = 0; i < 5; i++) {
        collection.insert(i);
    }

    // Check the iterator
    int expected = 0;
    for (auto i = collection.begin(); i != collection.end(); ++i) {
        TEST_ASSERT_EQUAL_UINT32(expected, *i);
        expected++;
    }
}

/**
 * @brief Test resizing set data[]
 */
void test_resize(void) {
    TinySet<std::string> collection(5);

    // Insert 100 elements in collection (forces resizing several times)
    for (unsigned i = 0; i < 100; i++) {
        collection.insert(std::to_string(i));
    }

    // Check the iterator
    int expected = 0;
    for (auto i = collection.begin(); i != collection.end(); ++i) {
        TEST_ASSERT_EQUAL_STRING(std::to_string(expected).c_str(), i->c_str());
        expected++;
    }
}

/**
 * @brief This is the unity tearDown method executed following each test
 */
void tearDown(void) {
}

// Unity test for native environment
int main() {
    // Initialize unity
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_insert);
    RUN_TEST(test_erase);
    RUN_TEST(test_string);
    RUN_TEST(test_iterator);
    RUN_TEST(test_resize);

    // Finished
    UNITY_END();
}  // main()
