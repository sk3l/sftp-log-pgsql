
set client_min_messages to warning;

-------------------------------------------------------------------------------
-- Grant usage-based privileges to skelonl agent
--grant usage on schema date_time to skelonl_agent;

--grant usage on schema blog to skelonl_agent;

--alter default privileges in schema blog grant select, insert, update, delete on tables 
--to skelonl_agent;

--alter default privileges in schema blog grant execute on functions 
--to skelonl_agent;

-------------------------------------------------------------------------------
-- Create sftp tables

drop table if exists sftp.sftp_file;
drop table if exists sftp.sftp_dir;
drop table if exists sftp.sftp_attrs;
-------------------------------------------------------------------------------
-- Create blog schema
--select util.drop_schema('sftp');
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
   dir_handle       text not null, 
   dir_type         char[1]  null,
   attr_id          serial   not null references sftp.sftp_attrs
);

create table sftp.sftp_file (
   file_id          serial not null primary key,
   file_name        text not null,
   file_data          text not null,
   file_handle       text not null, 
   file_type         char[1]  null,
   attr_id          serial   not null references sftp.sftp_attrs,
   dir_id          serial   not null references sftp.sftp_dir
);
/*
create table blog.category (
   category_id       serial       not null primary key,
   category_name     text         not null
);

create table blog.entry_categories (
   entry_id          serial      not null references blog.entry,
   category_id       serial      not null references blog.category,

   primary key(entry_id, category_id)
);

create table blog.tags (
   tag_id            serial      not null primary key,
   tag_name          text        not null
);

create table blog.entry_tags (
   entry_id          serial      not null references blog.entry,
   tag_id            serial      not null references blog.tags,

   primary key(entry_id,tag_id) 
);
*/
