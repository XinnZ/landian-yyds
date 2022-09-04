/*
 * xcmd.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <IfxAsclin_Asc.h>
#include <run.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uart.h>

#include <xcmd.h>
#include <xcmd_config.h>
#include <xcmd_default_cmds.h>
#include <xcmd_default_keys.h>
#include <xcmd_list.h>

#define CMD_IS_END_KEY(c) (((c >= 'A') && (c <= 'D')) || ((c >= 'P') && (c <= 'S')) || \
                           (c == '~') || (c == 'H') || (c == 'F'))

#define CMD_IS_PRINT(c) ((c >= 32) && (c <= 126))

typedef struct __history {
    char line[XCMD_LINE_MAX_LENGTH];
    struct __history *next;
    struct __history *prev;
} xcmd_history_t;

struct XCMDER {
    struct
    {
        uint16_t len;
        xcmd_t *head;
        xcmd_t *tail;
    } cmd_list;

    struct
    {
        uint16_t len;
        xcmd_key_t *head;
        xcmd_key_t *tail;
    } key_list;

    struct
    {
#if XCMD_HISTORY_MAX_NUM
        struct xcmd {
            xcmd_history_t pool[XCMD_HISTORY_MAX_NUM];
            uint16_t index;
        } history_pool;
        struct
        {
            uint16_t len;
            xcmd_history_t *head;
            xcmd_history_t *slider;
        } history_list;
#endif
        char display_line[XCMD_LINE_MAX_LENGTH + 1]; /* 显示区的缓存 */
        const char *prompt;                          /* 显示区的提示 */
        uint16_t byte_num;                           /* 当前行的字符个数 */
        uint16_t cursor;                             /* 光标所在位置 */
        uint16_t encode_case_stu;
        char encode_buf[7];
        uint16_t encode_count;
        uint32_t key_val;
        uint16_t param_len;
    } parser;
} g_xcmder;

static char *xcmd_strpbrk(char *s, const char *delim)  // 返回s1中第一个满足条件的字符的指针, 并且保留""号内的源格式
{
    uint16_t flag = 0;
    for (uint16_t i = 0; s[i]; i++) {
        for (uint16_t j = 0; delim[j]; j++) {
            if (0 == flag) {
                if (s[i] == '\"') {
                    for (uint16_t k = i; s[k]; k++) {
                        s[k] = s[k + 1];
                    }
                    flag = 1;
                    continue;
                }
            }

            if (flag) {
                if (s[i] == '\"') {
                    for (uint16_t k = i; s[k]; k++) {
                        s[k] = s[k + 1];
                    }
                    flag = 0;
                } else {
                    continue;
                }
            }

            if (s[i] == delim[j]) {
                return &s[i];
            }
        }
    }
    return NULL;
}

/* 字符串切割函数，使用
        char s[] = "-abc-=-def";
        char *sp;
        x = strtok_r(s, "-", &sp);      // x = "abc", sp = "=-def"
        x = strtok_r(NULL, "-=", &sp);  // x = "def", sp = NULL
        x = strtok_r(NULL, "=", &sp);   // x = NULL
                // s = "abc/0-def/0"
*/
static char *xcmd_strtok(char *s, const char *delim, char **save_ptr) {
    char *token;

    if (s == NULL)
        s = *save_ptr;

    /* Scan leading delimiters.  */
    s += strspn(s, delim);  // 返回字符串中第一个不在指定字符串中出现的字符下标，去掉了以空字符开头的
                            // 字串的情况
    if (*s == '\0')
        return NULL;

    /* Find the end of the token.  */
    token = s;
    s = xcmd_strpbrk(token, delim);  //   返回s1中第一个满足条件的字符的指针
    if (s == NULL)
        /* This token finishes the string.  */
        *save_ptr = strchr(token, '\0');
    else {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}

static int xcmd_get_param(char *msg, char *delim, char *get[], int max_num) {
    int i, ret;
    char *ptr = NULL;
    char *sp = NULL;
    ptr = xcmd_strtok(msg, delim, &sp);
    for (i = 0; ptr != NULL && i < max_num; i++) {
        get[i] = ptr;
        ptr = xcmd_strtok(NULL, delim, &sp);
    }
    ret = i;
    return ret;
}

static int xcmd_cmd_match(int argc, char *argv[]) {
    xcmd_t *p = g_xcmder.cmd_list.head;
    uint16_t flag = 0;
    int ret = -1;
    while (p) {
        if (strcmp(p->name, argv[0]) == 0) {
            flag = 1;
            if (argc > 1) {
                if ((strcmp(argv[1], "?") == 0) ||
                    (strcmp(argv[1], "-h") == 0)) {
                    xcmd_print("%s\r\n", p->help);
                    break;
                }
            }
            ret = p->func(argc, argv);
            break;
        }
        p = p->next;
    }
    if (flag) {
        xcmd_print("\r\n");
    } else {
        xcmd_print(TX_YELLOW "command \"%s\" does not found\r\n" TX_DEF, argv[0]);
    }
    return ret;
}

static void xcmd_key_match(char *key) {
    xcmd_key_t *p = g_xcmder.key_list.head;
    while (p) {
        if (strcmp(key, p->key) == 0) {
            p->func(&g_xcmder);
            break;
        }
        p = p->next;
    }
}

static void xcmd_key_exec(char *key) {
    xcmd_key_match(key);
}

static uint16_t xcmd_rcv_encode(char byte) {
    uint16_t ret = 0;

    switch (g_xcmder.parser.encode_case_stu) {
        case 0:
            g_xcmder.parser.encode_count = 0;

            if (byte == 0x1B)  // ESC
            {
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
                g_xcmder.parser.encode_case_stu = 1;
                g_xcmder.parser.key_val = byte;
            } else {
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = '\0';
                g_xcmder.parser.encode_count = 0;
                ret = 1;
            }
            break;
        case 1:
            if (CMD_IS_END_KEY(byte)) {
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count] = '\0';
                ret = g_xcmder.parser.encode_count;
                g_xcmder.parser.encode_case_stu = 0;
            } else {
                g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
                if (g_xcmder.parser.encode_count >= 6) {
                    g_xcmder.parser.encode_case_stu = 0;
                    ret = 0;
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

char *xcmd_display_line_end(void) {
    char *ret = g_xcmder.parser.display_line;
    if (g_xcmder.parser.byte_num) {
#if XCMD_HISTORY_MAX_NUM
        if (g_xcmder.parser.history_list.head == NULL) {
            xcmd_history_insert(ret);
        } else {
            char *head_line = g_xcmder.parser.history_list.head->line;
            if (strcmp(head_line, ret) != 0) {
                xcmd_history_insert(ret);
            }
        }
#endif
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
        xcmd_history_slider_reset();
    }
    return ret;
}

static void xcmd_parser(char byte) {
    uint16_t num = 0;

    num = xcmd_rcv_encode(byte);

    if (num > 0) {
        if (CMD_IS_PRINT(g_xcmder.parser.encode_buf[0])) {
            xcmd_display_insert_char(g_xcmder.parser.encode_buf[0]);
            return;
        } else {
            xcmd_key_exec(g_xcmder.parser.encode_buf);
        }
    }
}

void xcmd_print(const char *fmt, ...) {
    /* Disable log printing when uploading data */
    if (Run_Params.Mode_Upload)
        return;

    va_list arg;
    short length;
    char ucstring[XCMD_PRINT_BUF_MAX_LENGTH] = {0};

    va_start(arg, fmt);
    length = (short)vsnprintf(ucstring, XCMD_PRINT_BUF_MAX_LENGTH - 1, fmt, arg);
    va_end(arg);

    /*---------------------------------------- Transmit Function ---------------------------------------------------*/

    UART_Console.Transmit((const char *)ucstring, &length);

    /*---------------------------------------------- END -----------------------------------------------------------*/

    return;
}

void xcmd_display_write(const char *buf, uint16_t len) {
    xcmd_display_clear();
    if (len > XCMD_LINE_MAX_LENGTH) {
        len = XCMD_LINE_MAX_LENGTH;
    }
    memcpy(g_xcmder.parser.display_line, buf, len);
    g_xcmder.parser.display_line[len] = '\0';
    xcmd_print(g_xcmder.parser.display_line);
    g_xcmder.parser.byte_num = len;
    g_xcmder.parser.cursor = len;
}

void xcmd_display_print(const char *msg) {
    xcmd_display_write(msg, strlen(msg));
}

char *xcmd_display_get(void) {
    char *line = g_xcmder.parser.display_line;
    return line;
}

void xcmd_display_clear(void) {
    char *line = xcmd_display_get();
    xcmd_print(DL(1));
    g_xcmder.parser.cursor = 0;
    xcmd_print(CHA(g_xcmder.parser.cursor + 1));

#ifndef XCMD_DEFAULT_PROMPT_CLOLR
    xcmd_print(TX_DEF "%s", xcmd_get_prompt());
#else
    xcmd_print(XCMD_DEFAULT_PROMPT_CLOLR "%s" TX_DEF, xcmd_get_prompt());
#endif
    g_xcmder.parser.byte_num = 0;
    g_xcmder.parser.cursor = 0;
    line[0] = '\0';
}

void xcmd_display_insert_char(char c) {
    char *line = xcmd_display_get();
    if (g_xcmder.parser.byte_num < XCMD_LINE_MAX_LENGTH - 1) {
        for (uint16_t i = g_xcmder.parser.byte_num; i > g_xcmder.parser.cursor; i--) {
            line[i] = line[i - 1];
        }
        g_xcmder.parser.byte_num++;
        line[g_xcmder.parser.byte_num] = '\0';
        line[g_xcmder.parser.cursor++] = c;
        xcmd_print(ICH(1));
        xcmd_print("%c", c);
    }
}

void xcmd_display_delete_char(void) {
    char *line = xcmd_display_get();
    if (g_xcmder.parser.cursor > 0) {
        for (uint16_t i = g_xcmder.parser.cursor - 1; i < g_xcmder.parser.byte_num - 1; i++) {
            line[i] = line[i + 1];
        }
        g_xcmder.parser.byte_num--;
        g_xcmder.parser.cursor--;
        line[g_xcmder.parser.byte_num] = ' ';
        line[g_xcmder.parser.byte_num] = '\0';
        xcmd_print(CUB(1));
        xcmd_print(DCH(1));
    }
}

uint16_t xcmd_display_current_char(char *cha) {
    if (g_xcmder.parser.cursor < g_xcmder.parser.byte_num) {
        char *line = xcmd_display_get();
        *cha = line[g_xcmder.parser.cursor];
        return 1;
    }
    return 0;
}

void xcmd_display_cursor_set(uint16_t pos) {
    if (pos > g_xcmder.parser.byte_num) {
        pos = g_xcmder.parser.byte_num;
    }
    g_xcmder.parser.cursor = pos;
    xcmd_print(CHA(g_xcmder.parser.cursor + XCMD_DEFAULT_PROMPT_LEN + 1));
}

uint16_t xcmd_display_cursor_get(void) {
    return g_xcmder.parser.cursor;
}

void xcmd_history_insert(char *str) {
#if XCMD_HISTORY_MAX_NUM
    if (g_xcmder.parser.history_list.len < XCMD_HISTORY_MAX_NUM) {
        xcmd_history_t *new_p = &(g_xcmder.parser.history_pool.pool[g_xcmder.parser.history_pool.index++]);
        if (g_xcmder.parser.history_list.len == 0) /* 头插 */
        {
            strncpy(new_p->line, str, XCMD_LINE_MAX_LENGTH);
            g_xcmder.parser.history_list.head = new_p;
            g_xcmder.parser.history_list.head->next = new_p;
            g_xcmder.parser.history_list.head->prev = new_p;
            g_xcmder.parser.history_list.slider = new_p;
            g_xcmder.parser.history_list.len++;
        } else {
            strncpy(new_p->line, str, XCMD_LINE_MAX_LENGTH);
            xcmd_history_t *old_head = g_xcmder.parser.history_list.head;
            g_xcmder.parser.history_list.head = new_p;
            new_p->next = old_head;
            new_p->prev = old_head->prev;
            old_head->prev->next = new_p;
            old_head->prev = new_p;
            g_xcmder.parser.history_list.len++;
        }
    } else {
        g_xcmder.parser.history_list.head = g_xcmder.parser.history_list.head->prev;
        strncpy(g_xcmder.parser.history_list.head->line, str, XCMD_LINE_MAX_LENGTH);
    }
#endif
}

char *xcmd_history_next(void) {
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if (g_xcmder.parser.history_list.len) {
        line = g_xcmder.parser.history_list.slider->line;
        if (g_xcmder.parser.history_list.slider->next != g_xcmder.parser.history_list.head) {
            g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.slider->next;
        }
    }
#endif
    return line;
}

char *xcmd_history_prev(void) {
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if (g_xcmder.parser.history_list.len) {
        if (g_xcmder.parser.history_list.slider != g_xcmder.parser.history_list.head) {
            g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.slider->prev;
            line = g_xcmder.parser.history_list.slider->line;
        }
    }
#endif
    return line;
}

char *xcmd_history_current(void) {
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if (g_xcmder.parser.history_list.len) {
        if (g_xcmder.parser.history_list.slider) {
            line = g_xcmder.parser.history_list.slider->line;
        }
    }
#endif
    return line;
}

uint16_t xcmd_history_len(void) {
#if XCMD_HISTORY_MAX_NUM
    return g_xcmder.parser.history_list.len;
#else
    return 0;
#endif
}

void xcmd_history_slider_reset(void) {
#if XCMD_HISTORY_MAX_NUM
    g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.head;
#endif
}

int xcmd_exec(char *str) {
    int param_num = 0;
    char *cmd_param_buff[XCMD_PARAM_MAX_NUM];
    char temp[XCMD_LINE_MAX_LENGTH];
    strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
    param_num = xcmd_get_param(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
    if (param_num > 0) {
        return xcmd_cmd_match(param_num, cmd_param_buff);
    }
    return -1;
}

int xcmd_key_register(xcmd_key_t *keys, uint16_t number) {
    uint8_t i = 0;
    if (g_xcmder.key_list.len == 0) {
        g_xcmder.key_list.head = &keys[i++];
        g_xcmder.key_list.head->next = NULL;
        g_xcmder.key_list.tail = g_xcmder.key_list.head;
        ++g_xcmder.key_list.len;
    }

    while (i < number) {
        g_xcmder.key_list.tail->next = &keys[i];
        g_xcmder.key_list.tail = g_xcmder.key_list.tail->next;
        keys[i].next = NULL;
        ++g_xcmder.key_list.len;
        ++i;
    }
    return g_xcmder.key_list.len;
}

int xcmd_cmd_register(xcmd_t *cmds, uint16_t number) {
    uint8_t i = 0;
    if (g_xcmder.cmd_list.len == 0) {
        g_xcmder.cmd_list.head = &cmds[i++];
        g_xcmder.cmd_list.head->next = NULL;
        g_xcmder.cmd_list.tail = g_xcmder.cmd_list.head;
        ++g_xcmder.cmd_list.len;
    }

    while (i < number) {
        g_xcmder.cmd_list.tail->next = &cmds[i];
        g_xcmder.cmd_list.tail = g_xcmder.cmd_list.tail->next;
        cmds[i].next = NULL;
        ++g_xcmder.cmd_list.len;
        ++i;
    }
    return g_xcmder.cmd_list.len;
}

xcmd_key_t *xcmd_keylist_get(void) {
    return g_xcmder.key_list.head;
}

xcmd_t *xcmd_cmdlist_get(void) {
    return g_xcmder.cmd_list.head;
}

int xcmd_unregister_cmd(char *cmd) {
    xcmd_t *p = g_xcmder.cmd_list.head;
    xcmd_t *bk = p;
    while (p) {
        if (strcmp(cmd, p->name) == 0) {
            if (g_xcmder.cmd_list.len == 1) {
                g_xcmder.cmd_list.head = g_xcmder.cmd_list.tail = NULL;
            } else {
                bk->next = p->next;
                if (p->next == NULL) {
                    g_xcmder.cmd_list.tail = bk;
                }
            }
            g_xcmder.cmd_list.len--;
            return 0;
        }
        bk = p;
        p = p->next;
    }
    return -1;
}

int xcmd_unregister_key(char *key) {
    xcmd_key_t *p = g_xcmder.key_list.head;
    xcmd_key_t *bk = p;
    while (p) {
        if (strcmp(key, p->key) == 0) {
            if (g_xcmder.key_list.len == 1) {
                g_xcmder.key_list.head = g_xcmder.key_list.tail = NULL;
            } else {
                bk->next = p->next;
                if (p->next == NULL) {
                    g_xcmder.key_list.tail = bk;
                }
            }
            g_xcmder.key_list.len--;
            return 0;
        }
        bk = p;
        p = p->next;
    }
    return -1;
}

void xcmd_set_prompt(const char *prompt) {
    if (prompt) {
        g_xcmder.parser.prompt = prompt;
    }
}

const char *xcmd_get_prompt(void) {
    return g_xcmder.parser.prompt;
}

static void xcmd_init(void) {
    memset(&g_xcmder, 0, sizeof(struct XCMDER));
    g_xcmder.parser.prompt = XCMD_DEFAULT_PROMPT;

    default_keys_init();
    default_cmds_init();
    extern_cmds_init();

    xcmd_exec("clear");
    xcmd_exec("logo");
    // xcmd_exec("rst");

    xcmd_display_clear();

#if XCMD_HISTORY_MAX_NUM
    g_xcmder.parser.history_list.len = 0;
    g_xcmder.parser.history_list.head = NULL;
    g_xcmder.parser.history_list.slider = NULL;
    g_xcmder.parser.history_pool.index = 0;
#endif
}

static void xcmd_rece(void) {
    char i = 0;
    short count = 0;
    short readCount = XCMD_LINE_MAX_LENGTH;
    char receiveDatas[XCMD_LINE_MAX_LENGTH];

    UART_Console.Receive(receiveDatas, &readCount);
    count += readCount;

    for (i = 0; i < count; i++) {
        xcmd_parser(receiveDatas[i]);
    }
}

XCMD_TypeDef Xcmd =
    {
        .Init = xcmd_init,
        .Rece = xcmd_rece};
