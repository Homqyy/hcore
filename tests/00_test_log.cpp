extern "C"
{
    #include <hcore_base.h>
    #include <hcore_constant.h>
    #include <hcore_log.h>
    #include <hcore_pool.h>
}

#include <gtest/gtest.h>
#include <egtest.h>

class LogTest : public ::testing::Test {
  protected:
    void
    SetUp() override
    {
        fLogFile = (char *)"/tmp/hcore_log_test.log";
        hcore_open_log(&fLog, HCORE_LOG_FILE_STDOUT, HCORE_LOG_DEBUG);

        fPool = hcore_create_pool(HCORE_POOL_SIZE_DEFAULT, &fLog);

    }

    void
    TearDown() override
    {
        hcore_destroy_log(&fLog);
        hcore_destroy_pool(fPool);
    }

    char                    *fLogFile;
    hcore_log_t              fLog;
    hcore_pool_t            *fPool;
    etesting::MatchesRegex   fMatcher;
};

TEST_F(LogTest, CreateWithNullArgs)
{
#ifdef _HCORE_DEBUG
    EXPECT_DEATH(hcore_create_log(NULL, NULL, HCORE_LOG_ERR), "hcore_create_log: Assertion");
    EXPECT_DEATH(hcore_create_log(NULL, fLogFile, HCORE_LOG_ERR), "hcore_create_log: Assertion");
    EXPECT_DEATH(hcore_create_log(fPool, NULL, HCORE_LOG_ERR), "hcore_create_log: Assertion");
#endif
}

TEST_F(LogTest, CreateWithInvalidLogLevel)
{
    EXPECT_FALSE(hcore_create_log(fPool, fLogFile, 99));
    EXPECT_FALSE(hcore_create_log(fPool, fLogFile, HCORE_LOG_UNSET));
}

TEST_F(LogTest, CreateLog)
{
    // create a log with a file
    hcore_log_t *log = hcore_create_log(fPool, fLogFile, HCORE_LOG_ERR);

    ASSERT_TRUE(log);
    EXPECT_EQ(log->log_level, HCORE_LOG_ERR);
    EXPECT_GT(log->fd, 0);
    EXPECT_EQ(log->filename, fLogFile);
    EXPECT_FALSE(log->object);
    EXPECT_FALSE(log->handler);
    EXPECT_FALSE(log->data);
    EXPECT_FALSE(log->action);
    EXPECT_FALSE(log->get_time);
    EXPECT_FALSE(log->internal);

    hcore_destroy_log(log);

    // create a log with stdout
    log = hcore_create_log(fPool, HCORE_LOG_FILE_STDOUT, HCORE_LOG_ERR);

    ASSERT_TRUE(log);
    EXPECT_EQ(log->log_level, HCORE_LOG_ERR);
    EXPECT_GT(log->fd, 0);
    EXPECT_EQ(log->filename, HCORE_LOG_FILE_STDOUT);
    EXPECT_FALSE(log->object);
    EXPECT_FALSE(log->handler);
    EXPECT_FALSE(log->data);
    EXPECT_FALSE(log->action);
    EXPECT_FALSE(log->get_time);
    EXPECT_TRUE(log->internal);

    hcore_destroy_log(log);

    // create a log with stderr
    log = hcore_create_log(fPool, HCORE_LOG_FILE_STDERR, HCORE_LOG_ERR);

    ASSERT_TRUE(log);
    EXPECT_EQ(log->log_level, HCORE_LOG_ERR);
    EXPECT_GT(log->fd, 0);
    EXPECT_EQ(log->filename, HCORE_LOG_FILE_STDERR);
    EXPECT_FALSE(log->object);
    EXPECT_FALSE(log->handler);
    EXPECT_FALSE(log->data);
    EXPECT_FALSE(log->action);
    EXPECT_FALSE(log->get_time);
    EXPECT_TRUE(log->internal);

    hcore_destroy_log(log);

    // create a log with a file and a custom log level
    int levels[] = {
        HCORE_LOG_STDERR,
        HCORE_LOG_EMERG,
        HCORE_LOG_ALERT,
        HCORE_LOG_CRIT,
        HCORE_LOG_ERR,
        HCORE_LOG_WARN,
        HCORE_LOG_NOTICE,
        HCORE_LOG_INFO,
        HCORE_LOG_DEBUG,
    };

    for (unsigned int i = 0; i < HCORE_ARRAY_NUM(levels); i++)
    {
        log = hcore_create_log(fPool, fLogFile, levels[i]);

        ASSERT_TRUE(log);
        EXPECT_EQ(log->log_level, levels[i]);
        EXPECT_GT(log->fd, 0);
        EXPECT_EQ(log->filename, fLogFile);
        EXPECT_FALSE(log->object);
        EXPECT_FALSE(log->handler);
        EXPECT_FALSE(log->data);
        EXPECT_FALSE(log->action);
        EXPECT_FALSE(log->get_time);
        EXPECT_FALSE(log->internal);

        hcore_destroy_log(log);
    }
}

TEST_F(LogTest, logParseLevel)
{
    EXPECT_EQ(hcore_log_parse_level("stderr"), HCORE_LOG_STDERR);
    EXPECT_EQ(hcore_log_parse_level("emerg"), HCORE_LOG_EMERG);
    EXPECT_EQ(hcore_log_parse_level("alert"), HCORE_LOG_ALERT);
    EXPECT_EQ(hcore_log_parse_level("crit"), HCORE_LOG_CRIT);
    EXPECT_EQ(hcore_log_parse_level("error"), HCORE_LOG_ERR);
    EXPECT_EQ(hcore_log_parse_level("warn"), HCORE_LOG_WARN);
    EXPECT_EQ(hcore_log_parse_level("notice"), HCORE_LOG_NOTICE);
    EXPECT_EQ(hcore_log_parse_level("info"), HCORE_LOG_INFO);
    EXPECT_EQ(hcore_log_parse_level("debug"), HCORE_LOG_DEBUG);
    EXPECT_EQ(hcore_log_parse_level("invalid"), HCORE_ERROR);
}

TEST_F(LogTest, openLog)
{
    hcore_log_t log;

    // open a log with a file
    ASSERT_EQ(hcore_open_log(&log, fLogFile, HCORE_LOG_ERR), HCORE_OK);
    EXPECT_EQ(log.log_level, HCORE_LOG_ERR);
    EXPECT_GT(log.fd, 0);
    EXPECT_EQ(log.filename, fLogFile);
    EXPECT_FALSE(log.object);
    EXPECT_FALSE(log.handler);
    EXPECT_FALSE(log.data);
    EXPECT_FALSE(log.action);
    EXPECT_FALSE(log.get_time);
    EXPECT_FALSE(log.internal);

    hcore_destroy_log(&log);
}

TEST_F(LogTest, logGetTime)
{
    hcore_uchar_t time[HCORE_LOG_TIME_LENGTH];

    hcore_log_get_time(time);

    auto result = fMatcher.matchesRegex((const char *)time, "[0-9]{4}/[0-9]{2}/[0-9]{2} [0-9]{2}:[0-5][0-9]:[0-5][0-9] [+-]0000 GMT");

    EXPECT_TRUE(result.first) << "fail to match string: " << result.second;
}

TEST_F(LogTest, logGetLocaltime)
{
    hcore_uchar_t time[HCORE_LOG_TIME_LENGTH];

    hcore_log_get_localtime(time);

    auto result = fMatcher.matchesRegex((const char *)time, "[0-9]{4}/[0-9]{2}/[0-9]{2} [0-9]{2}:[0-5][0-9]:[0-5][0-9] [+-][0-9]{4} [A-Z]{3}");

    EXPECT_TRUE(result.first) << "fail to match string: " << result.second;
}