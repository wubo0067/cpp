/*
 * @Author: CALM.WU 
 * @Date: 2021-12-02 10:34:06 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-02 10:34:41
 */

#include "procfile.h"
#include "common.h"
#include "compiler.h"
#include "log.h"

#define PROCFILE_DATA_BUFFER_SIZE 512
#define PFLINES_INCREASE_STEP 10
#define PFWORDS_INCREASE_STEP 200

//-------------------------------------------------------------------------------------------------
// file name
char *procfile_filename(struct proc_file *ff)
{
    if (ff->filename[0])
        return ff->filename;

    char buffer[FILENAME_MAX + 1];
    snprintf(buffer, FILENAME_MAX, "/proc/self/fd/%d", ff->fd);

    ssize_t l = readlink(buffer, ff->filename, FILENAME_MAX);
    if (unlikely(l == 0)) {
        ff->filename[l] = 0;
    } else {
        snprintf(ff->filename, FILENAME_MAX, "unknown filename for fd %d",
                 ff->fd);
    }
    return ff->filename;
}

//-------------------------------------------------------------------------------------------------
// lines
static inline struct pf_lines *new_pflines()
{
    size_t size = PFLINES_INCREASE_STEP * sizeof(struct pf_line);

    struct pf_lines *pfl =
        (struct pf_lines *)malloc(sizeof(struct pf_lines) + size);
    pfl->len  = 0;
    pfl->size = size;
    return pfl;
}

static inline void reset_pfilines(struct pf_lines *pfl)
{
    pfl->len = 0;
}

static inline void free_pflines(struct pf_lines *pfl)
{
    free(pfl);
    pfl = NULL;
}

//-------------------------------------------------------------------------------------------------
// words

static inline struct pf_words *new_pfwords()
{
    size_t size = PFWORDS_INCREASE_STEP * sizeof(char *);

    struct pf_words *pfw =
        (struct pf_words *)malloc(sizeof(struct pf_words) + size);
    pfw->len  = 0;
    pfw->size = size;
    return pfw;
}

static inline void reset_pfwords(struct pf_words *pfw)
{
    pfw->len = 0;
}

static inline void free_pfwords(struct pf_words *pfw)
{
    free(pfw);
    pfw = NULL;
}

//-------------------------------------------------------------------------------------------------
// file

static void procfile_parser(struct proc_file *ff)
{
    char *s = ff->data;
    char *e = &ff->data[ff->len];
    char *t = ff->data;

    while(s < e) {

    }
}

static void procfile_set_separators(struct proc_file *ff, char *seps)
{
}

struct proc_file *procfile_readall(struct proc_file *ff)
{
    ff->len   = 0;
    ssize_t r = 1;

    while (r > 0) {
        ssize_t s = ff->len;      // 使用空间
        ssize_t x = ff->size - s; // 剩余空间

        if (unlikely(!x)) {
            // 空间不够，扩展
            debug("procfile %s buffer size not enough, expand to %lu",
                  procfile_filename(ff), ff->size + PROCFILE_DATA_BUFFER_SIZE);
            // 再增加一个PROCFILE_DATA_BUFFER_SIZE
            ff = (struct proc_file *)realloc(ff, sizeof(struct proc_file) +
                                                     ff->size +
                                                     PROCFILE_DATA_BUFFER_SIZE);
            ff->size += PROCFILE_DATA_BUFFER_SIZE;
        }

        debug("read file '%s', from position %ld with length '%ld'",
              procfile_filename(ff), s, (ff->size - s));
        r = read(ff->fd, &ff->data[s], ff->size - s);
        if (unlikely(r < 0)) {
            error("read file '%s' on fd %d failed, error %s", procfile_filename(ff), ff->fd,
                  strerror(errno));
            procfile_close(ff);
            return NULL;
        }
        ff->len += r;
    }

    debug("rewind file '%s'", procfile_filename(ff));
    lseek(ff->fd, 0, SEEK_SET);

    reset_pfilines(ff->lines);
    reset_pfwords(ff->words);
    procfile_parser(ff);

    debug("read file '%s' done", procfile_filename(ff));

    return ff;
}

// open a /proc or /sys file
struct proc_file *procfile_open(const char *filename, const char *separators,
                                uint32_t flags)
{
    debug("open procfile '%s'", filename);

    int32_t fd = open(filename, O_RDONLY, 0666);
    if (unlikely(fd == -1)) {
        error("open procfile '%s' failed. error: %s", filename,
              strerror(errno));
        return NULL;
    }

    struct proc_file *ff = (struct proc_file *)malloc(
        sizeof(struct proc_file) + PROCFILE_DATA_BUFFER_SIZE);
    ff->filename[0] = '\0';
    ff->fd          = fd;
    ff->size        = PROCFILE_DATA_BUFFER_SIZE;
    ff->len         = 0;
    ff->flags       = flags;
    ff->lines       = new_pflines();
    ff->words       = new_pfwords();

    // set separators 设置分隔符
    procfile_set_separators(ff, separators);
    return ff;
}

void procfile_close(struct proc_file *ff)
{
    if(unlikely(!ff))
        return;

    debug("close procfile %s", procfile_filename(ff));

    if(likely(ff->lines))
        free_pflines(ff->lines);

    if(likely(ff->words))
        free_pfwords(ff->words);

    if(likely(ff->fd != -1)) {
        close(ff->fd);
        ff->fd = -1;
    }
    free(ff);
}
