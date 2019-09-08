
set client_min_messages to warning;

-------------------------------------------------------------------------------
-- Create sftp tables
drop table if exists sftp.sftp_handle;
drop table if exists sftp.sftp_file;
drop table if exists sftp.sftp_dir;
drop table if exists sftp.sftp_attrs;
-------------------------------------------------------------------------------
drop schema if exists sftp;
create schema sftp;

create table sftp.sftp_attrs (
   attr_id          serial not null primary key,
   attr_flags       int not null,
   attr_uid         int not null,
   attr_gid         int not null, 
   attr_atime       timestamp  null,
   attr_mtime       timestamp null
);

create table sftp.sftp_dir (
   dir_id          serial not null primary key,
   dir_name         text not null,
   dir_fqn          text not null,
   dir_type         char(1)  null,
   attr_id          serial   not null references sftp.sftp_attrs,
   -- parent dir
   parent_dir_id  int null         
);

create table sftp.sftp_file (
   file_id          serial not null primary key,
   file_name        text not null,
   file_data        bytea null,
   file_type        char(1)  null,
   attr_id          serial   not null references sftp.sftp_attrs,
   -- parent dir
   dir_id          serial not null references sftp.sftp_dir 
);

create table sftp.sftp_handle (
   handle_id serial not null primary key,
   handle_type        char(1) not null, -- 'F' or 'D'
   handle_name text not null,
   handle_open boolean null,
   handle_flags int null,
   handle_bytes_read int null,
   handle_bytes_written int null,
   dir_id        int null, 
   file_id        int null
);

