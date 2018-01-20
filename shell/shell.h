#ifndef SHELL_H
#define SHELL_H

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <array>
#include <termios.h>
#include <algorithm>
#include <map>
#include <exception>
#include <fcntl.h>

using std::vector;
using std::array;

std::string builtinLibrary[] = {"bg", "cd", "exit", "export", "fg", "help", "jobs", "kill"}; 
std::string * builtinBeg = builtinLibrary;
std::string * builtinEnd = builtinLibrary + 8;

int lastExitStatus;
std::string jobArg;

int shl_processes;
pid_t shellPG;
int shell_terminal;
bool foregroundJob;
vector<vector<std::string>> shl_argv;
//vector<vector<std::string>> jobsList;

std::string shl_stdin;
std::string shl_stdout;
std::string shl_stderr;

int shellLoop();
void outputPrinter();
void cmdParser(std::string cmd);
void builtinHandler();

//Command running/job control stuff be below here
vector<char *> convertVector(vector<std::string> &stringVector);
void deleteVector(vector<char *> & charVector);
void execProg(vector<std::string> stringArgs);
void close_pipe(int pipefd[2]);
void commandRunner();
void printJobs();
void jobToForeground(pid_t pgid, bool cont);
void jobToBackground(pid_t pgid, bool cont);
void jobWaiter(pid_t pgid);
void continueJob(pid_t pgid, bool foreground);
int findJob(pid_t pgid);

int isJobComplete(struct job jobCheck);
int isJobStopped(struct job jobCheck);
int setProcessStatus(pid_t pid, int status);
void updateStatus();
void jobNotification();
void sigchildhandler(int signum);

struct process
{
  pid_t pid;
  bool completed = false;
  bool stopped = false;
  int status;
};

struct job
{
  std::string command;
  std::string status;
  vector<process> processes;
  pid_t pgid;
  bool notified = false;
  int lastExitStat;
};

vector<job> vectorJobs;

const std::map<std::string, int> signal_map { 
#ifdef SIGHUP
 {"SIGHUP", SIGHUP},
#endif
#ifdef SIGINT
 {"SIGINT", SIGINT},
#endif
#ifdef SIGQUIT
 {"SIGQUIT", SIGQUIT},
#endif
#ifdef SIGILL
 {"SIGILL", SIGILL},
#endif
#ifdef SIGTRAP
 {"SIGTRAP", SIGTRAP},
#endif
#ifdef SIGABRT
 {"SIGABRT", SIGABRT},
#endif
#ifdef SIGIOT
 {"SIGIOT", SIGIOT},
#endif
#ifdef SIGBUS
 {"SIGBUS", SIGBUS},
#endif
#ifdef SIGFPE
 {"SIGFPE", SIGFPE},
#endif
#ifdef SIGKILL
 {"SIGKILL", SIGKILL},
#endif
#ifdef SIGUSR1
 {"SIGUSR1", SIGUSR1},
#endif
#ifdef SIGSEGV
 {"SIGSEGV", SIGSEGV},
#endif
#ifdef SIGUSR2
 {"SIGUSR2", SIGUSR2},
#endif
#ifdef SIGPIPE
 {"SIGPIPE", SIGPIPE},
#endif
#ifdef SIGALRM
 {"SIGALRM", SIGALRM},
#endif
#ifdef SIGTERM
 {"SIGTERM", SIGTERM},
#endif
#ifdef SIGSTKFLT
 {"SISTKFLT", SIGSTKFLT},
#endif
#ifdef SIGCHLD
 {"SIGCHLD", SIGCHLD},
#endif
#ifdef SIGCONT
 {"SIGCONT", SIGCONT},
#endif
#ifdef SIGSTOP
 {"SIGSTOP", SIGSTOP},
#endif
#ifdef SIGTSTP
 {"SIGTSTP", SIGTSTP},
#endif
#ifdef SIGTTIN
 {"SIGTTIN", SIGTTIN},
#endif
#ifdef SIGTTOU
 {"SIGTTOU", SIGTTOU},
#endif
#ifdef SIGURG
 {"SIGURG", SIGURG},
#endif
#ifdef SIGXCPU
 {"SIGXCPU", SIGXCPU},
#endif
#ifdef SIGXFSZ
 {"SIGXFSZ", SIGXFSZ},
#endif
#ifdef SIGVTALRM
 {"SIGVTALRM", SIGVTALRM},
#endif
#ifdef SIGPROF
 {"SIGPROF", SIGPROF},
#endif
#ifdef SIGWINCH
 {"SIGWINCH", SIGWINCH},
#endif
#ifdef SIGIO
 {"SIGIO", SIGIO},
#endif
#ifdef SIGPOLL
 {"SIGPOLL", SIGPOLL},
#endif
#ifdef SIGPWR
 {"SIGPWR", SIGPWR},
#endif
#ifdef SIGSYS
 {"SIGSYS", SIGSYS},
#endif
#ifdef SIGUNUSED
 {"SIGUNUSED", SIGUNUSED},
#endif
#ifdef SIGRTMIN
 {"SIGRTMIN", SIGRTMIN},
#endif
#ifdef SIGRTMAX
 {"SIGRTMAX", SIGRTMAX},
#endif

};

#endif
