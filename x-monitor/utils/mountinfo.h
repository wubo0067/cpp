/*
 * @Author: CALM.WU 
 * @Date: 2021-12-20 16:55:51 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-20 17:07:05
 */

#pragma once

#include <stdint.h>

struct mountinfo {
    int32_t
        id; // mount ID: unique identifier of the mount (may be reused after umount(2)).
    int32_t
             parent_id; // parent ID: ID of parent mount (or of self for the top of the mount tree).
    uint32_t major; // major: major device number of device.
    uint32_t minor; // minor: minor device number of device.

    char *   persistent_id; // a calculated persistent id for the mount point
    uint32_t persistent_id_hash;

    char *   root; // root: root of the mount within the filesystem.
    uint32_t root_hash;

    char *mount_options; // mount options: per-mount options.

    int32_t optional_fields_count;

    char *filesystem; // filesystem type: name of filesystem in the form "type[.subtype]".
    uint32_t filesystem_hash;

    char *mount_source; // mount source: filesystem-specific information or "none".
    uint32_t mount_source_hash;

    char *   mount_source_name;
    uint32_t mount_source_name_hash;

    char *super_options; // super options: per-superblock options.

    uint32_t          flags;
    dev_t             st_dev; // id of device as given by stat()
    struct mountinfo *next;
};

extern struct mountinfo *mountinfo_read(void);
extern void mountinfo_free(struct mountinfo *mi);