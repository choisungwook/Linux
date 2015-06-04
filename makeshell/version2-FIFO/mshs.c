#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <utime.h>
#include <time.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>

#define SZ_STR_BUF		256
#define SZ_FILE_BUF 1024
#define err_return() \
do { \
  perror(argv[0]); \
  return; \
}while(0)

//error macro function
//인자가 부족하거나 많으면 longjmp 실행
#define chk_arg_opt(usgp, opt, isopt, cnt) \
  chk_argument(argc, argv, usgp, opt, isopt, cnt)
#define chk_arg01_opt(usgp, opt, isopt) \
  chk_arg_opt(usgp, opt, isopt, -1)
#define chk_arg0_opt(usgp, opt, isopt) \
  chk_arg_opt(usgp, opt, isopt, 0)
#define chk_arg2_opt(usgp, opt, isopt) \
  chk_arg_opt(usgp, opt, isopt, 2)

#define chk_arg(usgp,cnt)  chk_argument(argc, argv, usgp, NULL, NULL, cnt)
#define chk_arg01(usgp)	chk_arg(usgp, -1)
#define chk_arg0(usgp)	chk_arg(usgp, 0)
#define chk_arg1(usgp)	chk_arg(usgp, 1)
#define chk_arg2(usgp)	chk_arg(usgp, 2)


char cwd[SZ_STR_BUF];


jmp_buf jmpbuf;

//functionchk
void help(int argc, char *argv[]);
void get_max_name_len(DIR *dp, int *mnl,int *npl);
void date(int argc, char *argv[]);

static char**
get_argv(char *cmd_line, int *argcp)
{
  #define ARGV_SIZE 5

  static char **argv = NULL;
  static int max_argc = ARGV_SIZE;
  static int size = ARGV_SIZE * sizeof(char*);

  char *t;
  int argc;

  if( (argv == NULL) && ((argv = malloc(size)) == NULL))
    goto err;

  if((t = strtok(cmd_line, " \t\n")) == NULL)
    goto jump;

  argv[0] = t;

  for(argc = 1; (argv[argc] = strtok(NULL, " \t\n")); ++argc)
  {
    //if argument more than ARGV_SIZE,
    //you must realloc argv
    if (argc == (max_argc-1))
    {
      max_argc += ARGV_SIZE;
      size = max_argc * sizeof(char*);
      if( (argv = realloc(argv,size)) == NULL)
        goto err;
    }
  }

  *argcp = argc;
  return(argv);

err:
  printf("get_argv() : malloc() or realloc() error\n");
jump:
  longjmp(jmpbuf,2);
}


//function that check argument
static int chk_argument(int argc, char *argv[], char *usgp, char *opt,
                         int *isopt, int cnt)
{
  int isless = 0;

  if(opt)
    *isopt = 0;

  //check '-'
  if( (--argc > 0) && (argv[1][0] == '-'))
  {
	  if(opt && !(strcmp(opt,argv[1])))
		  *isopt = 1, --argc;
	  else{
		  printf("wrong options\n");
		  goto usage;
	  }
  }

  if(cnt < 0)
	  isless = 1, cnt = -cnt;
  if(argc > cnt)
	  printf("Too many arguments\n");
  else if(!isless && (argc < cnt))
	  printf("Insufficient arguments\n");
  else
	  return(argc);

usage:
 	 printf("usage : %s %s\n",argv[0],usgp);
 	 longjmp(jmpbuf,1);
}


void run_cmd(int argc,char *argv[])
{
  pid_t pid;

  //fail
  if( (pid = fork()) < 0)
    perror("rum_cmd");

   //child process
   else if(pid == 0)
   {
    if(execvp(argv[0],argv) < 0)
    {
      printf("%s : Command not found\n",argv[0]);
      exit(1);
    }
   }
   //parent process
   else
   {
    if(waitpid(pid,NULL,0) < 0)
      perror("run_cmd");
   }
}

void
Alarm(int argc, char *argv[])
{
	chk_arg1("time");
	date(1,NULL);
	alarm(atoi(argv[1]));
}

void
cat(int argc,char *argv[])
{
	int fd,len;
	char buf[SZ_FILE_BUF];

	chk_arg1("file_name");

	if( (fd = open(argv[1],O_RDONLY)) < 0)
		err_return();

	while	( (len = read(fd,buf,SZ_FILE_BUF)) > 0){
		if( write(STDOUT_FILENO,buf,len) != len){
			len = -1;
			break;
		}
	}

	close(fd);

	if(len >= 0)
		err_return();
}

void
cd(int argc, char *argv[])
{
	int cnt = chk_arg01("[dir_name]");

	//cd directory
	if (cnt == 1)
	{
		if( chdir(argv[1]) < 0)
			err_return();
	}
	//cd homeDirectory
	else
	{
		struct passwd *pwp = getpwuid(getuid());
		if( (pwp == NULL) || (chdir(pwp->pw_dir) < 0))
			err_return();
	}

	//get current path
	getcwd(cwd,SZ_STR_BUF);
}

void
changemod(int argc, char *argv[])
{
	int mode = 0;

	chk_arg2("mode name");
	sscanf(argv[1], "%o",&mode);

	if (chmod(argv[2], mode) < 0)
		err_return();
}


void
cp(int argc, char *argv[])
{
	int rfd, wfd, len;
	char buf[SZ_FILE_BUF];
	struct stat st_buf;

	chk_arg2("oldname newname");

	if( (stat(argv[1], &st_buf) < 0) ||
			((rfd = open(argv[1],O_RDONLY)) < 0))
		err_return();
	if( (wfd = creat(argv[2],st_buf.st_mode)) < 0)
	{
		perror(argv[0]);
		close(rfd);
		return;
	}

	do
	{
		if( ((len = read(rfd,buf,SZ_FILE_BUF)) < 0) ||
				( (len > 0) && (write(wfd,buf,len)) < 0 ) )
		{
			perror(argv[0]);
			break;
		}
	}while(len > 0);

	close(rfd);
	close(wfd);

}

void
date(int argc, char *argv[])
{
  time_t ctm;
  struct tm* ltm;
  char *atm, stm[128];

  chk_arg0("date is only command");

  ctm = time(NULL);
  ltm = localtime(&ctm);

  atm = asctime(ltm);
  printf("atm : %s",atm);

  atm = ctime(&ctm);
  printf("ctime : %s",atm);

  strftime(stm,sizeof(stm),"%a %b %d %X %Z %Y",ltm);
  printf("stm : %s\n",stm);
}

void
id(int argc, char *argv[])
{
  struct passwd *pwp;
  struct group *grp;
  int idFlag = 0;

  idFlag = chk_arg01("[username]");

  idFlag ? (pwp = getpwnam(argv[1])) : (pwp = getpwuid(getuid()));

  if(pwp == NULL){
      printf("wrong user\n");
      return;
  }

  if( (grp = getgrgid(pwp->pw_gid)) < 0)
  {
    printf("wrong group\n");
    return;
  }


  printf("uid=%ld(%s) gid=%ld(%s)\n",pwp->pw_uid,pwp->pw_name,
    grp->gr_gid,grp->gr_name);

}

void
hostname(int argc, char *argv[])
{
	chk_arg0("hostname is only command");
	char hostname[SZ_STR_BUF] = {0,};
	gethostname(hostname,SZ_STR_BUF);
	printf("%s\n",hostname);
}

void
echo(int argc, char *argv[])
{
	int k;

	for(k=1; k<argc; ++k)
	{
		printf("%s ",argv[k]);
	}
	printf("\n");
}

void
print_file_attributes(char *path,char *fn,char *argv)
{
	struct stat st_buf;
	char full_path[SZ_STR_BUF], buf[SZ_STR_BUF], c;
	struct passwd *pwp;
	struct group *grp;
	char time_buf[13];
	struct tm *tmp;

	sprintf(full_path,"%s/%s",path,fn);
	if(lstat(full_path,&st_buf) < 0){
		perror(argv);
		return;
	}
	else if(S_ISDIR(st_buf.st_mode))
		c = 'd';
	else if(S_ISCHR(st_buf.st_mode))
		c = 'c';
	else if(S_ISBLK(st_buf.st_mode))
		c = 'b';
	else if(S_ISFIFO(st_buf.st_mode))
		c = 'f';
	else if(S_ISLNK(st_buf.st_mode))
		c = 'l';
	else if(S_ISSOCK(st_buf.st_mode))
		c = 's';


	buf[0] = c;
	buf[1] = (st_buf.st_mode & S_IRUSR) ? 'r' : '-';
	buf[2] = (st_buf.st_mode & S_IWUSR) ? 'w' : '-';
	buf[3] = (st_buf.st_mode & S_IXUSR) ? 'x' : '-';
	buf[4] = (st_buf.st_mode & S_IRGRP) ? 'r' : '-';
	buf[5] = (st_buf.st_mode & S_IWGRP) ? 'w' : '-';
	buf[6] = (st_buf.st_mode & S_IXGRP) ? 'x' : '-';
	buf[7] = (st_buf.st_mode & S_IROTH) ? 'r' : '-';
	buf[8] = (st_buf.st_mode & S_IWOTH) ? 'w' : '-';
	buf[9] = (st_buf.st_mode & S_IXOTH) ? 'x' : '-';
	buf[10] = '\0';

	pwp = getpwuid(st_buf.st_uid);
	grp = getgrgid(st_buf.st_gid);
	tmp = localtime(&st_buf.st_mtime);
	strftime(time_buf,13,"%b %d %H:%M",tmp);
	sprintf(buf+10, " %3ld %-8s %-8s %8ld %s %s",
			st_buf.st_nlink, pwp->pw_name, grp->gr_name,
			st_buf.st_size, time_buf, fn);

	printf("%s\n",buf);
}

void
ls(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *dirp;
	int max_name_len, num_per_line, argcnt, cnt = 0;
	int list;
	char *path;

	argcnt = chk_arg01_opt("[-l] [dir_name]", "-l", &list);
	// if path is "ls", argcnt is = "."
	// if path is "ls /etc", argcnt is argv[1](/etc)
	// if path is "ls -l /etc", argcnt is argv[2](/etc)

	path = argcnt ? (list ? argv[2] :argv[1]) : ".";

	if((dp = opendir(path)) == NULL)
		err_return();
	if(!list)
		get_max_name_len(dp,&max_name_len,&num_per_line);

	while((dirp = readdir(dp)) != NULL){
		if(list)
			print_file_attributes(path,dirp->d_name,argv[0]);
		else{
			printf("%-*s",max_name_len,dirp->d_name);
			if ((++cnt % num_per_line) == 0)
				printf("\n");
		}
	}

	if(!list && ((cnt % num_per_line) != 0))
		printf("\n");

	closedir(dp);
}


void
get_max_name_len(DIR *dp, int *mnl,int *npl)
{
	struct dirent *dirp;
	*mnl = 0;

	while((dirp = readdir(dp)) != NULL){
		int name_len = strlen(dirp->d_name);
		if(name_len > *mnl)
			*mnl = name_len;
	}
	rewinddir(dp);
	*mnl += 4;
	*npl = 80;
}

void
makedir(int argc, char *argv[])
{
	chk_arg1("dir_name");
	if (mkdir(argv[1], 0755) < 0)
		err_return();
}

void
mv(int argc, char *argv[])
{
	chk_arg2("oldname newname");

	if (link(argv[1], argv[2]) < 0)
		err_return();
	else if (unlink(argv[1]) < 0)
		err_return();
}


void
ln(int argc, char *argv[])
{
	int is_symlink, ret;

	chk_arg2_opt("[-s] oldname newname","-s",&is_symlink);
	ret = is_symlink ? symlink(argv[2], argv[3]) : link(argv[1], argv[2]);
	if (ret < 0)
		err_return();
}

void
Sleep(int argc, char *argv[])
{
	chk_arg1("time");
	date(1,NULL);
	sleep(atoi(argv[1]));
	date(1,NULL);
	alarm(atoi(argv[1]));
	pause(); // SIGALAM
}

void
touch(int argc, char *argv[1])
{
	int creatDescriptor = 0;

	chk_arg1("filename");

	if(utime(argv[1],NULL) < 0){
		if(errno == ENOENT){
			creatDescriptor = creat(argv[1],0644);
			close(creatDescriptor);
		}
		else
			err_return();
	}
}

void
unixname(int argc, char *argv[])
{
	struct utsname un;
	int isopt = 0;

	chk_arg0_opt("[-a]","-a",&isopt);

	uname(&un);
	printf("%s ", un.sysname);

	if (isopt)
		printf("%s %s %s %s", un.nodename, un.release,
		                      un.version,  un.machine);

	printf("\n");
	;
}

void
pwd(int argc, char *argv[])
{
	chk_arg0("pwd only command");
	printf("%s\n",cwd);
}

void
removedir(int argc, char *argv[])
{
	chk_arg1("dir_name");

	if(rmdir(argv[1]) < 0)
		err_return();
}

void
rm(int argc, char *argv[])
{
	struct stat buf;
	int ret;

	chk_arg1("name");


	if ((ret = lstat(argv[1], &buf)) < 0)
		err_return();

	if (S_ISDIR(buf.st_mode))
		ret = rmdir(argv[1]);
	else
		ret = unlink(argv[1]);

	if(ret < 0)
		err_return();
}

void
quit()
{
	exit(0);
}

void
whoami(int argc, char* argv[])
{
  struct passwd *pwp = NULL;

  chk_arg0("whoami is only command");
  pwp = getpwuid(getuid());
  printf("%s\n",pwp->pw_name);
}


typedef struct {
	char *cmd;
	void (*func)(int, char*[]);
} cmd_disp_t;

cmd_disp_t cmd_disp[] = {
	{ "cp",			cp },
	{ "echo",		echo },
	{ "help",		help },
	{ "pwd",		pwd },
	{ "exit",		quit },
	{ "rm",			rm },
	{ "hostname",	hostname },
	{ "whoami",		whoami },
	{ "ls",	 		ls},
	{ "cd",	 		cd},
	{ "uname",	 	unixname},
	{ "cat",	 	cat},
	{ "mkdir",	 	makedir},
	{ "rmdir",	 	removedir},
	{ "mv",	 		mv},
	{ "chmod",	 	changemod},
	{ "ln",	 		ln},
	{ "touch",	 	touch},
	{ "date",	 	date},
	{ "id",	 		id},
	{ "alarm",	 	Alarm},
	{ "sleep",	 	Sleep},
};

int num_cmd = sizeof(cmd_disp) / sizeof(cmd_disp[0]);


void
help(int argc, char *argv[])
{
	int  k;
	chk_arg0("help only command");

	for (k = 0; k < num_cmd; ++k)
		printf("%s ", cmd_disp[k].cmd);
	printf("\n");
}



void
proc_cmd(char *cmd_line)
{
	int  k;
	int argc;
	char **argv;

	argv = get_argv(cmd_line,&argc);

	for (k = 0; k < num_cmd; ++k)
	{
		if (!strcmp(argv[0], cmd_disp[k].cmd)) {
			cmd_disp[k].func(argc,argv);
			return;
		}
  }

  run_cmd(argc,argv);
}


void (*signal(int signo,void (*func)(int)))(int)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if( (signo != SIGALRM) && (signo != SIGINT) )
    act.sa_flags |= SA_RESTART;
  if(sigaction(signo,&act,&oact) < 0)
    return(SIG_ERR);

  return(oact.sa_handler);
}


static void sig_CtrlC(int sig)
{
  printf("sig_CtrlC : SIGINT cautght\n");
}


void err_exit(char *msg){
	perror(msg);
	exit(1);
}

void connect_to_client(void){
	char *fifo_ctos = "fifo_ctos";
	char *fifo_stoc = "fifo_stoc";
	int ifd, ofd;

	if((mkfifo(fifo_ctos, 0666) < 0) && (errno != EEXIST))
		err_exit(fifo_ctos);
	if((mkfifo(fifo_stoc, 0666) < 0) && (errno != EEXIST))
		err_exit(fifo_stoc);
	printf("Waiting for client's connection...\n");
	if((ifd=open(fifo_ctos, O_RDONLY)) <0)
		err_exit(fifo_ctos);
	if((ofd = open(fifo_stoc, O_WRONLY)) < 0)
		err_exit(fifo_stoc);

	printf("Connected to client.\n");
	dup2(ifd, 0);
	dup2(ofd, 1);
	dup2(ofd, 2);
	printf("mshs: SERVER is READY,\n");

	close(ifd);
	close(ofd);
}

int
main(int argc, char *argv[])
{
  char cmd_line[SZ_STR_BUF];
  int cmd_cnt = 1;

  setbuf(stdout,NULL);
  setbuf(stderr,NULL);

  signal(SIGINT,sig_CtrlC);

 

  connect_to_client();

  getcwd(cwd,SZ_STR_BUF);

  if (setjmp(jmpbuf) == 1)
    cmd_cnt++;

	while(1) {
		printf("<%s> %d >> ",cwd,cmd_cnt);

		if( gets(cmd_line) != NULL)
		{
			proc_cmd(cmd_line);
			++cmd_cnt;
		}
		else if( feof(stdin))
			return 1;
		else if (errno == EINTR)
			printf("\n");
		else
			break;

	}

	return 0;
}


