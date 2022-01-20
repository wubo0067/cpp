/*
 * @Author: CALM.WU
 * @Date: 2021-10-28 16:45:19
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-18 14:32:19
 */

#include "consts.h"

char *DEFAULT_PLUGINS_DIR = "/usr/libexec/netdata/plugins.d";
char *DEFAULT_PIDFILE     = "/tmp/x-monitor.pid";

char premetheus_instance_label[PROM_METRIC_LABEL_VALUE_LEN] = { 0 };