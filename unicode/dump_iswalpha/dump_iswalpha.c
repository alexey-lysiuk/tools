
#include <locale.h>
#include <stdio.h>
#include <wctype.h>

#define CHARACTER_COUNT 65536
#define CHARACTER_PER_LINE 128
#define CHARACTER_PER_BYTE 8

int main(int argc, char** argv)
{
    const char* locale = "en_US.UTF-8";

    if (argc > 1)
    {
        locale = argv[1];
    }

    printf("// %s\nconst uint8_t ISWALPHA[] =\n{\n", setlocale(LC_ALL, locale));

    for (int i = 0; i < CHARACTER_COUNT; i += CHARACTER_PER_LINE)
    {
        printf("\t");

        for (int j = 0; j < CHARACTER_PER_LINE; j += CHARACTER_PER_BYTE)
        {
            int b = 0;

            for (int k = 0; k < CHARACTER_PER_BYTE; ++k)
            {
                if (iswalpha(i + j + k) != 0)
                {
                    b |= 1 << k;
                }
            }

            printf("0x%02X, ", b);
        }

        printf("// %04X..%04X\n", i, i + CHARACTER_PER_LINE - 1);
    }

    printf("};\n\n");
}
