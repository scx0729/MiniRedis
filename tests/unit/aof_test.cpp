#include "test_framework.h"
#include "miniredis/persistence/aof.h"
#include "miniredis/persistence/aof_replay.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/command/registry.h"
#include "miniredis/base/clock.h"
#include <cstdio>
#include <fstream>

using namespace miniredis;

static const char* TEST_AOF_FILE = "/tmp/miniredis_test_aof.aof";

// Helper: write AOF content using ofstream (truncates)
static void WriteAofFile(const char* path, const std::string& content) {
    std::ofstream f(path, std::ios::trunc | std::ios::binary);
    f.write(content.data(), static_cast<std::streamsize>(content.size()));
}

TEST(aof_write_and_read) {
    std::remove(TEST_AOF_FILE);

    {
        AofWriter writer(TEST_AOF_FILE);
        auto r = writer.Write("*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
        ASSERT_TRUE(r.ok());
    }

    std::ifstream f(TEST_AOF_FILE);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    ASSERT_EQ(content, "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    std::remove(TEST_AOF_FILE);
}

TEST(aof_replay) {
    std::remove(TEST_AOF_FILE);

    WriteAofFile(TEST_AOF_FILE,
        "*3\r\n$3\r\nSET\r\n$1\r\na\r\n$1\r\n1\r\n"
        "*3\r\n$3\r\nSET\r\n$1\r\nb\r\n$1\r\n2\r\n");

    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay(TEST_AOF_FILE, db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 2);

    ASSERT_TRUE(db.Exists("a"));
    ASSERT_EQ(db.Get("a").value().value(), "1");
    ASSERT_TRUE(db.Exists("b"));
    ASSERT_EQ(db.Get("b").value().value(), "2");

    std::remove(TEST_AOF_FILE);
}

TEST(aof_replay_empty_file) {
    std::remove(TEST_AOF_FILE);
    WriteAofFile(TEST_AOF_FILE, "");

    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay(TEST_AOF_FILE, db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 0);

    std::remove(TEST_AOF_FILE);
}

TEST(aof_replay_nonexistent_file) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay("/tmp/nonexistent_aof.aof", db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 0);
}

TEST(aof_replay_truncated_tail) {
    std::remove(TEST_AOF_FILE);

    // Write a complete command followed by a truncated one
    WriteAofFile(TEST_AOF_FILE,
        "*3\r\n$3\r\nSET\r\n$1\r\na\r\n$1\r\n1\r\n"
        "*3\r\n$3\r\nSET\r\n$1\r\nb\r\n");  // missing value part

    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay(TEST_AOF_FILE, db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 1);  // only the first command replays

    ASSERT_TRUE(db.Exists("a"));
    ASSERT_FALSE(db.Exists("b"));

    std::remove(TEST_AOF_FILE);
}

TEST_MAIN()
