#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "util.h"
#include "opt.h"

#define ROS_RUN_REPL "run"
#define ROS_RUN_COMPILE "output"

#ifdef _WIN32
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlChar){
  CHAR szPrintBuffer[512];
  DWORD nCharsWritten;
  if(CTRL_C_EVENT == ctrlChar){
    return TRUE;
  }
  return FALSE;
}
#endif

LVal run_commands=(LVal)NULL;
LVal run_options =(LVal)NULL;

extern char** cmd_run_sbcl(int argc,char** argv,struct sub_command* cmd);
extern char** cmd_run_ccl(int argc,char** argv,struct sub_command* cmd);
extern LVal register_runtime_options(LVal opt);

int cmd_run_star(int argc,char **argv,struct sub_command* cmd);

int cmd_run(int argc,char **argv,struct sub_command* cmd)
{
  char* current=get_opt("program");
  if(verbose>0)
    fprintf(stderr,"cmd_%s:argc=%d argv[0]=%s\n",cmd->name,argc,argv[0]);
  if(argc==1 && !current) {
    char* tmp[]={(char*)cmd->name,"--"};
    return proccmd(2,tmp,top_options,top_commands);
  }else {
    int i;
    for(i=1;i<argc;i+=proccmd(argc-i,&argv[i],run_options,run_commands));
    current=get_opt("program");
    if(strcmp((char*)cmd->name,ROS_RUN_REPL)!=0) {
      char* tmp[]={"--"};
      proccmd(1,tmp,run_options,run_commands);
    }else {
      char* tmp[]={"--",ROS_RUN_REPL};
      proccmd(1,tmp,run_options,run_commands);
      //return proccmd(1,tmp,top_options,top_commands);
    }
    if(verbose>0) {
      fprintf(stderr,"cmd_%s ends here %d\n",cmd->name,i);
    }
    return i;
  }
}

int cmd_script(int argc,char **argv,struct sub_command* cmd)
{
  char* current=get_opt("program");
  if(verbose>0)
    fprintf(stderr,"script_%s:argc=%d argv[0]=%s\n",cmd->name,argc,argv[0]);
  if(argc==1 && !current &&
     strcmp(argv[0],"--")==0) {
    char* tmp[]={"help","--"};
    if(verbose>0)
      fprintf(stderr,"current=%s\n",current);
    return proccmd(2,tmp,top_options,top_commands);
  }else {
    int i;
    char* result=q("");
    char* tmp[]={"script"};
    if(strcmp(argv[0],"--")==0)
      i=1;
    else
      i=0;
    for (;i<argc;++i) {
      char* val=escape_string(argv[i]);
      result=cat(result,"\"",val,"\"",NULL);
      s(val);
    }
    set_opt(&local_opt,"script",result,0);
    s(result);
    cmd_run_star(1,tmp,cmd);
  }
  return 0;
}

static int script_frontend_sentinel=0;

int cmd_script_frontend(int argc,char **argv,struct sub_command* cmd)
{
  FILE* in;
  char buf[800];
  int i,j,c;
  int argc_;
  char** argv_;
  char** argv_gen;
  if(script_frontend_sentinel)
    return cmd_script(argc,argv,cmd);
  script_frontend_sentinel=1;
  if(strcmp(argv[0],"--")==0)
    ++argv,--argc;
  if(verbose>0)
    fprintf(stderr,"frontend:script_%s:argc=%d argv[0]=%s\n",cmd->name,argc,argv[0]);
  if((in=fopen(argv[0],"rb"))!=NULL) {
    if(fgetc(in)!='#'||fgetc(in)!='!') {
      fclose(in);
      cmd_script(argc,argv,cmd);
    }
    for(i=0;i<3;++i)
      while((c=fgetc(in))!=EOF && c!='\n');
    i=0;
    for(;(c=fgetc(in))!=EOF;buf[i++]=c)
      if(c=='\r'||c=='\n'||i==799)
        break;
    buf[i]='\0';
    fclose(in);
  }
  if(verbose>0)
    fprintf(stderr,"ros_script_cmd=%s\n",buf);
  argv_=parse_cmdline(buf,&argc_);
  argv_gen=alloc(sizeof(char**)*(argc+argc_-2));
  for(i=0;i<argc_-2&&strcmp(argv_[i+2],"$0")!=0;++i)
    argv_gen[i]=argv_[i+2];
  for(j=i;i<j+argc;++i)
    argv_gen[i]=argv[i-j];
  j=i;
  for(i=0;i<j;i+=proccmd(j-i,&argv_gen[i],top_options,top_commands));
  return 0;
}

char* ql_path(void) {
  char* env_ql=getenv("QUICKLISP_HOME");
  if(env_ql){
    env_ql=q(env_ql);
  }else {
    env_ql=q(get_opt("quicklisp"));
  }
  env_ql=append_trail_slash(env_ql);
  return env_ql;
}

int cmd_run_star(int argc,char **argv,struct sub_command* cmd)
{
  int ret=1;
  char* impl;
  char* version=NULL;
  int pos;
  char* config=configdir();
  set_opt(&local_opt,"quicklisp",cat(config,"impls",SLASH,"ALL",SLASH,"ALL",SLASH,"quicklisp",SLASH,NULL),0);
  set_opt(&local_opt,"argv0",argv_orig[0],0);
  set_opt(&local_opt,"homedir",config,0);
  if(rc) {
    char* init=s_cat(configdir(),q("init.lisp"),NULL);
#ifdef _WIN32
    char* etc="";
#else
    char* etc="/etc/rosrc";
#endif
    char* current=get_opt("program");
    char *path,*would;
    if(file_exist_p(init)) {
      path=cat("(:load \"",init,"\")",NULL);
      would=cat(path,current?current:"",NULL);
      s(current);
      set_opt(&local_opt,"program",would,0);
      s(path);
    }
    s(init);
    current=get_opt("program");
    if(file_exist_p(etc)) {
      path=cat("(:load \"",etc,"\")",NULL);
      would=cat(path,current?current:"",NULL);
      set_opt(&local_opt,"program",would,0);
    }
  }
  if(verbose>0) {
    fprintf(stderr,"cmd_run_star:%s argc=%d argv[0]=%s \n",cmd->name,argc,argv[0]);
    fprintf(stderr,"localopt:%s\n",sexp_opts(local_opt));
  }
  impl=get_opt("lisp");
  if(impl && (pos=position_char("/",impl))!=-1) {
    version=subseq(impl,pos+1,0);
    impl=subseq(impl,0,pos);
  }else {
    if(!impl)
      impl=get_opt("default.lisp");
    if(impl) {
      char* opt=s_cat(q(impl),q("."),q("version"),NULL);
      version=get_opt(opt);
      s(opt);
    }
    if(impl) 
      impl=q(impl);
    if(version) 
      version=q(version);
  }

  if(impl) {
    char** arg=NULL;
    int i;
    set_opt(&local_opt,"impl",cat(impl,"/",version,NULL),0);
    {
      struct sub_command cmd;
      cmd.name=impl;
      cmd.short_name=version;
      if(strcmp(impl,"sbcl")==0 ||
         strcmp(impl,"sbcl-bin")==0) {
        arg=cmd_run_sbcl(argc,argv,&cmd);
      }else if(strcmp(impl,"ccl-bin")==0) {
        arg=cmd_run_ccl(argc,argv,&cmd);
      }
    }
    if(arg && file_exist_p(arg[0])) {
      char* cmd;
      char* opts=sexp_opts(local_opt);
      setenv("ROS_OPTS",opts,1);
      if(verbose>0 ||testing) {
        fprintf(stderr,"args ");
        for(i=0;arg[i]!=NULL;++i)
          fprintf(stderr,"%s ",arg[i]);
        fprintf(stderr,"\nROS_OPTS %s\n",getenv("ROS_OPTS"));
        if(testing)
          exit(EXIT_SUCCESS);
      }
      s(opts);
#ifdef _WIN32
      cmd=q(arg[0]);
      for(i=1;arg[i]!=NULL;++i) {
        cmd=s_cat(cmd,q(" "),q("\""),escape_string(arg[i]),q("\""),NULL);
      }
      SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
      exit(system(cmd));
      s(cmd);
#else
      execvp(arg[0],arg);
#endif
    }else{
      fprintf(stderr,"%s/%s is not installed.stop.\n",impl,version);
    }
  }else {
    fprintf(stderr,"lisp doesn't specified stop\n");
  }
  s(config),s(impl),s(version);
  return ret;
}

void register_cmd_run(void)
{
  char* _help;
  /*options*/
  run_options=register_runtime_options(run_options);
  run_options=add_command(run_options,"",NULL,cmd_run_star,OPT_SHOW_NONE,1,NULL,NULL);
  run_options=nreverse(run_options);
  //run_commands=add_command(run_commands,"*",NULL,cmd_run_star,OPT_SHOW_NONE,1,NULL,NULL);

  /*commands*/
  top_options=add_command(top_options,""         ,NULL,cmd_script_frontend,OPT_SHOW_NONE,1,"Run lisp environment then quit (default)",NULL);
  //  top_commands=add_command(top_commands,"output"     ,NULL,cmd_run,1,1,"Generate an executable script or binary from the software specification",NULL);
  top_commands=add_command(top_commands,ROS_RUN_REPL ,NULL,cmd_run,OPT_SHOW_HELP,1,"Run repl",NULL);
  top_commands=add_command(top_commands,"*"         ,NULL,cmd_script_frontend,OPT_SHOW_NONE,1,"Run lisp environment then quit (default)",NULL);

  _help=cat("Usage: ",argv_orig[0]," [OPTIONS] "ROS_RUN_REPL" [OPTIONS] [-- implementation-native-options...]\n\n",NULL);
  top_helps=add_help(top_helps,ROS_RUN_REPL,_help,run_commands,run_options,NULL,NULL,NULL);
  s(_help);
  _help=cat("Usage: ",argv_orig[0]," [OPTIONS] [--] script-file arguments...\n\n",
            NULL);
  top_helps=add_help(top_helps,"--",_help,run_commands,run_options,NULL,NULL,NULL);
  s(_help);
}
