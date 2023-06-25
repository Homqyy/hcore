/**
 * @file hcore_pack.c
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-10-18
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#include <hcore_debug.h>
#include <hcore_pack.h>

static hcore_int_t hcore_pack_add_value(hcore_pack_t *p, const char *name,
                                    hcore_pack_elem_type_e type,
                                    const void *         value);

static hcore_pack_elem_t *hcore_pack_elem_get(hcore_pack_t *p, const hcore_str_t *name,
                                          const hcore_pack_elem_type_e type);

static hcore_pack_elem_t *hcore_pack_elem_create(hcore_pool_t *pool, hcore_str_t *name,
                                             const hcore_pack_elem_type_e type,
                                             unsigned int               num);

static hcore_int_t hcore_pack_elem_add_data(hcore_pack_elem_t *e, const void *data,
                                        size_t size);

static hcore_int_t hcore_pack_elem_add_int(hcore_pack_elem_t *   e,
                                       const unsigned int *i);

static hcore_int_t hcore_pack_elem_add_str(hcore_pack_elem_t *e,
                                       const hcore_str_t *str);

static int hcore_pack_read_int(const hcore_uchar_t *buffer, unsigned int *i);
static hcore_uchar_t *hcore_pack_write_int(unsigned int i, hcore_uchar_t *buf);
static hcore_uchar_t *hcore_pack_write_str(const hcore_str_t *str, hcore_uchar_t *buf);

hcore_uchar_t *
hcore_pack_write(hcore_pack_t *pack, size_t *size)
{
    unsigned int     num = HCORE_LIST_NUM(pack->elements);
    unsigned int     i;
    hcore_uint_t       j;
    hcore_pack_elem_t *e;
    void *           value;

    if (num == 0) { return NULL; }

    if (pack->buf) { return pack->buf; }

    hcore_uchar_t *buf = hcore_pnalloc(pack->pool, pack->total_size);
    if (buf == NULL) { return NULL; }

    hcore_uchar_t *p = buf;

    p = hcore_pack_write_int(num, p);

    for (i = 0; i < num; i++)
    {
        e = HCORE_LIST_DATA(pack->elements, i);

        if (e == NULL) return NULL;

        p = hcore_pack_write_str(&e->name, p); // element name

        p = hcore_pack_write_int(e->type, p); // type of element

        p = hcore_pack_write_int(e->values->nelts, p); // number of value

        value = e->values->elts;
        for (j = 0; j < e->values->nelts; j++)
        {
            switch (e->type)
            {
            case HCORE_PACK_ELEM_TYPE_INT:
                p = hcore_pack_write_int(((unsigned int *)value)[j], p);
                break;

            case HCORE_PACK_ELEM_TYPE_STR:
            case HCORE_PACK_ELEM_TYPE_DATA:
                {
                    hcore_pack_elem_value_data_t data =
                        ((hcore_pack_elem_value_data_t *)value)[j];

                    hcore_str_t str = {data.size, data.data};

                    p = hcore_pack_write_str(&str, p);
                }
                break;

            default: continue;
            }
        }
    }

    hcore_assert(p == buf + pack->total_size);

    pack->buf = buf;
    *size     = p - buf;

    return buf;
}

static hcore_uchar_t *
hcore_pack_write_int(unsigned int i, hcore_uchar_t *buf)
{
    return hcore_cpymem(buf, &i, sizeof(i));
}

static hcore_uchar_t *
hcore_pack_write_str(const hcore_str_t *str, hcore_uchar_t *buf)
{
    buf = hcore_pack_write_int(str->len, buf);
    return hcore_cpymem(buf, str->data, str->len);
}

ssize_t
hcore_pack_read(const hcore_uchar_t *buffer, const hcore_uchar_t *last,
              hcore_pack_t *pack)
{
    const hcore_uchar_t *p;

    hcore_assert(buffer && last && buffer <= last);

    if (last < buffer) { return -1; }

    p = buffer;

    while (p + pack->need_size <= last)
    {
        switch (pack->phase)
        {
        case hcore_pack_phase_init:
            {
                unsigned int num;

                p += hcore_pack_read_int(p, &num);

                if (num == 0)
                {
                    return -1; // must '> 0'
                }

                pack->num     = num;
                pack->cur_num = 0;

                pack->phase     = hcore_pack_phase_elem_name_len;
                pack->need_size = sizeof(unsigned int);
            }

            break;

        case hcore_pack_phase_elem_name_len:
            {
                unsigned int len;

                if (pack->max && pack->max < pack->num) { return -1; }

                p += hcore_pack_read_int(p, &len);

                pack->elem_name.len = len;

                pack->phase     = hcore_pack_phase_elem_name;
                pack->need_size = len;
            }
            break;

        case hcore_pack_phase_elem_name:
            {
                pack->elem_name.data =
                    hcore_pnalloc(pack->pool, pack->elem_name.len);
                if (pack->elem_name.data == NULL) { return -1; }

                hcore_memcpy(pack->elem_name.data, p, pack->elem_name.len);
                p += pack->elem_name.len;

                pack->phase     = hcore_pack_phase_elem_type;
                pack->need_size = sizeof(unsigned int);
            }
            break;

        case hcore_pack_phase_elem_type:
            {
                p += hcore_pack_read_int(p, &pack->elem_type);

                pack->phase     = hcore_pack_phase_value_num;
                pack->need_size = sizeof(unsigned int);
            }
            break;

        case hcore_pack_phase_value_num:
            {
                p += hcore_pack_read_int(p, &pack->value_num);

                if (pack->value_num == 0)
                {
                    return -1; // must '> 0'
                }

                pack->cur_value_num = 0;

                pack->cur_elem =
                    hcore_pack_elem_create(pack->pool, &pack->elem_name,
                                         pack->elem_type, pack->value_num);
                if (pack->cur_elem == NULL) { return -1; }

                if (hcore_list_insert(pack->elements, pack->cur_elem)
                    != HCORE_OK)
                {
                    return -1;
                }

                switch (pack->elem_type)
                {
                case HCORE_PACK_ELEM_TYPE_INT:
                    pack->phase     = hcore_pack_phase_value_data_int;
                    pack->need_size = sizeof(unsigned int);
                    break;

                case HCORE_PACK_ELEM_TYPE_STR:
                case HCORE_PACK_ELEM_TYPE_DATA:
                    pack->phase     = hcore_pack_phase_value_data_value_len;
                    pack->need_size = sizeof(unsigned int);
                    break;

                default: return -1; // unknown type
                }
            }
            break;

        case hcore_pack_phase_value_data_int:
            {
                unsigned int value_i;
                p += hcore_pack_read_int(p, &value_i);
                pack->cur_value_num++;

                if (hcore_pack_elem_add_int(pack->cur_elem, &value_i)
                    != HCORE_OK)
                {
                    return -1;
                }

                if (pack->cur_value_num == pack->value_num)
                {
                    if (++pack->cur_num == pack->num)
                    {
                        pack->phase = hcore_pack_phase_done;
                        goto done;
                    }

                    pack->phase     = hcore_pack_phase_elem_name_len;
                    pack->need_size = sizeof(unsigned int);
                }
                else
                {
                    pack->phase     = hcore_pack_phase_value_data_int;
                    pack->need_size = sizeof(unsigned int);
                }
            }
            break;

        case hcore_pack_phase_value_data_value_len:
            {
                p += hcore_pack_read_int(p, &pack->value_len);

                pack->phase     = hcore_pack_phase_value_data_value;
                pack->need_size = pack->value_len;
            }
            break;

        case hcore_pack_phase_value_data_value:
            {
                if (pack->elem_type == HCORE_PACK_ELEM_TYPE_STR)
                {
                    hcore_str_t v_str;

                    v_str.len  = pack->value_len;
                    v_str.data = (hcore_uchar_t *)p;
                    if (hcore_pack_elem_add_str(pack->cur_elem, &v_str)
                        != HCORE_OK)
                    {
                        return -1;
                    }
                }
                else
                {
                    if (hcore_pack_elem_add_data(pack->cur_elem, p,
                                               pack->value_len)
                        != HCORE_OK)
                    {
                        return -1;
                    }
                }

                p += pack->value_len;
                pack->cur_value_num++;

                if (pack->cur_value_num == pack->value_num)
                {
                    if (++pack->cur_num == pack->num)
                    {
                        pack->phase = hcore_pack_phase_done;
                        goto done;
                    }

                    pack->phase     = hcore_pack_phase_elem_name_len;
                    pack->need_size = sizeof(unsigned int);
                }
                else
                {
                    pack->phase     = hcore_pack_phase_value_data_value_len;
                    pack->need_size = sizeof(unsigned int);
                }
            }
            break;

        case hcore_pack_phase_done: goto done;
        }
    }

done:

    return p - buffer;
}

static hcore_int_t
hcore_pack_elem_add_data(hcore_pack_elem_t *e, const void *data, size_t size)
{
    hcore_pack_elem_value_data_t *v;


    v = hcore_array_push(e->values);
    if (v == NULL) { return HCORE_ERROR; }

    v->data = hcore_pnalloc(e->pool, size);
    if (v->data == NULL) { return HCORE_ERROR; }

    hcore_memcpy(v->data, data, size);
    v->size = size;

    return HCORE_OK;
}

static hcore_int_t
hcore_pack_elem_add_str(hcore_pack_elem_t *e, const hcore_str_t *str)
{
    hcore_str_t *v;


    v = hcore_array_push(e->values);
    if (v == NULL) { return HCORE_ERROR; }

    v->data = hcore_pnalloc(e->pool, str->len + 1);
    if (v->data == NULL) { return HCORE_ERROR; }

    v->len = hcore_snprintf(v->data, str->len, "%s", str->data) - v->data;
    v->data[v->len] = '\0';

    hcore_assert(v->len == str->len);

    return HCORE_OK;
}

static hcore_int_t
hcore_pack_elem_add_int(hcore_pack_elem_t *e, const unsigned int *i)
{
    unsigned int *v;

    v = hcore_array_push(e->values);
    if (v == NULL) { return HCORE_ERROR; }

    *v = *i;

    return HCORE_OK;
}

static int
hcore_pack_read_int(const hcore_uchar_t *buffer, unsigned int *i)
{
    *i = *((unsigned int *)buffer);

    return sizeof(unsigned int);
}

hcore_pack_t *
hcore_pack_create(hcore_log_t *log, hcore_uint_t max)
{
    hcore_pack_t *p;
    hcore_pool_t *pool;


    pool = hcore_create_pool(HCORE_POOL_SIZE_DEFAULT, log);
    if (pool == NULL) { return NULL; }

    p = hcore_pcalloc(pool, sizeof(hcore_pack_t));
    if (p == NULL)
    {
        hcore_destroy_pool(pool);
        return NULL;
    }

    if (hcore_pack_init(p, pool, max) != HCORE_OK)
    {
        hcore_destroy_pool(pool);
        return NULL;
    }

    return p;
}

void
hcore_pack_destroy(hcore_pack_t *p)
{
    if (p == NULL) { return; }

    if (--p->ref == 0)
    {
        if (p->pool) hcore_destroy_pool(p->pool);
    }
}

int
hcore_pack_cmp_name(const void *p1, const void *p2)
{
    const hcore_pack_elem_t *o1, *o2;


    if (p1 == NULL || p2 == NULL) { return 0; }

    o1 = *(const hcore_pack_elem_t **)p1;
    o2 = *(const hcore_pack_elem_t **)p2;

    if (o1 == NULL || o2 == NULL) { return 0; }

    if (o1->type != o2->type) { return o1->type - o2->type; }

    if (o1->name.len != o2->name.len) { return o1->name.len - o2->name.len; }

    return hcore_memcmp(o1->name.data, o2->name.data, o1->name.len);
}

hcore_int_t
hcore_pack_get_int(hcore_pack_t *p, const char *name, unsigned int *i)
{
    unsigned int *i_array;
    hcore_uint_t    num;

    *i = 0;

    if (hcore_pack_get_int_array(p, name, &i_array, &num) != HCORE_OK)
    {
        return HCORE_ERROR;
    }

    *i = i_array[0];

    return HCORE_OK;
}

hcore_int_t
hcore_pack_get_int_array(hcore_pack_t *p, const char *name, unsigned int **i,
                       hcore_uint_t *num)
{
    hcore_pack_elem_t *e;
    hcore_str_t        name_str;


    if (p == NULL || name == NULL || i == NULL) { return HCORE_ERROR; }

    *i   = NULL;
    *num = 0;

    name_str.data = (u_char *)name;
    name_str.len  = strlen(name);

    e = hcore_pack_elem_get(p, &name_str, HCORE_PACK_ELEM_TYPE_INT);
    if (e == NULL) { return HCORE_ERROR; }

    if (e->values->nelts == 0) { return HCORE_ERROR; }

    *i   = e->values->elts;
    *num = e->values->nelts;

    return HCORE_OK;
}

hcore_int_t
hcore_pack_get_str(hcore_pack_t *p, const char *name, hcore_str_t *str)
{
    hcore_str_t *str_array;
    hcore_uint_t num;

    hcore_assert(p && name && str);

    hcore_str_set(str, "");

    if (hcore_pack_get_str_array(p, name, &str_array, &num) != HCORE_OK)
    {
        return HCORE_ERROR;
    }

    *str = str_array[0];

    return HCORE_OK;
}

hcore_int_t
hcore_pack_get_str_array(hcore_pack_t *p, const char *name, hcore_str_t **str,
                       hcore_uint_t *num)
{
    hcore_pack_elem_t *e = NULL;
    hcore_str_t        name_str;


    // Validate arguments
    if (p == NULL || name == NULL || str == NULL) { return HCORE_ERROR; }

    *str = NULL;
    *num = 0;

    name_str.data = (hcore_uchar_t *)name;
    name_str.len  = strlen(name);

    e = hcore_pack_elem_get(p, &name_str, HCORE_PACK_ELEM_TYPE_STR);
    if (e == NULL) { return HCORE_ERROR; }

    if (e->values->nelts == 0) { return HCORE_ERROR; }

    *str = e->values->elts;
    *num = e->values->nelts;

    return HCORE_OK;
}

hcore_int_t
hcore_pack_get_data(hcore_pack_t *p, const char *name, void **data, size_t *size)
{
    hcore_pack_elem_value_data_t *data_array;
    hcore_uint_t                  num;

    hcore_assert(p && name && data && size);

    *data = NULL;
    if (size) *size = 0;

    if (hcore_pack_get_data_array(p, name, &data_array, &num) != HCORE_OK)
    {
        return HCORE_ERROR;
    }

    *data = data_array[0].data;
    *size = data_array[0].size;

    return HCORE_OK;
}

hcore_int_t
hcore_pack_get_data_array(hcore_pack_t *p, const char *name,
                        hcore_pack_elem_value_data_t **data, hcore_uint_t *num)
{
    hcore_pack_elem_t *e = NULL;
    hcore_str_t        name_str;


    hcore_assert(p && name && data && num);

    name_str.data = (u_char *)name;
    name_str.len  = strlen(name);

    e = hcore_pack_elem_get(p, &name_str, HCORE_PACK_ELEM_TYPE_DATA);
    if (e == NULL) { return HCORE_ERROR; }

    if (e->values->nelts == 0) { return HCORE_ERROR; }

    *data = e->values->elts;
    *num  = e->values->nelts;

    return HCORE_OK;
}

hcore_int_t
hcore_pack_add_int(hcore_pack_t *p, const char *name, unsigned int i)
{
    // Validate arguments
    if (p == NULL || name == NULL)
    {
        return HCORE_ERROR;
        ;
    }

    return hcore_pack_add_value(p, name, HCORE_PACK_ELEM_TYPE_INT, &i);
}

hcore_int_t
hcore_pack_add_str(hcore_pack_t *p, const char *name, const hcore_str_t *str)
{
    // Validate arguments
    if (p == NULL || name == NULL || str == NULL) { return HCORE_ERROR; }

    return hcore_pack_add_value(p, name, HCORE_PACK_ELEM_TYPE_STR, str);
}

hcore_int_t
hcore_pack_add_str_char(hcore_pack_t *p, const char *name, const char *str)
{
    // Validate arguments
    if (p == NULL || name == NULL || str == NULL) { return HCORE_ERROR; }

    hcore_str_t data;

    data.data = (void *)str;
    data.len  = strlen((char *)str);

    return hcore_pack_add_value(p, name, HCORE_PACK_ELEM_TYPE_STR, &data);
}

hcore_int_t
hcore_pack_add_data(hcore_pack_t *p, const char *name, const void *data,
                  size_t size)
{
    hcore_pack_elem_value_data_t v_data;

    // Validate arguments
    if (p == NULL || name == NULL || data == NULL) { return HCORE_ERROR; }

    v_data.data = (void *)data;
    v_data.size = size;

    return hcore_pack_add_value(p, name, HCORE_PACK_ELEM_TYPE_DATA, &v_data);
}

static hcore_int_t
hcore_pack_add_value(hcore_pack_t *p, const char *name, hcore_pack_elem_type_e type,
                   const void *value)
{
    hcore_pack_elem_t *e = NULL;
    hcore_str_t        elem_name;


    elem_name.data = (u_char *)name;
    elem_name.len  = strlen(name);

    // get element

    e = hcore_pack_elem_get(p, &elem_name, type);

    if (e == NULL)
    {
        if (p->max && p->max <= HCORE_LIST_NUM(p->elements))
        {
            return HCORE_ERROR;
        }

        e = hcore_pack_elem_create(p->pool, &elem_name, type, 0);
        if (e == NULL) { return HCORE_ERROR; }

        if (hcore_list_insert(p->elements, e) != HCORE_OK)
        {
            return HCORE_ERROR;
        }

        p->total_size += 4; /* length of element name */
        ;
        p->total_size += elem_name.len;

        p->total_size += 4; /* type of element */
        ;
        p->total_size += 4; /* number of value */
        ;
    }

    switch (type)
    {
    case HCORE_PACK_ELEM_TYPE_INT:
        p->total_size += sizeof(unsigned int);
        return hcore_pack_elem_add_int(e, value);

    case HCORE_PACK_ELEM_TYPE_STR:
        {
            hcore_str_t *str;

            str = (hcore_str_t *)value;

            p->total_size += 4; /* size of value */
            p->total_size += str->len;

            return hcore_pack_elem_add_str(e, str);
        }

    case HCORE_PACK_ELEM_TYPE_DATA:
        {
            hcore_pack_elem_value_data_t *data;

            data = (hcore_pack_elem_value_data_t *)value;

            p->total_size += 4; /* size of value */
            p->total_size += data->size;

            return hcore_pack_elem_add_data(e, data->data, data->size);
        }

    default: return HCORE_ERROR;
    }

    return HCORE_ERROR;
}

static hcore_pack_elem_t *
hcore_pack_elem_get(hcore_pack_t *p, const hcore_str_t *name,
                  const hcore_pack_elem_type_e type)
{
    hcore_pack_elem_t t;


    // Validate arguments
    if (p == NULL || name == NULL) { return NULL; }

    // Search
    t.name = *name;
    t.type = type;

    return hcore_list_search(p->elements, &t);
}

static hcore_pack_elem_t *
hcore_pack_elem_create(hcore_pool_t *pool, hcore_str_t *name,
                     const hcore_pack_elem_type_e type, unsigned int num)
{
    hcore_pack_elem_t *e;
    size_t           size;


    e = hcore_palloc(pool, sizeof(hcore_pack_elem_t));
    if (e == NULL) { return NULL; }

    e->name.len  = name->len;
    e->name.data = hcore_palloc(pool, e->name.len);
    if (e->name.data == NULL) { return NULL; }

    hcore_memcpy(e->name.data, name->data, e->name.len);

    e->type = type;
    e->pool = pool;

    switch (type)
    {
    case HCORE_PACK_ELEM_TYPE_INT: size = sizeof(unsigned int); break;

    case HCORE_PACK_ELEM_TYPE_STR:
    case HCORE_PACK_ELEM_TYPE_DATA:
        size = sizeof(hcore_pack_elem_value_data_t);
        break;

    default: return NULL;
    }

    if (num == 0) num = 1; // default value

    e->values = hcore_array_create(pool, num, size);
    if (e->values == NULL) { return NULL; }

    return e;
}