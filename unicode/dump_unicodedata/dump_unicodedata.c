
#include <stdio.h>
#include <stdlib.h>

#define CHARACTER_COUNT 65536
#define CHARACTER_PER_LINE 128
#define CHARACTER_PER_BYTE 8

char calpha[CHARACTER_COUNT];
char buffer[1024];

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        puts("Usage: %s UnicodeData.txt");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");

    if (!f)
    {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }

    while (fgets(buffer, sizeof buffer, f))
    {
        char* curr = buffer;
        int code = strtol(buffer, &curr, 16);

        if (code >= CHARACTER_COUNT)
        {
            break;
        }

        for (int i = 0; i < 2; ++i)
        {
            while (*curr++ != ';');
        }

        calpha[code] = *curr == 'L' ? 1 : 0;
    }

    fclose(f);


    puts("// UnicodeData.txt\nconst uint8_t ISWALPHA[] =\n{");

    for (int i = 0; i < CHARACTER_COUNT; i += CHARACTER_PER_LINE)
    {
        printf("\t");

        for (int j = 0; j < CHARACTER_PER_LINE; j += CHARACTER_PER_BYTE)
        {
            int b = 0;

            for (int k = 0; k < CHARACTER_PER_BYTE; ++k)
            {
                if (calpha[i + j + k] != 0)
                {
                    b |= 1 << k;
                }
            }

            printf("0x%02X, ", b);
        }

        printf("// %04X..%04X\n", i, i + CHARACTER_PER_LINE - 1);
    }

    puts("};\n");
}
