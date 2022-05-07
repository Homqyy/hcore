/**
 * @file hcore_pid.c
 * @author homqyy (yilupiaoxuewhq@163.com)
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

#include <hcore_debug.h>
#include <hcore_log.h>
#include <hcore_pid.h>
#include <hcore_string.h>

#include <signal.h>

pid_t
hcore_create_pidfile(const char *pidfile, pid_t pid)
{
    FILE *fp;

    hcore_assert(pidfile);

    if (pidfile == NULL) return HCORE_ERROR;

    fp = fopen(pidfile, "w+");
    if (fp == NULL) return HCORE_ERROR;

    fprintf(fp, "%d", pid);

    fclose(fp);

    return pid;
}

hcore_bool_t
hcore_proc_is_running(const char *pidfile)
{
    hcore_assert(pidfile);

    if (pidfile == NULL) return 0;

    if (access(pidfile, F_OK) == 0)
    {
        pid_t pid = hcore_get_pid(pidfile);
        if (pid == HCORE_ERROR) return 0;

        return kill(pid, 0) == 0;
    }

    return 0;
}

pid_t
hcore_get_pid(const char *pidfile)
{
    FILE *fp;
    pid_t pid;

    hcore_assert(pidfile);

    if (pidfile == NULL) return HCORE_ERROR;

    fp = fopen(pidfile, "r");
    if (fp == NULL) return HCORE_ERROR;

    if (fscanf(fp, "%d", &pid) != 1)
    {
        fclose(fp);
        return HCORE_ERROR;
    }

    fclose(fp);

    return pid;
}