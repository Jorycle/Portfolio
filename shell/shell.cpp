#include "shell.h"

bool inputRed = false;
bool outTrunc = false;
bool outAppend = false;
bool errTrunc = false;
bool errAppend = false;

/*
 * Main program entry.
 */
int main()
{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ); //Set output to unbuffered
  int exitChecker;

  //Enter the shell loop
  if ((exitChecker = shellLoop()) == -1)
      printf("Unexpected termination.\n");
  else
    printf("Exiting 1730sh.\n");

  return exitChecker;

} //main

void sigchildhandler(int signum)
{
  jobNotification();
}

/*
 * The shell loop! Does nothing but loop until a proper exit.
 */
int shellLoop()
{
  shl_processes = 0;
  shl_stdin = "STDIN_FILENO";
  shl_stdout = "STDOUT_FILENO";
  shl_stderr = "STDERR_FILENO";
  int shellStatus = 1;

  //JOB CONTROL BLOCK
  shell_terminal = STDIN_FILENO; //JOB CONTROL - SETTING THE TERMINAL!
  lastExitStatus = 0; //JOB CONTROL - STATUS
  foregroundJob = true; //JOB CONTROL - STUFF
  //Ignore a bunch of signals
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  //signal(SIGCHLD, SIG_IGN);
  //Put shell in its own process group
  shellPG = getpid();
  if (setpgid (shellPG, shellPG) < 0)
    {
      perror ("Shell process group");
      exit(EXIT_FAILURE);
    }
  tcsetpgrp(shell_terminal, shellPG);
  //END OF JOB CONTROL STUFF  

  //Loop until bad status
  while (shellStatus >= 0)
    {
      std::string cmd;                //cmd input
   
      jobNotification();

      //Get current dir and fiddle with it for pretty ~ prompts
      char * rawDirName = get_current_dir_name();
      std::string homePath = getenv("HOME");
      std::string currentDirName(rawDirName);
      if (!strncmp(currentDirName.c_str(), homePath.c_str(), homePath.size()))
	{ //We're in a sub directory of home, ~ it up
	  std::string truncDirName = currentDirName.substr(homePath.size(),std::string::npos);
	  printf("shell:~%s$ ", truncDirName.c_str());
	}
      else //We're not in a subdirectory of home, don't ~
	printf("shell:%s$ ", currentDirName.c_str()); //shell prompt
      free(rawDirName);

      std::getline(std::cin, cmd);    //Get input
      if (std::cin.eof() == 1) //Break if the user ctrl+Ds
	cmd = "exit";

      jobArg = cmd;
 
      if (cmd.length() > 0) //Only if an actual command input
	{
	  cmdParser(cmd); //Pass to the parsing function

	  if (shl_argv.size() > 0) //If the argument list is valid
	    {
	      if (std::find(builtinBeg, builtinEnd, shl_argv[0].at(0)) !=  builtinEnd)
		builtinHandler(); //It's a builtin, do builtin stuff
	      else //Not a builtin, must be a command
		  commandRunner();
	    } //Exiting arg size if
	} //Exiting cmd size if

            
      //If everything's cool, print the job parse output
      //      if (shellStatus >= 0)          
      //	outputPrinter();
      
      //RESET EVERYTHING BACK TO DEFAULT
      inputRed = outTrunc = outAppend = errTrunc = errAppend = false;

      shl_stdin = "STDIN_FILENO";
      shl_stdout = "STDOUT_FILENO";
      shl_stderr = "STDERR_FILENO";
      shl_processes = 0;
      foregroundJob = true; //RESET JOB CONTROL VARIABLE
      shl_argv.clear();
      jobArg.clear(); //RESET JOBARG
    }

  return 0;
}

/*
 * Handles the built-in commands:
 * exit, cd, export, jobs, fg, bg, kill, help
 */
void builtinHandler()
{
  std::string homePath = getenv("HOME");
  int opt;
  int signo = 15;
  int processToKill;
  char * sigToSend;
  bool sOption = false;

  if (shl_argv[0].at(0) == "exit") //"Exit" input entered
    {
      if (shl_argv[0].size() == 1) //No custom exit status
	exit(lastExitStatus);
      else //Custom exit status, use it
	{//But make sure it's valid first
	  char * eCheck = (char *)shl_argv[0].at(1).c_str();
	  long converter = strtol(shl_argv[0].at(1).c_str(), &eCheck, 10);

	  if (*eCheck) //Not a valid number!
	    exit(lastExitStatus);
	  else //Valid number
	    exit(converter);
	}
    }
  else if (shl_argv[0].at(0) == "cd") //"cd" input entered
    { //No argument = go home
      if (shl_argv[0].size() == 1)
	chdir(homePath.c_str());
      else
	{ //Argument = go there
	  if (chdir(shl_argv[0].at(1).c_str()) == -1)
	    perror("-1730sh: cd");
	}
    }
  else if (shl_argv[0].at(0) == "export")
    {
      if (shl_argv[0].size() > 1)
	{
	  putenv((char *)shl_argv[0].at(1).c_str());
	}
    }
  else if (shl_argv[0].at(0) == "jobs") //JOB CONTROL - JOB LIST
    {
      printJobs();
    }
  else if (shl_argv[0].at(0) == "fg") //JOB CONTROL - FOREGROUND
    {
      pid_t fgpid = atoi(shl_argv[0].at(1).c_str());
      continueJob(fgpid, true);
    }
  else if (shl_argv[0].at(0) == "bg") //JOB CONTROL - BACKGROUND
    {
      pid_t bgpid = atoi(shl_argv[0].at(1).c_str());
      continueJob(bgpid, false);
    }
  else if (shl_argv[0].at(0) == "kill")
    {
      //   shl_argv[0].erase(shl_argv[0].begin() + 0); //Get rid of kill before conversion
      vector<char *> killArgs = convertVector(shl_argv[0]); //CONVERT
      while ((opt = getopt(killArgs.size() -1, &killArgs.at(0), "s:")) != -1)
	{
	  switch (opt)
	    {
	    case 's':
	      sigToSend = optarg;
	      sOption = true;
	      break;
	    default:
	      optind--;
	    }
	}
      if (!killArgs.at(optind))
        {
          printf("kill: process id required.\n");
          return;
        }

      if (sOption)
	{
	  char * killCheck = killArgs.at(optind);
	  long converter = strtol(killArgs.at(optind), &killCheck, 10);

	  if (*killCheck) //Not a valid number!
	    {
	      try
		{
		  signo = signal_map.at(sigToSend);
		}
	      catch (std::exception & e)
		{
		  printf("kill: invalid signal\n");
		  return;
		} // try
	    }
	  else //Valid number
	    signo = converter;
	}
      processToKill = atoi(killArgs.at(optind));
      int jobToKill; 
      
      if (processToKill < -1)
	{
	  if ((jobToKill = findJob((-1)*processToKill)) > 0)
	    {
	      for (unsigned int i = 0; i < vectorJobs[jobToKill].processes.size(); i++)
		kill (vectorJobs[jobToKill].processes[i].pid, signo);
	    }
	}
      
      else if (kill (processToKill, signo) < 0)
	perror("kill");
    }
  else if (shl_argv[0].at(0) == "help") //"Help" input entered"
    {
      if (shl_argv[0].size() == 1) //No argument, show all topics
	{
	  printf("Available help topics:\n%s\t%s\t%s\n", "bg", "cd", "exit"); // LINE 1
	  printf("%s\t%s\t%s\n", "export", "fg", "help"); // LINE 2
	  printf("%s\t%s\n", "jobs", "kill");
	}
      else if (std::find(builtinBeg, builtinEnd, shl_argv[0].at(1)) ==  builtinEnd) //Invalid help topic
	printf("Invalid help topic.\n");
      else if (shl_argv[0].at(1) == "help") //Help help
	printf("Get help for various commands supported by the shell.\n%s\n%s\n", "\nFormat: help [topic]\n", "Example: help help");
      else if (shl_argv[0].at(1) == "cd") //Help cd
	printf("Change the current working directory.\n%s\n%s\n","\nFormat: cd [PATH]\n", "Example: cd myDirectoryName");
      else if (shl_argv[0].at(1) == "exit") //Help exit
	printf("Exit the shell. You can exit with the status of the last exited job or a custom status.\n%s\n%s\n",
	       "\nFormat: exit [OPTIONAL STATUS INTEGER]\n", "Example: exit 1");
      else if (shl_argv[0].at(1) == "bg") //Help bg
        printf("Resume the stopped job ID in the background, as if it had been started with &.\n%s\n%s\n","\nFormat: bg [JOB ID]\n", "Example: bg 1234");
      else if (shl_argv[0].at(1) == "fg") //Help fg
        printf("Resume the stopped job ID in the foreground, and make it the current job.\n%s\n%s\n","\nFormat: fg [JOB ID]\n", "Example: fg 1234");
      else if (shl_argv[0].at(1) == "export") //Help export
        printf("Name is automatically included in the environment of subsequently executed jobs.\n%s\n%s\n","\nFormat: export NAME=[WORD]\n", "Example: export MyVar=5");
      else if (shl_argv[0].at(1) == "jobs") //Help jobs
        printf("List jobs currently in operation.\n%s\n%s\n","\nFormat: jobs\n", "Example: jobs\n(Yes, it's really that easy)");
      else if (shl_argv[0].at(1) == "kill") //Help kill
        printf("Send a signal to specified process or job ID. Positive values correspond to processes, negative values correspond to jobs.\n-s [signal] may be used to specificy a specific signal, otherwise, SIGTERM is used.\n%s\n%s\n","\nFormat: kill [-s SIGNAL] PID\n", "Example: signal -s SIGCONT 1234");
    }
}

/*
 * Parses the command input. Very ugly.
 * @param cmd The string to be parsed character by character.
 */
void cmdParser(std::string cmd)
{
  enum parseTypes {
    appendOut, truncateOut, redirectIn, errorAppend, errorTruncate, regular
  };
  parseTypes wordTypes = regular;
  size_t pos = 0;
  size_t startSub = 0;
  bool inQuotes = false;
  bool wordFinished = false;
  bool reset = false; //Needed to break out of the loop for certain characters

  //The array to temporarily store things
  vector<std::string> cmdArgs;

  // This is a background job
  if (cmd[cmd.length() - 1] == '&')
    {
      foregroundJob = false;
      cmd.erase((cmd.length() - 1),1); 
    }

  //The big loop: continue until the string is finished
  while (pos < cmd.length())
    {
      if (!wordFinished) //If the word isn't finished, go here
      	{
	  if (reset) //If we have reset to be here, clear the reset flag
	    reset = false;

	  //Not a quote, fast forward through spaces
	  while (!inQuotes && isspace(cmd[pos]) && pos < cmd.length())
	    {
	      pos++;
	      startSub++;
	    }

	  //We're entering a quote, enable special handling
	  if (cmd[pos] == '"' && cmd[pos-1] != '\\')
	    {
	      inQuotes = true;
	      pos++;
	      startSub++;
	    }

	  //Loop through non-space characters not in quotes
	  while (!inQuotes && !wordFinished && !isspace(cmd[pos]) && pos < cmd.length())
	    {
	      //If the start symbol is a special character, prepare to
	      //handle it in a special way
	      if (startSub == pos && ((cmd[pos] == '>') || (cmd[pos] == '<') || (cmd[pos] == 'e' && cmd[pos+1] == '>') ||cmd[pos] == '|'))
	      	{
		  if (cmd[pos] == '>') //Output handling
		    {
		      pos++;
		      startSub++;	      
		      if (cmd[pos] == '>') //>> = append
			{
			  pos++;
			  startSub++;
			  wordTypes = appendOut;
			}
		      else // > = truncate
			wordTypes = truncateOut;
		    }
		  
		  else if (cmd[pos] == '<')
                    { // < = input redirect
		      wordTypes = redirectIn;
		      pos++;
		      startSub++;
                    }
		  
                  else if (cmd[pos] == 'e')
                    { // e> = truncate
		      pos+=2;
                      startSub+=2;
                      if (cmd[pos] == '>')
                        {// e>> = append
                          pos++;
                          startSub++;
			  wordTypes = errorAppend;
			}
		      else
			wordTypes = errorTruncate;
		    }		  
		  
		  else if (cmd[pos] == '|')
                    { //Pipe = new process
		      if (cmdArgs.size() > 0) //Don't break on empty vectors
			{
			  shl_argv.push_back(cmdArgs);
			  cmdArgs.clear();
			}
		      pos++;
		      startSub++;
		      wordTypes = regular;
		    }
		  
		  reset = true; //Break back to the beginning loop
		  
	      	} //EXITING STARTPOS == WEIRD SYMBOL IF STATEMENT
	      else //IF ANYTHING ELSE - regular characters
	      	{
		  if (cmd[pos] == '"' && cmd[pos-1] != '\\')
		    { //This is a quote in a normal word, so we get rid of it
		      cmd.erase(pos,1);
		      pos--;
		      continue;
		    }
		  
		  //This is a special character in the middle of a string, finish the word
		  if ((cmd[pos] == '>') || (cmd[pos] == '<') || (cmd[pos] == 'e' && cmd[pos+1] == '>') || (cmd[pos] == '|'))
		      wordFinished = true;
		  
		  //If the word's not finished yet, move forward
		  if (!wordFinished)
		    pos++;
		  
		}
	      
	      if (reset) //Reset = break out of this loop too
		break;

	    }//EXITING THE NON-QUOTE WHILE LOOP - HIT A SPACE OR END	  
	 
	  if (reset) //Reset = restart the main loop
	    continue;
	  
	  //We're in a quotation, fast forward through the entire string
	  while (inQuotes && !wordFinished && pos < cmd.length())
	    {
	      if (cmd[pos] == '"' && cmd[pos-1] != '\\')
		{ //This is the end of the quote
		  cmd.erase(pos,1); //Get rid of the quotation mark
		  pos--;
		  wordFinished = true;
		  inQuotes = false;
		}
	      
	      pos++;
	    }

	}//Exiting "if !WordFinished"	  
      
      wordFinished = true; //I mean we have to be done if we're here, right?

      if (wordFinished) //Handle finished words
	{
	  std::string temp(cmd.substr(startSub, pos-startSub));
	  
	  //Removing the \ from escape sequence quotes
	   for (uint i = 0; i < temp.length(); i++)
	    {
	      if (temp[i] == '\\' && temp[i+1] == '"')
		temp.erase(i,1);
	    }

	   //If the string isn't empty, handle it! 
	   if (temp.length() > 0)
	     {
	       switch (wordTypes)
		 {
		 case appendOut:
		   {//Set append out
		     shl_stdout = temp;
		     outAppend = true;
		     //		     shl_stdout += " (append)";
		     break;
		   }
		 case truncateOut:
		   {//Set truncate out
		     shl_stdout = temp;
		     outTrunc = true;
		     //shl_stdout += " (truncate)";
		     break;
		   }
		 case redirectIn:
		   {//Set input redirection
		     shl_stdin = temp;
		     inputRed = true;
		     break;
		   }
		 case errorTruncate:
		   {//Set error truncate redirection
		     shl_stderr = temp;
		     errTrunc = true;
		     //shl_stderr += " (truncate)";
		     break;
		   }
		 case errorAppend:
		   {//Set error append redirection
		     shl_stderr = temp;
		     errAppend = true;
		     //shl_stderr += " (append)";
		     break;
		   }
		 default:
		   {//Normal word, put it in the array
		     cmdArgs.push_back(temp);
		     break;
		   }
		 }
	       wordTypes = regular;
	     }
	   
	   //Reset our variables
	   startSub = pos;
	   inQuotes = wordFinished = false;
	}  
    }//Exiting entire while loop

  //Push whatever is left to the thing
  if (cmdArgs.size() > 0)
      shl_argv.push_back(cmdArgs);

  shl_processes = shl_argv.size();
  cmdArgs.clear();
}

/*
 * Prints our job info and process stuff for debugging.
 */
void outputPrinter()
{
  printf("\n");
  printf("Job STDIN = %s\n", shl_stdin.c_str());
  printf("Job STDOUT = %s\n", shl_stdout.c_str());
  printf("Job STDERR = %s\n", shl_stderr.c_str());

  printf("\n");

  printf("%d pipe(s)\n", shl_processes > 0 ? shl_processes - 1 : 0);
  printf("%d process(es)\n", shl_processes);

  printf("\n");

  for (unsigned int i = 0; i < shl_argv.size(); i++)
    {
      printf("Process %d argv:\n", i);
      for (unsigned int j = 0; j < shl_argv[i].size(); j++)
	{
	  printf("%d: %s\n", j, shl_argv[i].at(j).c_str());
	}
      printf("\n");
    }
}


//JOB CONTROL STUFF IS EVERYTHING BELOW HERE
/*
 * Runs our programs
 */
void commandRunner()
{
  int pid;
  pid_t pgid;
  struct job newJob;

  vector<array<int, 2>> pipes; //Our pipe arrays

  newJob.command = jobArg;
  newJob.status = "Running";

  //CYCLE THROUGH ARGUMENT ARRAYS
  for (unsigned int i = 0; i < shl_argv.size(); ++i)
    {    
      struct process newProcess;

      if (i != shl_argv.size() - 1) //Not the last command
	{
	  int pipefd [2]; //Make some pipes
	  if (pipe(pipefd) == -1)
	    {
	      perror("-1730sh: pipe");
	      exit(EXIT_FAILURE);
	    } //if
	  pipes.push_back({pipefd[0], pipefd[1]});
	} // if
      
      //Start FORKING processes
      if ((pid = fork()) == -1)
	{
	  perror("-1730sh: fork");
	  exit(EXIT_FAILURE);
	}
      else if (pid == 0) //In the child process
	{
	  //Reset the ignores to default for child
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTSTP, SIG_DFL);
	  signal(SIGTTIN, SIG_DFL);
	  signal(SIGTTOU, SIG_DFL);
	  signal(SIGCHLD, SIG_DFL);

	  pid = getpid();
	  if (!pgid)
	    pgid = pid;
	  setpgid(pid, pgid);
	  	  
	  if (inputRed)
	    {
	      int fd0 = open(shl_stdin.c_str(), O_RDONLY);
	      dup2(fd0, STDIN_FILENO);
	      close(fd0);
	    }
	  
	  if (outAppend)
	    {
	      int fd1 = open(shl_stdout.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
	      dup2(fd1, STDOUT_FILENO);
	      close(fd1);
	    }
	  else if (outTrunc)
	    {
	      int fd2 = creat(shl_stdout.c_str(), 0644);
	      dup2(fd2, STDOUT_FILENO);
	      close(fd2);
	    }
	  if (errAppend)
	    {
	      int fd3 = open(shl_stderr.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
	      dup2(fd3, STDERR_FILENO);
	      close(fd3);
	    }
	  else if (errTrunc)
	    {
	      int fd4 = creat(shl_stderr.c_str(), 0644);
	      dup2(fd4, STDERR_FILENO);
	      close(fd4);
	    }
	  
	  if (i != 0)
	    { //Not the first command
	      if (dup2(pipes.at(i-1)[0], STDIN_FILENO) == -1)
		{
		  perror("-1730sh: dup2");
		  exit(EXIT_FAILURE);
		}
	    }
	  if (i != shl_argv.size() - 1)
	    { //Not the last command
	      if (dup2(pipes.at(i)[1], STDOUT_FILENO) == -1)
		{
		  perror("-1730sh: dup2");
		  exit(EXIT_FAILURE);
		}
	    }

	  //close all pipes created so far
	  for (unsigned int i = 0; i < pipes.size(); ++i)
	    close_pipe(pipes.at(i).data());
	  //Finally, let's make a program!
	  execProg(shl_argv.at(i));
	
	} // end if-child
      else
	{
	  newProcess.pid = pid;
	  if (!pgid)
	    {
	      newJob.pgid = pid;
	      pgid = pid;
	    }
	  setpgid(pid, pgid);
	}
      newJob.processes.push_back(newProcess);
    } // end for all commands

  vectorJobs.push_back(newJob);

  // close all pipes
  for (unsigned int i = 0; i < pipes.size(); ++i)
    close_pipe(pipes.at(i).data());
  
  if (foregroundJob)
    jobToForeground(pgid, false);
  else
    jobToBackground(pgid, false);
  
}

/*
 * Converts a vector of strings into a vector of characters. Adds nullptr
 * as the last element of the vector.
 * @param stringVector The vector of strings to be converted.
 * @return The new vector of characters.
 */
vector<char *> convertVector(vector<std::string> &stringVector)
{
  vector<char *> charVector;

  for (unsigned int i = 0; i < stringVector.size(); ++i)
    {
      charVector.push_back(new char [stringVector.at(i).size() + 1]);
      strcpy(charVector.at(i), stringVector.at(i).c_str());
    }
  charVector.push_back(nullptr);
  
  return charVector;
  }

/*
 * Deletes our dynamically allocated character array from conversion.
 * @param charVector Vector to be deleted.
 */
void deleteVector(vector<char *> & charVector)
{
  for (unsigned int i = 0; i < charVector.size(); ++i)
    delete[] charVector.at(i);
}

/*
 * Executes the program with an argument list of strings. Sends the
 * strings to be converted to characters as appropriate first.
 * @param stringArgs Vector of strings to be passed to program.
 */
void execProg(vector<std::string> stringArgs)
{
  vector<char *> charArgs = convertVector(stringArgs);
  execvp(charArgs.at(0), &charArgs.at(0));
  perror("-1730sh: "); //This only occurs if we done goofed
  deleteVector(charArgs);
  exit(EXIT_FAILURE);
}

/*
 * Closes the pipes!
 * @param pipefed The array of pipes to be closed
 */
void close_pipe(int pipefd [2])
{
  if (close(pipefd[0]) == -1)
    {
      perror("-1730sh: ");
      exit(EXIT_FAILURE);
    }
  if (close(pipefd[1]) == -1)
    {
      perror("-1730sh: ");
      exit(EXIT_FAILURE);
    }
}

//Print the list of all active jobs
void printJobs()
{
  if (vectorJobs.size() > 0)
    printf("JID\tStatus\t\tCommand\n");

  for (unsigned int i = 0; i < vectorJobs.size(); i++)
    {
      printf("%d\t%s\t\t%s\n", vectorJobs[i].pgid, vectorJobs[i].status.c_str(), vectorJobs[i].command.c_str());
    }

}

//Sends the job to the foreground
void jobToForeground(pid_t pgid, bool cont)
{
  tcsetpgrp(shell_terminal, pgid);

  // Send the job a continue signal if needed
  if (cont)
    {
      if (kill (-pgid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }
  
  jobWaiter(pgid);

  //Put shell back into foreground
  tcsetpgrp(shell_terminal, shellPG);
}

//Sends the job to the background
void jobToBackground(pid_t pgid, bool cont)
{
  // Send the job a continue signal if called
  if (cont)
    {
      if (kill (-pgid, SIGCONT) < 0)
	perror ("kill (SIGCONT)");
    }
}

//Waits on the job
void jobWaiter(pid_t pgid)
{
  pid_t wpid;
  int status;

  do
    {
      wpid = waitpid(WAIT_ANY, &status, WUNTRACED);
      
    } while (!setProcessStatus(wpid, status) && !isJobComplete(vectorJobs[findJob(pgid)]) && !isJobStopped(vectorJobs[findJob(pgid)]));
  
  //  tcsetpgrp(shell_terminal, shellPG);
   

  //exitStatusHandler(pgid, status);
}

//Continues the job
void continueJob(pid_t pgid, bool foreground)
{
  int job = findJob(pgid);
  
  if (job != -1)
    {
      for (unsigned int i = 0; i < vectorJobs[job].processes.size(); i++)
	vectorJobs[job].processes[i].stopped = false;
      printf("%d\t%s\t\t%s\n", vectorJobs[job].pgid, "Continued", vectorJobs[job].command.c_str());
      vectorJobs[job].notified = false;
      vectorJobs[job].status = "Running";
    }
  if (foreground)
    jobToForeground (pgid, true);
  else
    jobToBackground (pgid, true);
}

//Finds the vector with the job in the jobsList
int findJob(pid_t pgid)
{
  for (unsigned int i = 0; i < vectorJobs.size(); i++)
    {
      if (vectorJobs[i].pgid == pgid)
	return i;
    }
  return -1;
}

int isJobComplete(struct job jobCheck)
{
  for (unsigned int i = 0; i < jobCheck.processes.size(); i++)
    {
      if (!jobCheck.processes[i].completed)
	return 0;
    }
  return 1;
}

int isJobStopped(struct job jobCheck)
{
  for (unsigned int i = 0; i < jobCheck.processes.size(); i++)
    {
      if (jobCheck.processes[i].stopped)
        return 1;
    }

  return 0;

}

int setProcessStatus(pid_t pid, int status)
{
  if (pid > 0)
    {
      for (unsigned int i = 0; i < vectorJobs.size(); i++)
        for (unsigned int j = 0; j < vectorJobs[i].processes.size(); j++)
          if (vectorJobs[i].processes[j].pid == pid)
            {
              vectorJobs[i].processes[j].status = status;
              vectorJobs[i].lastExitStat = status;
	      if (WIFSTOPPED (status))
                vectorJobs[i].processes[j].stopped = true;
	      else
                {
                  vectorJobs[i].processes[j].completed = true;
                  if (WIFSIGNALED (status))
		    {
		      printf("%d terminated by signal (%d)\n", pid, WTERMSIG (vectorJobs[i].processes[j].status));
		    }
                }
              return 0;
	    }
      printf("No child process %d\n", pid);
      return -1;
    }
  else if (pid == 0 || errno == ECHILD)
    return -1; //No processes are done
  else
    { //Other stuff is broken
      perror ("waitpid");
      return -1;
    }
}

void updateStatus()
{
  int status;
  pid_t pid;
  
  do
    pid = waitpid(WAIT_ANY, &status, WUNTRACED|WNOHANG);
  while (!setProcessStatus(pid, status));
}

void jobNotification()
{
  updateStatus(); //Update child process stuff

  for (unsigned int i = 0; i < vectorJobs.size(); i++)
    {
      if (isJobComplete(vectorJobs[i])) //If everything's done, tell the user and delete it
	{
	  if (WIFEXITED(vectorJobs[i].lastExitStat)) //Normal exit
	    lastExitStatus = WEXITSTATUS(vectorJobs[i].lastExitStat);
	  else if (WIFSIGNALED(vectorJobs[i].lastExitStat)) //Abnormal exit
	    lastExitStatus = WTERMSIG(vectorJobs[i].lastExitStat);
	  else
	    lastExitStatus = vectorJobs[i].lastExitStat;
	  
	  printf("%d\t%s (%d)\t\t%s\n", vectorJobs[i].pgid, "Exited", lastExitStatus, vectorJobs[i].command.c_str());
	  vectorJobs.erase(vectorJobs.begin() + i);
	  i--;
	}
      else if (isJobStopped(vectorJobs[i]) && !vectorJobs[i].notified)
	{ //If things have stopped, tell the user
	  printf("%d\t%s\t\t%s\n", vectorJobs[i].pgid, "Stopped", vectorJobs[i].command.c_str());
	  vectorJobs[i].status = "Stopped";
	  vectorJobs[i].notified = true;
	}
    }
}
