#include "build.h"
#define PROJECT "%s"
#define CC "gcc"
#define CFLAGS "-Wall -Wextra -Werror -O3"

int main(void) {
    ConstructRules rules = {0};

    ConstructRule rule = {SA("build/"PROJECT), SA("src/main.c"), 0};
    command_append(&rule.command, CC, "src/main.c", "-o", "build/"PROJECT, CFLAGS);
    da_append(&rules, rule);

    if (!run_construct(&rules, true)) {print_log(ERROR, "Build failed"); return 1;}
    print_log(INFO, "Build finished");

    return 0;
}