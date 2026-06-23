#include "test_framework.h"
#include "miniredis/config/config.h"
#include <fstream>

using namespace miniredis;

TEST(config_defaults) {
    Config config;
    ASSERT_EQ(config.bind_address, "127.0.0.1");
    ASSERT_EQ(config.port, 6380);
    ASSERT_TRUE(config.appendonly);
    ASSERT_EQ(config.appendfilename, "miniredis.aof");
    ASSERT_TRUE(config.fsync_always);
}

TEST(config_parse_port) {
    Config config;
    auto r = config.ParseLine("port 6379");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(config.port, 6379);
}

TEST(config_parse_bind) {
    Config config;
    auto r = config.ParseLine("bind 0.0.0.0");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(config.bind_address, "0.0.0.0");
}

TEST(config_parse_appendonly) {
    Config config;
    config.ParseLine("appendonly no");
    ASSERT_FALSE(config.appendonly);
    config.ParseLine("appendonly yes");
    ASSERT_TRUE(config.appendonly);
}

TEST(config_parse_appendfsync) {
    Config config;
    config.ParseLine("appendfsync no");
    ASSERT_FALSE(config.fsync_always);
    config.ParseLine("appendfsync always");
    ASSERT_TRUE(config.fsync_always);
}

TEST(config_parse_invalid_port) {
    Config config;
    auto r = config.ParseLine("port abc");
    ASSERT_FALSE(r.ok());
}

TEST(config_parse_out_of_range_port) {
    Config config;
    auto r = config.ParseLine("port 99999");
    ASSERT_FALSE(r.ok());
}

TEST(config_parse_buffer_size) {
    Config config;
    auto r = config.ParseLine("max-client-input-buffer 1048576");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(config.max_client_input_buffer, 1048576u);
}

TEST(config_load_from_file) {
    const char* path = "/tmp/miniredis_test.conf";
    {
        std::ofstream f(path);
        f << "# Test config\n";
        f << "bind 192.168.1.1\n";
        f << "port 6390\n";
        f << "appendonly no\n";
    }

    auto result = Config::Load(path);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().bind_address, "192.168.1.1");
    ASSERT_EQ(result.value().port, 6390);
    ASSERT_FALSE(result.value().appendonly);

    std::remove(path);
}

TEST(config_load_nonexistent) {
    auto result = Config::Load("/tmp/nonexistent_config.conf");
    ASSERT_TRUE(result.ok());  // returns defaults
    ASSERT_EQ(result.value().port, 6380);
}

TEST(config_parse_unknown_key) {
    Config config;
    auto r = config.ParseLine("unknown-key value");
    ASSERT_TRUE(r.ok());  // unknown keys are ignored
}

TEST_MAIN()
