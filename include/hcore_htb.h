#ifndef _HCORE_HTB_H_INCLUDE_
#define _HCORE_HTB_H_INCLUDE_

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

// HTB: HASH TABLE
#define HCORE_HTB_INDEX_INVALID				-1
#define HCORE_HTB_DONT_FOUNT					-1

enum {
	HCORE_HTB_STATUS_EMPTY = 0,
	HCORE_HTB_STATUS_EXIST,
	HCORE_HTB_STATUS_DELETE
};

enum {
	HCORE_HTB_ERR_SUCCESS = 0,
	HCORE_HTB_ERR_ARGS_ERROR = -1,
	HTP_ERR_MALLOC_FAILED = -2,
	HCORE_HTB_ERR_SIZE_FULL_WARNING = -3,
	HCORE_HTB_ERR_ELEM_EXIST = -4,
	HCORE_HTB_ERR_INSERT_FAILED = -5,
	HCORE_HTB_ERR_ELEM_NO_EXIST = -6,
	HCORE_HTB_ERR_SIZE_FULL = -7
};

typedef enum {
	HCORE_HTB_TYPE_COMMON,	// Auto call malloc to alloc memory
	HCORE_HTB_TYPE_FIXEDSIZE	// Don't call malloc and fixed size
} hcore_htb_type_e;

typedef struct {
	int             status;
    unsigned int    ref;            // number of referred
	char            data[0];
} hcore_htb_elem_t;

typedef struct {
	unsigned int elem_max;
	unsigned int resize;
} hcore_htb_limit_t;

// Function pointer type to get a hash function
typedef unsigned int (*hcore_htb_get_hash_pt)(void *d);

// Function pointer type to compare data function
typedef int (*hcore_htb_compare_pt)(void *d1, void *d2);

typedef struct {
	hcore_htb_compare_pt compare_proc;
	hcore_htb_get_hash_pt get_hash_proc;
} hcore_htb_proc_t;

typedef struct {
	hcore_htb_limit_t       limit;
	hcore_htb_type_e        type;
	unsigned int    size;
	unsigned int    count;
	unsigned int    data_size;
	hcore_htb_elem_t        elem[0];
} hcore_htb_table_t;

#define HCORE_HTB_TABLE_SIZE              (sizeof(hcore_htb_table_t))
#define HCORE_HTB_ELEM_NO_DATA_SIZE       (sizeof(hcore_htb_elem_t))
#define HCORE_HTB_DATA_SIZE(tb)           ((tb)->data_size)
#define HCORE_HTB_ELEM_SIZE(tb)           (HCORE_HTB_ELEM_NO_DATA_SIZE + HCORE_HTB_DATA_SIZE(tb))
#define HCORE_HTB_OFFSET_ELEM(tb)         ((char *)(tb) + HCORE_HTB_TABLE_SIZE)
#define HCORE_HTB_GET_ELEM(tb, i)         (hcore_htb_elem_t *)(HCORE_HTB_OFFSET_ELEM(tb) + i * HCORE_HTB_ELEM_SIZE(tb))


unsigned int hcore_htb_get_size_total(unsigned int size, unsigned int data_size);
int hcore_htb_init(hcore_htb_table_t **ptb, unsigned int size, unsigned int data_size, hcore_htb_type_e type, hcore_htb_limit_t *limit);
int hcore_htb_free(hcore_htb_table_t **ptb);
int hcore_htb_insert(hcore_htb_table_t **ptb, void *data, hcore_htb_proc_t *proc, unsigned int existed);
int hcore_htb_remove(hcore_htb_table_t **ptb, void *data, hcore_htb_proc_t *proc);
unsigned int hcore_htb_find_pos(hcore_htb_table_t *tb, void *data, hcore_htb_proc_t *proc);
void *hcore_shm_create(char *shm_name, int shm_size);
int hcore_fstat(int fd, struct stat *buf);
int hcore_close(int fd);
void hcore_errno_msg(const char *fmt, ...);
int hcore_shm_open(const char *name, int oflag, mode_t mode);
void *hcore_shm_get(char *shm_name, unsigned int *size);
void *hcore_shm_create(char *shm_name, int shm_size);
int hcore_ftruncate(int fd, off_t length);
void *hcore_mmap(void *addr, size_t length, int prot, int flags,
	int fd, off_t offset);
int hcore_int_to_prime(int n, int max);


#endif // !_HCORE_HTB_H_INCLUDE_
