#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//#include "config.h"
//#include "sshbuf.h"

#include "sftp-common.h"
#include "sftp-pgsql-plugin-store.h"

//:#define SSHDIR "/etc/ssh"

int main(int argc, char ** argv)
{
    int home_dir_handle;
    int rc = sftp_cf_open_dir(1, "/home/mskelton",  &home_dir_handle);
    if (rc)
    {
        std::cerr << "Encountered error in open_dir '/home/mskelton'; aborting" << std::endl;
        exit(1);
    }
    // do some more work
    //
    int file_handle;
    rc = sftp_cf_open_file(2, "/home/mskelton/foo.txt",  600, OpenModes::WRITE|OpenModes::CREATE, &file_handle);
    if (rc)
    {
        std::cerr << "Encountered error in open_file '/home/mskelton/foo.txt'; aborting" << std::endl;
        exit(1);
    }

    rc = sftp_cf_close(3, file_handle);
    rc = sftp_cf_close(4, home_dir_handle);
}
