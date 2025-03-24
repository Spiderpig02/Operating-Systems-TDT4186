#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void uint64_to_hex(uint64 value, char *hex_string)
{
    int i;
    for (i = 7; i >= 0; i--)
    {
        uint64 digit = value & 0xF;
        if (digit < 10)
        {
            hex_string[i] = '0' + digit;
        }
        else
        {
            hex_string[i] = 'A' + (digit - 10);
        }
        value >>= 4;
    }
    hex_string[8] = '\0';
}

int main(int argc, char *argv[])
{
    char hex_string[9];

    if (argc == 1)
    {
        printf("Usage: vatopa virtual_address [pid]\n");
    }
    else if (argc == 2)
    {
        uint64 vm = (uint64)atoi(argv[1]);
        uint64 result = va2pa(vm, (int)getpid());

        uint64_to_hex(result, hex_string);
        printf("0x%s\n", hex_string);
    }
    else if (argc == 3)
    {
        uint64 vm = (uint64)atoi(argv[1]);
        uint64 result = va2pa(vm, atoi(argv[2]));
        
        uint64_to_hex(result, hex_string);
        printf("0x%s\n", hex_string);
    };

    exit(0);
}