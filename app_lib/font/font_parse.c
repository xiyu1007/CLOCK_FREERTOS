#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define uint8_t unsigned char
#define NUM_DEFAULTS (sizeof(defaults) / sizeof(defaults[0]))
#define MAX_LINES 1024
#define MAX_LINE_LEN 1024
static int parse_hex_byte(const char *p, uint8_t *out);
static int hex_char_to_int(char c);

typedef struct
{
    const char *name;
    const uint8_t *model;
    int size;
    int width;
} font_chinese_t;

typedef struct
{
    const char *field_name;
    const char *default_value;
} field_default_t;

field_default_t defaults[] = {
    // {"size", "16"},
    // {"width", "16"},
    // {"type", "\"normal\""},
};

char g_font_parse_name[] = "font_parse.txt";


// 格式
// 0x000x200x000x200x7B0xFE0x480x200x490xFC0x480x200x4B0xFE0x780x00,
// 0x490xFC0x490x040x490xFC0x490x040x790xFC0x490x040x010x140x010x08,//"晴",0
// 0x000x000x7D0xFC0x450x040x490x040x490x040x510xFC0x490x040x490x04,
// 0x450x040x450xFC0x450x040x690x040x520x040x420x040x440x140x480x08,//"阴",1


int is_number(const char *str)
{
    if (!str || *str == '\0')
        return 0;
    if (*str == '-' || *str == '+')
        str++;
    while (*str)
    {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

int main()
{
    char *data[MAX_LINES];
    int data_len = 0;

    printf("请输入字模数据，每行回车结束，空行结束输入:\n");
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), stdin))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            break;
        data[data_len] = strdup(line);
        data_len++;
        if (data_len >= MAX_LINES)
            break;
    }

    FILE *fp = fopen(g_font_parse_name, "w");
    if (!fp)
    {
        perror("无法创建文件");
        return 1;
    }

    fprintf(fp, "static const font_chinese_t g_font_parse[] = {\n");

    char *current_font[MAX_LINES];
    int current_len = 0;
    int font_count = 0;

    for (int i = 0; i < data_len; i++)
    {
        current_font[current_len++] = data[i];

        char *comment = strstr(data[i], "//");
        if (comment)
        {
            char name[32] = {0};
            int index = 0;
            sscanf(comment, "//\"%[^\"]\",%d", name, &index);

            fprintf(fp, "    {\n");
            fprintf(fp, "        .name = \"%s\",\n", name);

            for (int k = 0; k < NUM_DEFAULTS; k++)
            {
                if (is_number(defaults[k].default_value))
                {
                    fprintf(fp, "        .%s = %s,\n", defaults[k].field_name, defaults[k].default_value);
                }
                else
                {
                    fprintf(fp, "        .%s = %s,\n", defaults[k].field_name, defaults[k].default_value);
                }
            }

            fprintf(fp, "        .model = (const uint8_t[]) {\n");

            // 输出每个0x数值并用逗号隔开
            for (int j = 0; j < current_len; j++)
            {
                char *comment_pos = strstr(current_font[j], "//");
                if (comment_pos)
                    *comment_pos = 0;

                const char *p = current_font[j];
                while (*p)
                {
                    uint8_t value;
                    if (parse_hex_byte(p, &value))
                    {
                        fprintf(fp, "0x%02X,", value);
                        p += 4;  // 精确跳过 "0xHH"
                    }
                    else
                    {
                        p++;     // 非 0xHH，继续扫描
                    }
                }
                fprintf(fp, "\n");
            }

            fprintf(fp, "            /*\"%s\",%d*/\n", name, index);
            fprintf(fp, "        },\n");
            fprintf(fp, "    },\n");

            current_len = 0;
            font_count++;
        }
    }

    fprintf(fp, "};\n");
    fprintf(fp, "// Total fonts: %d\n", font_count);

    fclose(fp);

    for (int i = 0; i < data_len; i++)
        free(data[i]);

    printf("解析完成，输出文件: %s\n", g_font_parse_name);
    return 0;
}


static int hex_char_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_hex_byte(const char *p, uint8_t *out)
{
    // 必须是 "0xHH"
    if (p[0] != '0' || p[1] != 'x')
        return 0;

    int hi = hex_char_to_int(p[2]);
    int lo = hex_char_to_int(p[3]);

    if (hi < 0 || lo < 0)
        return 0;

    *out = (uint8_t)((hi << 4) | lo);
    return 1;
}