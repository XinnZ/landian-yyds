/*
 * xcmd_list.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef XCMD_LIST_H_
#define XCMD_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xcmd.h"
#include "xcmd_config.h"

#define xcmd_param_check(need, argc) (need < argc)

void extern_cmds_init(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_LIST_H_ */
