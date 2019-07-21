#include <sys/types.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <pqxx/pqxx>
#include <pqxx/except>

#include "sftp-callback.h"
#include "sftp-plugin.h"

static std::unique_ptr<pqxx::connection> dbConn_;
static std::string connStr_ = "postgres://sftpadmin:abc123@127.0.0.1/postgres";
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
         "insert into public.sftp_logs (log_event) values ($1)"
        );

        pqxx::work pqw(*dbConn_);
        auto result = pqw.prepared("sftp_pgin_ins")(data).exec();
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
        u_int32_t attrs,
        enum PLUGIN_SEQUENCE seq)
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
       << flags
       << ", attrs = "
       << attrs;

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_open_dir(u_int32_t rqstid,
        const char * dirpath)
//        enum PLUGIN_SEQUENCE seq)
{
    if (init() != 0)
        return 1;
    
    *(log_.get()) << "Invoking sftp_cf_open_dir()" 
                  << std::endl;

   
    std::stringstream ss;
    ss << "Received open_dir event, path ="
       << dirpath; 

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_close(u_int32_t rqstid,
        const char * handle,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_read(u_int32_t rqstid,
        const char * handel,
        u_int64_t offset,
        u_int32_t length,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_read_dir(u_int32_t rqstid,
        const char * handle,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_write(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        const char * data,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_remove(u_int32_t rqstid,
        const char * filename,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_rename(u_int32_t rqstid,
        const char * oldfilename,
        const char * newfilename,
        u_int32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_mkdir(u_int32_t rqstid,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_rmdir(u_int32_t rqstid,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_stat(u_int32_t rqstid,
        const char * path,
        u_int32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_lstat(u_int32_t rqstid,
        const char * path,
        u_int32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_fstat(u_int32_t rqstid,
        const char * handle,
        u_int32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_setstat(u_int32_t rqstid,
        const char * path,
        u_int32_t attrs,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_fsetstat(u_int32_t rqstid,
        const char * handle,
        u_int32_t attrs,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_link(u_int32_t rqstid,
        const char * newlink,
        const char * curlink,
        int symlink,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_lock(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        u_int64_t length,
        int lockmask,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_unlock(u_int32_t rqstid,
        const char * handle,
        u_int64_t offset,
        u_int64_t length,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_realpath(u_int32_t rqstid,
        const char * origpath,
        u_int8_t ctlbyte,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

