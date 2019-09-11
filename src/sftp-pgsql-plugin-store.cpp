#include <string.h>
#include <sys/types.h>

#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <memory>
#include <sstream>
#include <pqxx/pqxx>
#include <pqxx/except>
#include <pqxx/binarystring>
#include <pqxx/transaction>
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

          dbConn_->prepare(
            "sftp_attrs_ins",
            "insert into sftp.sftp_attrs\
            (attr_id,attr_flags,attr_uid,attr_gid,attr_atime,attr_mtime)\
            values\
            (DEFAULT,$1, $2, $3, $4, $5)\
            RETURNING attr_id;"
          );

          dbConn_->prepare(
            "sftp_file_qry",
            "select file_id,file_name,file_type,attr_id,dir_id from sftp.sftp_file where dir_id = $1 and file_name = $2"
          );

          dbConn_->prepare(
             "sftp_file_ins",
             "insert into sftp.sftp_file\
             (file_id,file_name, file_type,attr_id,dir_id)\
             values\
             (DEFAULT,$1, $2, $3, $4)\
             RETURNING file_id"
           );

          dbConn_->prepare(
             "sftp_file_read",
             "select substring(f.file_data from $1 for $2) as chunk from sftp.sftp_file f join sftp.sftp_handle h\
             on f.file_id = h.file_id\
             where h.handle_open = 't' and  h.handle_id = $3"
             );

          dbConn_->prepare(
             "sftp_file_write",
             "update sftp.sftp_file as f\
             set file_data =\
                case when file_data is NULL then $1\
                else overlay(file_data placing $1 from $2 for $3) end\
             from sftp.sftp_handle h\
             where f.file_id = h.file_id and h.handle_open = 't' and h.handle_id = $4"
           );

         dbConn_->prepare(
            "sftp_dir_qry",
            "select dir_id,dir_name,dir_fqn,dir_type,attr_id,parent_dir_id from sftp.sftp_dir where dir_fqn = $1"
          );

          dbConn_->prepare(
            "sftp_handle_ins",
            "insert into sftp.sftp_handle\
            (handle_id, handle_type,handle_name,handle_open,dir_id,file_id)\
             values (DEFAULT, $1, $2, $3, $4, $5)\
            RETURNING handle_id;"
          );

          dbConn_->prepare(
            "sftp_handle_udt_close",
            "update sftp.sftp_handle\
             set handle_open = false\
             where\
             handle_id = $1 and handle_open = true"
          );


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
/*
static int insert_sftp_attrs(
        pqxx::work & pqw,
        int flags,
        int uid,
        int gid,
        std::time_t * atime,
        std::time_t * mtime)
{

}
*/

static std::string date_time_now()
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream tstmp;
    tstmp << std::put_time(&tm, "%FT%T");

    return tstmp.str();
}

static void split_file_path(const std::string & path, std::string & dir, std::string & file)
{
    auto pos = path.rfind("/");

    if (pos == std::string::npos)
    {
       dir.assign(".");
    }
    else
    {
        dir = path.substr(0, pos);
        file = path.substr(pos+1);
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
    try
    {
        if (init() != 0)
           return 1;

        *(log_.get()) << "Invoking sftp_cf_open_file()"
                  << std::endl;

        *(log_.get()) << "Invoking sql for open_file()"
                      << std::endl;

         pqxx::work pqw(*dbConn_);

         std::string dir, file;
         split_file_path(filename, dir, file);

         // TO DO canonicalize
        auto dir_row = pqw.prepared("sftp_dir_qry")(dir).exec();
        if (dir_row.size() < 1)
        {
            *(log_.get()) << "ERROR: no directory with name '"
                          << dir
                          << "' found in open_file(); ."
                          << std::endl;
            return 1;
        }
        auto dir_id = dir_row[0][0].as<int>();

        auto file_row = pqw.prepared("sftp_file_qry")(dir_id)(file).exec();

        int file_id;
        if (file_row.size() < 1)
        {
            *(log_.get()) << "No file with name '"
                          << filename
                          << "' found in open_file()."
                          << std::endl;

            if (!(flags & OpenModes::CREATE))
            {
                *(log_.get()) << "Open Mode CREATE not present; aborting '"
                              << std::endl;

                return 1;
            }

            auto now = date_time_now();
            auto attr_row = pqw.prepared("sftp_attrs_ins")(
                            flags)(
                            1000)(
                            1000)(
                            now)(
                            now).exec();

           if (attr_row.affected_rows() < 1)
           {
                *(log_.get()) << "ERROR: Couldn't create file attrs in open_file()"
                              << std::endl;
                return 1;

           }
           int attr_id = attr_row[0][0].as<int>();
           file_row = pqw.prepared("sftp_file_ins")(
                   file)(
                   "F")(
                   attr_id)(
                   dir_id).exec();

           if (file_row.affected_rows() < 1)
           {
                *(log_.get()) << "ERROR: Couldn't create file in open_file()"
                              << std::endl;
                return 1;

           }
        }
        file_id = file_row[0][0].as<int>();

        auto result = pqw.prepared("sftp_handle_ins")("F")(file)(1)(0)(file_id).exec();
        *handle = result[0][0].as<int>();

        pqw.commit();

        *(log_.get()) << "Successfully opened file handle at '"
                      << filename
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

extern "C" int sftp_cf_open_dir(u_int32_t rqstid,
        const char * dirpath,
        int * handle)
{
    try
    {
        if (init() != 0)
           return 1;

        *(log_.get()) << "Invoking sftp_cf_open_dir()"
                  << std::endl;


         std::stringstream ss;
         ss << "Received open_dir event, path ="
           << dirpath;

        *(log_.get()) << "Invoking sql for open_dir()"
                      << std::endl;

         pqxx::work pqw(*dbConn_);

        auto row = pqw.prepared("sftp_dir_qry")(dirpath).exec();

        if (row.size() < 1)
        {
            *(log_.get()) << "ERROR: Couldn't locate directory '"
                          << dirpath
                          << "' in open_dir()"
                          << std::endl;
            return 1;
        }

        auto result = pqw.prepared("sftp_handle_ins")("D")(dirpath)(1)(row[0][0].as<int>())(0).exec();

        *handle = result[0][0].as<int>();

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
                             int handle)
{
    try
    {
       if (init() != 0)
           return 1;

       *(log_.get()) << "Invoking sftp_cf_close()"
                     << std::endl;

       std::stringstream ss;
       ss << "Received close event, handle ="
          << handle;

       pqxx::work pqw(*dbConn_);
       auto result = pqw.prepared("sftp_handle_udt_close")(handle).exec();

       if (result.affected_rows() < 1)
       {
            *(log_.get()) << "ERROR: Couldn't locate an open handle id='"
                          << handle
                          << "' in close()"
                          << std::endl;

           return 1;
       }

       pqw.commit();
       return 0;
    }
    catch (const std::exception & e)
    {
        *(log_.get()) << "Error executing SQL: "
                      << e.what()
                      << std::endl;

       return 1;
    }
}

extern "C" int sftp_cf_read(u_int32_t rqstid,
        const char * handle_name,
        int handle,
        u_int64_t offset,
        u_int32_t length,
        u_char * data,
        int * len)
{
    try
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

       // ***********
       // TO DO : check file handle permissions to ensure READable
 
       pqxx::work pqw(*dbConn_);
       auto result = pqw.prepared("sftp_file_read")((offset == 0?1:offset))(length)(handle).exec();

       if (result.affected_rows() < 1)
       {
               *(log_.get()) << "ERROR: Couldn't locate an open handle id='"
                             << handle
                             << "' in read()"
                             << std::endl;

              return 1;
       }

       pqxx::binarystring bstr(result[0][0]);

       pqw.commit();

       std::memcpy(data, bstr.data(), length);
       *len = bstr.size();

        return 0;
    }
    catch (const std::exception & e)
    {
        *(log_.get()) << "Error executing SQL: "
                      << e.what()
                      << std::endl;
        return 1;

    }
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
        const char * hstr,
        int handle,
        u_int64_t offset,
        u_int32_t length,
        u_char * data,
        int * len)
{
    try
    {
       if (init() != 0)
           return 1;

       *(log_.get()) << "Invoking sftp_cf_write()"
                     << std::endl;

       std::stringstream ss;
       ss << "Received write event,"
          << " offset = "
          << offset
          << ", length = "
          << length
          << ", handle = "
          << handle;

       // ***********
       // TO DO : check file handle permissions to ensure WRITEable

       pqxx::work pqw(*dbConn_);
       auto result = pqw.prepared("sftp_file_write")(pqw.esc_raw(data, length))(offset == 0?1:offset)(length)(handle).exec();

       if (result.affected_rows() < 1)
       {
               *(log_.get()) << "ERROR: Couldn't locate an open handle id='"
                             << handle
                             << "' in write()"
                             << std::endl;

              return 1;
       }


       pqw.commit();


       // ***********
       // TO DO : establish true byte count written 
 
       *len = length;

        return 0;
    }
    catch (const std::exception & e)
    {
        *(log_.get()) << "Error executing SQL: "
                      << e.what()
                      << std::endl;
        return 1;

    }

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

