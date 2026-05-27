#include <gtest/gtest.h>

import sync_map;

#include <string>

TEST(SyncMap, SetsAndGetsValues) {
  SyncMap<std::string, int> map;

  map.put("one", 1);
  map.put("two", 2);

  ASSERT_TRUE(map.get("one").has_value());
  EXPECT_EQ(*map.get("one"), 1);
  EXPECT_EQ(*map.get("two"), 2);
  EXPECT_FALSE(map.get("missing").has_value());
}

TEST(SyncMap, OverwritesExistingValue) {
  SyncMap<int, std::string> map;

  map.put(7, "old");
  map.put(7, "new");

  EXPECT_EQ(map.size(), 1U);
  EXPECT_EQ(*map.get(7), "new");
}

TEST(SyncMap, RemovesAndClearsValues) {
  SyncMap<int, int> map;
  map.put(1, 10);
  map.put(2, 20);

  EXPECT_TRUE(map.remove(1));
  EXPECT_FALSE(map.contains(1));
  EXPECT_FALSE(map.remove(1));

  map.clear();

  EXPECT_TRUE(map.empty());
}

TEST(SyncMap, ReturnsFallbackForMissingValue) {
  SyncMap<int, int> map;
  map.put(1, 10);

  EXPECT_EQ(map.valueOr(1, 99), 10);
  EXPECT_EQ(map.valueOr(2, 99), 99);
}

TEST(SyncMap, SnapshotIsIndependentCopy) {
  SyncMap<int, std::string> map;
  map.put(2, "two");
  map.put(1, "one");

  auto snapshot = map.snapshot();
  map.put(1, "changed");

  EXPECT_EQ(snapshot.size(), 2U);
  EXPECT_EQ(snapshot[1], "one");
  EXPECT_EQ(snapshot[2], "two");
}
