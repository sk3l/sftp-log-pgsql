#include <stdlib.h>
#include <stdio.h>

//#include "config.h"
//#include "sshbuf.h"

#include "sftp-pgsql-plugin-store.h" 

//:#define SSHDIR "/etc/ssh"

int main(int argc, char ** argv)
{
    int handle;
    int rc = sftp_cf_open_dir(1, "/home/mskelton",  &handle);
    /*
    int failures = 0;

   struct sshbuf * fbuf;
   if ((fbuf = sshbuf_new()) == NULL)
		printf("%s: sshbuf_new failed", __func__);

   plugins_cnt = load_plugin_conf("sftp_plugin_conf_test", fbuf);

   init_plugins(fbuf);

   if(load_plugins_so() != 0)
   {
       printf("%s: load_plugin_so failed", __func__);
       return 1;
   }
   sshbuf_free(fbuf);

   {
      struct callback_stats cbkstats;
      if (call_open_dir_plugins(1, "/test/path", PLUGIN_SEQ_BEFORE, &cbkstats) < 0)
         ++failures;
   }
   sftp_plugins_release();

   if (!failures)
   {
      printf("All tests passed\n");
      return 0;
   }

   printf("Some tests failed (as above)\n");
   return failures;
   */
}
