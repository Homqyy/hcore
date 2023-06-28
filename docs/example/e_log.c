#include <hcore_pool.h>

int
main()
{
    // 打开标准错误日志
    hcore_log_t errlog;

    if (hcore_open_log(&errlog, HCORE_LOG_FILE_STDERR, HCORE_LOG_ERR)
        != HCORE_OK)
    {
        hcore_write_stderr("Failed to open stderr log.\n");
        return -1;
    }

    // 创建内存池
    hcore_pool_t *pool = hcore_create_pool(HCORE_POOL_SIZE_DEFAULT, &errlog);

    if (pool == NULL)
    {
        hcore_write_stderr("Failed to create memory pool.\n");
        hcore_destroy_log(&errlog);
        return -1;
    }

    // 创建日志对象
    hcore_log_t *log =
        hcore_create_log(pool, "/var/log/myapp.log", HCORE_LOG_DEBUG);
    if (log == NULL)
    {
        hcore_write_stderr("Failed to create log object.\n");
        hcore_destroy_log(&errlog);
        hcore_destroy_pool(pool);
        return -1;
    }

    // 实践中我们也会让pool持有自己的log对象，它会跟着pool的销毁一同销毁
    pool->log = log;

    hcore_destroy_log(&errlog);

    // 使用日志对象记录一些消息
    hcore_log_debug(log, 0, "This is a debug message.");
    hcore_log_error(HCORE_LOG_WARN, log, 0, "This is a warning message.");
    hcore_log_error(HCORE_LOG_ERR, log, 0, "This is an error message.");

    // 清理日志对象和内存池
    hcore_destroy_pool(pool);

    return 0;
}