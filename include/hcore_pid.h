/**
 * @file hcore_pid.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-12-08
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_PID_H_INCLUDED_
#define _HCORE_PID_H_INCLUDED_

#include <hcore_types.h>


#define hcore_destroy_pidfile(file) unlink(file)
pid_t      hcore_create_pidfile(const char *pidfile, pid_t pid);
pid_t      hcore_get_pid(const char *pidfile);
hcore_bool_t hcore_proc_is_running(const char *pidfile);

#endif // !_HCORE_PID_H_INCLUDED_
