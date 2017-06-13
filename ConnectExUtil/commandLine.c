#include <string.h>
#include "CEUtil.h"

bool CommandLineExists(const int argc, char **argv, const char parameter)
{
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if ( strlen(argv[i]) > 1 &&
                argv[i][0] == '-' &&
                toupper(argv[i][1]) == toupper(parameter)) {
            return true;
        }
    }
    return false;
}
