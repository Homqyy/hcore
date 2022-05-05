/*
 * HTB: HASH TABLE
 */

#include "hcore_htb.h"

unsigned int
hcore_htb_get_size_total(unsigned int size, unsigned int data_size)
{
	return HCORE_HTB_TABLE_SIZE + size * (HCORE_HTB_ELEM_NO_DATA_SIZE + data_size);
}

int
hcore_htb_init(hcore_htb_table_t **ptb, unsigned int size, unsigned int data_size, hcore_htb_type_e type, hcore_htb_limit_t *limit)
{
	hcore_htb_table_t   *tb = NULL;
    hcore_htb_elem_t    *elem;
	unsigned int i = 0;

	if (ptb == NULL || *ptb == NULL || size == 0 || data_size == 0)
	{
		return HCORE_HTB_ERR_ARGS_ERROR;
	}

	if (type == HCORE_HTB_TYPE_COMMON)
	{
		tb = malloc(hcore_htb_get_size_total(size, data_size));
		if (tb == NULL)
		{
			return HTP_ERR_MALLOC_FAILED;
		}
	}
	else if (type == HCORE_HTB_TYPE_FIXEDSIZE)
	{
		tb = *ptb;
		if (tb == NULL)
		{
			return HCORE_HTB_ERR_ARGS_ERROR;
		}
	}

	/* Init */
	tb->size = size;
	tb->data_size = data_size;
	tb->count = 0;
	tb->type = type;

	if (limit != NULL)
	{
		tb->limit = *limit;
	}
	else
	{
		tb->limit.elem_max = 0; // Zero is no limit
		tb->limit.resize = 0;
	}

	for (i = 0; i < tb->size; i ++)
	{
	    elem = HCORE_HTB_GET_ELEM(tb, i);

		elem->status = HCORE_HTB_STATUS_EMPTY;
	}

	*ptb = tb;

	return HCORE_HTB_ERR_SUCCESS;
}

int
hcore_htb_free(hcore_htb_table_t **ptb)
{
	hcore_htb_table_t *tb = NULL;

	if (ptb == NULL || *ptb == NULL)
	{
		return HCORE_HTB_ERR_ARGS_ERROR;
	}

	tb = *ptb;

	if (tb->type == HCORE_HTB_TYPE_COMMON && tb != NULL)
	{
		free(tb);
		*ptb = NULL;
	}

	return HCORE_HTB_ERR_SUCCESS;
}

unsigned int
hcore_htb_find_pos(hcore_htb_table_t *tb, void *data, hcore_htb_proc_t *proc)
{
    hcore_htb_elem_t    *elem;
	unsigned int index = 0;
	unsigned int collision_num = 0;

	if (tb == NULL || data == NULL || proc == NULL)
	{
		return HCORE_HTB_ERR_ARGS_ERROR;
	}

	index = proc->get_hash_proc(data) % tb->size;

    elem = HCORE_HTB_GET_ELEM(tb, index);

	while (elem->status != HCORE_HTB_STATUS_EMPTY)
	{
		if (proc->compare_proc(elem->data, data) == 0) // Found element
		{
			break;
		}

		// di = i^2 (eg: 1, 4, 9, ...)
		index += (++collision_num << 1) - 1;
		if (tb->size <= index)
		{
			index %= tb->size;
		}

        elem = HCORE_HTB_GET_ELEM(tb, index);
	}

	return index;
}

int
hcore_htb_insert(hcore_htb_table_t **ptb, void *data, hcore_htb_proc_t *proc, unsigned int existed)
{
	hcore_htb_table_t *tb = NULL;
    hcore_htb_elem_t  *elem;
	unsigned int index = HCORE_HTB_INDEX_INVALID;
	int ret = HCORE_HTB_ERR_SUCCESS;

	if (ptb == NULL || *ptb == NULL)
	{
		return HCORE_HTB_ERR_ARGS_ERROR;
	}

	tb = *ptb;

	if (tb->count == tb->size) // 100%
	{
		return HCORE_HTB_ERR_SIZE_FULL;
	}
	else if (tb->type == HCORE_HTB_TYPE_COMMON && tb->count*10 / tb->size > 7) // greater than 70%
	{
		if (tb->limit.resize)
		{
			/* Resize */
		}
	}
	else if (tb->type == HCORE_HTB_TYPE_FIXEDSIZE && tb->count*10 / tb->size > 7) // greater than 70%
	{
		ret = HCORE_HTB_ERR_SIZE_FULL_WARNING; /* Return warning if no error */
	}

	index = hcore_htb_find_pos(tb, data, proc);

    elem = HCORE_HTB_GET_ELEM(tb, index);

	if (elem->status == HCORE_HTB_STATUS_DELETE)
	{
		elem->status = HCORE_HTB_STATUS_EXIST; /* Success, found deleted element */
        elem->ref = 1;
		tb->count++;
		return ret;
	}
	else if (elem->status == HCORE_HTB_STATUS_EXIST)
	{
	    if (existed) return HCORE_HTB_ERR_ELEM_EXIST; /* Failed Exist */

        elem->ref++;
        return HCORE_HTB_ERR_SUCCESS;
	}

	// Insert a new element
	memcpy(elem->data, data, tb->data_size);
	elem->status = HCORE_HTB_STATUS_EXIST;
    elem->ref = 1;
	tb->count++;

	return ret;
}

int
hcore_htb_remove(hcore_htb_table_t **ptb, void *data, hcore_htb_proc_t *proc)
{
    hcore_htb_table_t *tb = NULL;
    hcore_htb_elem_t  *elem;
    unsigned int index = HCORE_HTB_INDEX_INVALID;

    if (ptb == NULL || *ptb == NULL || data == NULL || proc == NULL)
    {
        return HCORE_HTB_ERR_ARGS_ERROR;
    }

    tb = *ptb;

    index = hcore_htb_find_pos(tb, data, proc);

    elem = HCORE_HTB_GET_ELEM(tb, index);
    if (elem->status == HCORE_HTB_STATUS_EXIST)
    {
        if (--elem->ref == 0)
        {
            elem->status = HCORE_HTB_STATUS_DELETE;
            tb->count--;
        }

        return HCORE_HTB_ERR_SUCCESS;
    }

    return HCORE_HTB_ERR_ELEM_NO_EXIST;
}


void *
hcore_shm_create(char *shm_name, int shm_size)
{
    int      fd = -1;
    void    *ptr = NULL;
    void    *ret_ptr = NULL;


    if (shm_name == NULL)
    {
        return NULL;
    }

	fd = hcore_shm_open(shm_name, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (fd == -1) {
		goto OUT;
	}

	if (hcore_ftruncate(fd, shm_size) == -1) {
		goto OUT;
	}

	// Map shared memory object
	ptr = hcore_mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) {
		goto OUT;
	}

	ret_ptr = ptr;

OUT:
	hcore_close(fd);

	return ret_ptr;
}

void *
hcore_shm_get(char *shm_name, unsigned int *size)
{
	struct stat sb;
	int		fd = -1;
	void	*ptr = NULL;
	void 	*ret_ptr = NULL;

	fd = hcore_shm_open(shm_name, O_RDWR, S_IREAD | S_IWRITE);
	if (fd == -1)
	{
		goto OUT;
	}

	if (hcore_fstat(fd, &sb) == -1)
	{
		 goto OUT;
	}

	// Map shared memory object
	ptr = hcore_mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) {
		goto OUT;
	}

    if (size) *size = sb.st_size;
	ret_ptr = ptr;

OUT:
	hcore_close(fd);

	return ret_ptr;
}


int
hcore_shm_open(const char *name, int oflag, mode_t mode)
{
	int fd;

	if ((fd = shm_open(name, oflag, mode)) == -1) {
		hcore_errno_msg("open shm %s error", name);
	}

	return fd;
}



void
hcore_errno_msg(const char *fmt, ...)
{
	return;
}


int
hcore_close(int fd)
{
	int ret;

again:
	if ((ret = close(fd)) == -1) {
		if (errno == EINTR)
		{
			goto again;
		}
		hcore_errno_msg("close error");
	}

	return ret;
}


int
hcore_fstat(int fd, struct stat *buf)
{
	int ret;

	if ((ret = fstat(fd, buf)) == -1) {
		hcore_errno_msg("fstat error");
	}

	return ret;
}


int
hcore_ftruncate(int fd, off_t length)
{
	int ret;

again:
	if ((ret = ftruncate(fd, length)) == -1) {
		if (errno == EINTR) {
			goto again;
		}
		hcore_errno_msg("ftruncate error");
	}

	return ret;
}


void *
hcore_mmap(void *addr, size_t length, int prot, int flags,
	int fd, off_t offset)
{
	void *ptr;

	if ((ptr = mmap(addr, length, prot, flags, fd, offset)) == ((void *) -1)) {
		hcore_errno_msg("mmap error");
		ptr = NULL;
	}

	return ptr;
}


int
hcore_int_to_prime(int n, int max)
{
	double x, y, i, max_value;
	int integer;


	if (max == 0)
	{
		max_value = INT_MAX;
	}
	else
	{
		max_value = (double) max;
	}

	for (x = n; x <= max_value; x++) // find first prime from x to max
	{
		for (i = 2.0; ; i++)
		{
			y = x / i;
			integer = (int) y;

			if (y != integer && i == x - 1) // is prime
			{
				return (int) x;
			}

			if (y == integer)
			{
				break; // Next x
			}
		}
	}

	return -1;
}
