#include <fstream>
#include <memory>
#include <sstream>
#include <pqxx/pqxx>
#include <pqxx/except>

#include "sftp-callback.h"
#include "sftp-plugin.h"

static std::unique_ptr<pqxx::connection> dbConn_;
static std::string connStr_ = "postgres://sftpngdb:abc123@127.0.0.1/sftpauthdb";
using sqlconn_t = pqxx::connection; 

static std::unique_ptr<std::ofstream> log_;

static void init()
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
    }
    catch (const pqxx::failure & e)
    {
        *(log_.get()) << "Error initializing PgSQL conn: "
                      << e.what()
                      << std::endl;   
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
         "insert into public.sftp_log (log_event) values ($1)"
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

extern "C" int sftp_cf_open_file(uint32_t rqstid,
        const char * filename,
        uint32_t access,
        uint32_t flags,
        uint32_t attrs,
        enum PLUGIN_SEQUENCE seq)
{
    init();

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

extern "C" int sftp_cf_open_dir(uint32_t rqstid,
        const char * dirpath,
        enum PLUGIN_SEQUENCE seq)
{
    init();
     
    *(log_.get()) << "Invoking sftp_cf_open_dir()" 
                  << std::endl;

   
    std::stringstream ss;
    ss << "Received open_dir event, path ="
       << dirpath; 

    do_sql(ss.str());

    return 0;
}

extern "C" int sftp_cf_close(uint32_t rqstid,
        const char * handle,
        enum PLUGIN_SEQUENCE seq)
{
    return 1;
}

extern "C" int sftp_cf_read(uint32_t rqstid,
        const char * handel,
        uint64_t offset,
        uint32_t length,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_read_dir(uint32_t rqstid,
        const char * handle,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_write(uint32_t rqstid,
        const char * handle,
        uint64_t offset,
        const char * data,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_remove(uint32_t rqstid,
        const char * filename,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_rename(uint32_t rqstid,
        const char * oldfilename,
        const char * newfilename,
        uint32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_mkdir(uint32_t rqstid,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_rmdir(uint32_t rqstid,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_stat(uint32_t rqstid,
        const char * path,
        uint32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_lstat(uint32_t rqstid,
        const char * path,
        uint32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_fstat(uint32_t rqstid,
        const char * handle,
        uint32_t flags,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_setstat(uint32_t rqstid,
        const char * path,
        uint32_t attrs,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_fsetstat(uint32_t rqstid,
        const char * handle,
        uint32_t attrs,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_link(uint32_t rqstid,
        const char * newlink,
        const char * curlink,
        int symlink,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_lock(uint32_t rqstid,
        const char * handle,
        uint64_t offset,
        uint64_t length,
        int lockmask,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_unlock(uint32_t rqstid,
        const char * handle,
        uint64_t offset,
        uint64_t length,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

extern "C" int sftp_cf_realpath(uint32_t rqstid,
        const char * origpath,
        uint8_t ctlbyte,
        const char * path,
        enum PLUGIN_SEQUENCE seq)
{
    return 0;
}

