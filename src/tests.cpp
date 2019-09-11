#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
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

    int read_handle;
    rc = sftp_cf_open_file(2, "/home/mskelton/foo.txt",  600, OpenModes::READ, &read_handle);
    if (rc)
    {
        std::cerr << "Encountered error in open_file '/home/mskelton/foo.txt'; aborting" << std::endl;
        exit(1);
    }

    int data_len;
    // Read from open file handle
    u_char rdbuff[32768];
    rc = sftp_cf_read(3, std::to_string(read_handle).c_str(), read_handle, 0, sizeof(rdbuff), rdbuff, &data_len);
    if (rc)
    {
        std::cerr << "Encountered error in readi: '/home/mskelton/foo.txt'; aborting" << std::endl;
        exit(1);
    }
    std::cout << "Successfully read file data from '/home/mskelton/foo.txt': " << rdbuff << std::endl;

    int write_handle;
    rc = sftp_cf_open_file(4, "/home/mskelton/bar.txt",  600, OpenModes::CREATE | OpenModes::WRITE, &write_handle);
    if (rc)
    {
        std::cerr << "Encountered error in open_file '/home/mskelton/bar.txt'; aborting" << std::endl;
        exit(1);
    }

    // Write to open file handle
    u_char wrtbuff[4] = {0x37, 0x38, 0x39, 0x40};
    rc = sftp_cf_write(5, std::to_string(write_handle).c_str(), write_handle, 0, sizeof(wrtbuff), wrtbuff, &data_len);
    if (rc)
    {
        std::cerr << "Encountered error in write: '/home/mskelton/bar.txt'; aborting" << std::endl;
        exit(1);
    }
    std::cout << "Successfully wrote file data to '/home/mskelton/bar.txt': " << wrtbuff << std::endl;

    rc = sftp_cf_close(6, read_handle);
    rc = sftp_cf_close(7, write_handle);
    rc = sftp_cf_close(8, home_dir_handle);
}
