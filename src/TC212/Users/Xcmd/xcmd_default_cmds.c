/*
 * xcmd_default_cmds.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <xcmd_default_cmds.h>
#include <xcmd_config.h>
#include <xcmd.h>

#include <../HeadFiles.h>

static int cmd_info(int argc, char *argv[]) {
    xcmd_print("Actual PLL output frequency: %f\r\n", IfxScuCcu_getPllFrequency());
    xcmd_print("Cpu frequency by CCUCON Reg: %f\r\n", IfxScuCcu_getCpuFrequency(IfxCpu_getCoreIndex()));
    xcmd_print("      SPB divider frequency: %f\r\n", IfxScuCcu_getSpbFrequency());
    xcmd_print("     System timer frequency: %f\r\n", IfxStm_getFrequency(&MODULE_STM0));

    return 0;
}

static int cmd_rst(int argc, char *argv[]) {
    scuRcuResetCode lastReset;
    lastReset = evaluateReset();

    switch (lastReset.resetType) {
        case IfxScuRcu_ResetType_application:
            xcmd_print("> Last rst: %s\r\n", "ResetType_application");
            break;

        case IfxScuRcu_ResetType_system:
            xcmd_print("> Last rst: %s\r\n", "ResetType_system");
            break;

        case IfxScuRcu_ResetType_warmpoweron:
            xcmd_print("> Last rst: %s\r\n", "ResetType_warmpoweron");
            break;

        default:
            xcmd_print("> Last rst: %s\r\n", "Unknown!");
            break;
    }

    clearColdPowerOnResetBits();
    return 0;
}

static int cmd_clear(int argc, char *argv[]) {
    xcmd_print("\033c");
    return 0;
}

static int cmd_help(int argc, char *argv[]) {
    xcmd_t *p = xcmd_cmdlist_get();
    while (p) {
        xcmd_print("> %-8s: %s\r\n", p->name, p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_keys(int argc, char *argv[]) {
    xcmd_key_t *p = xcmd_keylist_get();
    while (p) {
        xcmd_print("0x%08x\t", p->key);
        xcmd_print("%s\r\n", p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_logo(int argc, char *argv[]) {
    /*
    char *log = "__  ___             _____\r\n\
\\ \\/ (_)____  ____ |__  /\r\n\
 \\  /| |  _ \\|  _ \\  / /\r\n\
 /  \\| | | | | | | |/ /_\r\n\
/_/\\_\\_|_| |_|_| |_/____|";
    xcmd_print("%s\r\n\r\n", log);

    xcmd_print("> Author    XinnZ\r\n");
    xcmd_print("> Website   https://www.xinnz.cn/\r\n");
    xcmd_print("> Datetime  %s %s\r\n" , __DATE__,  __TIME__);
     */

    char *log =
        " __   ___   ______  ____\r\n\
 \\ \\ / \\ \\ / /  _ \\/ ___|\r\n\
  \\ V / \\ V /| | | \\___ \\\r\n\
   | |   | | | |_| |___) |\r\n\
   |_|   |_| |____/|____/ ";
    xcmd_print("%s\r\n\r\n", log);
    xcmd_print("> Teams: Landian YYDS Car\r\n");
    xcmd_print("> Topic: Baidu Complete Model Group\r\n");
    xcmd_print("> Build: %s. %s\r\n", __DATE__, __TIME__);

    return 0;
}

static xcmd_t cmds[] =
    {
        {"logo", 	cmd_logo, 		"show logo", 				NULL},
        {"help", 	cmd_help, 		"show this list", 			NULL},
        {"keys", 	cmd_keys, 		"show keys", 				NULL},
        {"clear", 	cmd_clear, 		"clear screen", 			NULL},
        {"rst", 	cmd_rst, 		"get the reset reason.", 	NULL},
        {"info", 	cmd_info, 		"hardware freq info.", 		NULL},
};

void default_cmds_init(void) {
    xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
