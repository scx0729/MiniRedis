#include "miniredis/config/config.h"
#include "miniredis/server/server.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static void PrintUsage(const char* prog) {
    fprintf(stderr, "Usage: %s [--config <config-file>]\n", prog);
    fprintf(stderr, "  --config, -c  Path to configuration file\n");
    fprintf(stderr, "  --help, -h    Show this help\n");
}

int main(int argc, char* argv[]) {
    std::string config_file = "config/miniredis.conf";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--config") == 0 || strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --config requires an argument\n");
                return 1;
            }
            config_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            PrintUsage(argv[0]);
            return 1;
        }
    }

    // Load configuration
    auto config_result = miniredis::Config::Load(config_file);
    if (!config_result.ok()) {
        fprintf(stderr, "Error loading config: %s\n", config_result.error_message().c_str());
        return 1;
    }

    auto config = config_result.value();

    // Create and start server
    miniredis::Server server(config);
    if (!server.Start()) {
        fprintf(stderr, "Error: failed to start server\n");
        return 1;
    }

    fprintf(stderr, "MiniRedis17 server started on %s:%d\n",
            config.bind_address.c_str(), config.port);

    // Run event loop (blocks until shutdown signal)
    server.Run();

    fprintf(stderr, "MiniRedis17 server shutting down\n");
    return 0;
}
