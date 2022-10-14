#include <string>

typedef void log_func(const std::string &msg);

struct Platform
{
    log_func *log = nullptr;
};

typedef void update_func(Platform &platform);
