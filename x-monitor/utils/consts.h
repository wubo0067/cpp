/*
 * @Author: CALM.WU
 * @Date: 2021-10-22 11:57:56
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-01 17:44:56
 */

#include <stdio.h>

#define PID_FILENAME_MAX 32
#define NUMBER_BUFFER_SIZE 32
#define EXTERNAL_PLUGIN_CMD_LINE_MAX (FILENAME_MAX * 2)
#define CONFIG_NAME_MAX 1024
#define STDOUT_LINE_BUF_SIZE 1024

extern char *DEFAULT_PLUGINS_DIR;
extern char *DEFAULT_PIDFILE;