#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_LIBCURL
#  include <curl/curl.h>
#endif
#ifdef HAVE_ARCHIVE_H
#  include <archive.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "opt.h"

int cmd_version(int argc,char **argv,struct sub_command* cmd)
{
  fprintf(stderr,"%s",PACKAGE_STRING);
  if(strcmp(argv[0],"--version")!=0) {
#ifdef HAVE_LIBCURL
    fprintf(stderr,"\nlibcurl %s",LIBCURL_VERSION);
#endif
#if defined(HAVE_ARCHIVE_H) && defined(_WIN32)
    fprintf(stderr,"\n%s",ARCHIVE_VERSION_STRING);
#endif
    fprintf(stderr,"\n");
    return 0;
  }
  fprintf(stderr,"\n");
  return 0;
}
