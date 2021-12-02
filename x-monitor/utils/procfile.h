/*
 * @Author: CALM.WU 
 * @Date: 2021-12-01 16:58:42 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-01 17:51:04
 */

#pragma once

#include <stdint.h>
#include <stdio.h>

struct pf_words {
    size_t len;     // used entries
    size_t size;    // capacity
    char * words[]; // array of pointers
};

struct pf_line {
    size_t words; // how many words this line has
    size_t first; // the id of the first word of this line
                  // in the words array
};

struct pf_lines {
    size_t         len;     // used entries
    size_t         size;    // capacity
    struct pf_line lines[]; // array of lines
};

enum procfile_separator {
    PF_CHAR_IS_SEPARATOR, // NUL '\0' (null character)
    PF_CHAR_IS_NEWLINE,   // SOH (start of heading)
    PF_CHAR_IS_WORD,      // STX (start of text)
    PF_CHAR_IS_QUOTE,     // ETX (end of text)
    PF_CHAR_IS_OPEN,      // EOT (end of transmission)
    PF_CHAR_IS_CLOSE      // ENQ (enquiry)
};

struct proc_file {
    char     filename[FILENAME_MAX + 1]; //
    uint32_t flags;
    int      fd;   // the file descriptor
    size_t   len;  // the bytes we have placed into data
    size_t   size; // the bytes we have allocated for data

    struct pf_lines *lines;
    struct pf_words *words;

    enum procfile_separator separators[256];

    char data[]; // allocated buffer to keep file contents
};

// close the proc file and free all related memory
extern void procfile_close(struct proc_file *ff);

// (re)read and parse the proc file
extern struct proc_file *procfile_readall(struct proc_file *ff);

// open a /proc or /sys file
extern struct proc_file *procfile_open(const char *filename,
                                       const char *separators, uint32_t flags);

//
extern char *procfile_filename(struct proc_file *ff);