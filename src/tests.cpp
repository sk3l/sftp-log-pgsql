#include <stdlib.h>
#include <stdio.h>

//#include "config.h"
//#include "sshbuf.h"

#include "sftp-pgsql-plugin-store.h" 

//:#define SSHDIR "/etc/ssh"

int main(int argc, char ** argv)
{
    int home_dir_handle;
    int rc = sftp_cf_open_dir(1, "/home/mskelton",  &home_dir_handle);

    // do some more work
    //

    rc = sftp_cf_close(10, home_dir_handle);
}
