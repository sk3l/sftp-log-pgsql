#include <sys/types.h>

#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <memory>
#include <sstream>
#include <pqxx/pqxx>
#include <pqxx/except>

#include "sftp-callback.h"
#include "sftp-plugin.h"

static std::unique_ptr<pqxx::connection> dbConn_;
static std::string connStr_ = "postgres://mskelton:abc123@127.0.0.1/postgres";
using sqlconn_t = pqxx::connection;

static std::unique_ptr<std::ofstream> log_;

static int init()
{
    if (log_ == nullptr)
        log_.reset(new std::ofstream("/tmp/sftp-log-pgsql.log"));

   try
    {
        if (dbConn_ == nullptr)
        {
            *(log_.get()) << "Initializing PgSQL conn: "
                      << connStr_
                      << std::endl;
            dbConn_.reset(new sqlconn_t(connStr_));
        }
        return 0;
    }
    catch (const pqxx::failure & e)
    {
        *(log_.get()) << "Error initializing PgSQL conn: "
                      << e.what()
                      << std::endl;
        return 1;
    }
}

static void do_sql(const std::string & data)
{
    try
    {
        *(log_.get()) << "Invoking do_sql()"
                      << std::endl;

         dbConn_->prepare(
         "sftp_pgin_ins",
         "insert into public.sftp_logs (log_event, log_time) values ($1, $2)"
        );

        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        std::stringstream tstmp;
        tstmp << std::put_time(&tm, "%FT%T");

        auto tsstr = tstmp.str();

        pqxx::work pqw(*dbConn_);
        auto result = pqw.prepared("sftp_pgin_ins")(data)(tsstr).exec();
        pqw.commit();
    }
    catch (const pqxx::failure & e)
    {
        *(log_.get()) << "Error executing SQL: "
                      << e.what()
                      << std::endl;
     }
}

extern "C" int sftp_cf_open_file(u_int32_t rqstid,
        const char * filename,
        u_int32_t access,
        u_int32_t flags,
        int * handle)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_open_file()"
                  << std::endl;

    std::stringstream ss;
    ss << "Received open_file event, filename ="
       << filename
       << ", access = "
       << access
       << ", flags = "
       << flags;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_open_dir(u_int32_t rqstid,
        const char * dirpath,
        int * handle)
{
    if (init() != 0)
        return 1;

    try
    {
        *(log_.get()) << "Invoking sftp_cf_open_dir()"
                  << std::endl;


         std::stringstream ss;
         ss << "Received open_dir event, path ="
           << dirpath;

        *(log_.get()) << "Invoking sql for open_dir()"
                      << std::endl;
        
         pqxx::work pqw(*dbConn_);
         dbConn_->prepare(
         "sftp_find_dir",
         "select * from sftp.sftp_dir where dir_fqn = $1"
        );

        auto row = pqw.prepared("sftp_find_dir")(dirpath).exec();

        if (row.size() < 1)
        {
            *(log_.get()) << "ERROR: Couldn't locate directory '"
                          << dirpath
                          << "' in open_dir()"
                          << std::endl;
            return 1;
        } 

        dbConn_->prepare(
         "sftp_handle_ins_dir",
         "insert into sftp.sftp_handle (handle_type,handle_name,handle_open,dir_id) values ($1, $2, $3, $4)"
        );

        auto result = pqw.prepared("sftp_handle_ins_dir")("D")(dirpath)(1)(row[0][0].as<int>()).exec();
        pqw.commit();
        *(log_.get()) << "Successfully opened dir handle at '"
                      << dirpath
                      << "'"
                      << std::endl;

        return 0;
    }
    catch (const std::exception & e)
    {
        *(log_.get()) << "Error executing SQL: "
                      << e.what()
                      << std::endl;
        return 1;
     }
    return 0;
}

extern "C" int sftp_cf_close(u_int32_t rqstid,
        const char * handle)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_close()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received close event, handle ="
       << handle;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_read(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        u_int32_t length)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_read()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received read event,"
       << " offset = "
       << offset
       << ", length = "
       << length
       << ", handle = "
       << handle;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_read_dir(u_int32_t rqstid,
        const char * handle)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_read_dir()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received read dir event, handle ="
       << handle ;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_write(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        const char * data)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_write()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received write event,"
       << ", offset = "
       << offset
       << ", length = "
       << std::strlen(data)
       << ", handle = "
       << handle;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_remove(u_int32_t rqstid,
        const char * filename)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_remove()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received remove event, filename ="
       << filename;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_rename(u_int32_t rqstid,
        const char * oldfilename,
        const char * newfilename,
        u_int32_t flags)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_rename()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received rename event, old filename ="
       << oldfilename
       << ", new filename ="
       << newfilename;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_mkdir(u_int32_t rqstid,
        const char * path)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_mkdir()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received mkdir event, path ="
       << path;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_rmdir(u_int32_t rqstid,
        const char * path)
{
    if (init() != 0)
        return 1;

    *(log_.get()) << "Invoking sftp_cf_rmdir()"
                  << std::endl;


    std::stringstream ss;
    ss << "Received rmdir event, path ="
       << path;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_stat(u_int32_t rqstid,
        const char * path,
        u_int32_t flags)
{
    return 0;
}

extern "C" int sftp_cf_lstat(u_int32_t rqstid,
        const char * path,
        u_int32_t flags)
{
    return 0;
}

extern "C" int sftp_cf_fstat(u_int32_t rqstid,
        const char * handle,
        u_int32_t flags)
{
    return 0;
}

extern "C" int sftp_cf_setstat(u_int32_t rqstid,
        const char * path,
        u_int32_t attrs)
{
    return 0;
}

extern "C" int sftp_cf_fsetstat(u_int32_t rqstid,
        const char * handle,
        u_int32_t attrs)
{
    return 0;
}

extern "C" int sftp_cf_link(u_int32_t rqstid,
        const char * newlink,
        const char * curlink,
        int symlink)
{
    return 0;
}

extern "C" int sftp_cf_lock(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        u_int64_t length,
        int lockmask)
{
    return 0;
}

extern "C" int sftp_cf_unlock(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        u_int64_t length)
{
    return 0;
}

extern "C" int sftp_cf_realpath(u_int32_t rqstid,
        const char * origpath,
        u_int8_t ctlbyte,
        const char * path)
{
    return 0;
}

