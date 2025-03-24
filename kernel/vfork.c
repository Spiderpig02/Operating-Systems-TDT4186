#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int pid;

    pid = vfork();
    if (pid < 0)
    {
        printf(2, "vfork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        exec(argv[1], &argv[1]);
        printf(2, "vfork: exec failed\n");
        exit(1);
    }

    wait();
    exit(0);
}