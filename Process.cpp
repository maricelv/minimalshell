///////////////////////////////////////////////////////////////////////////////////////////
// Process.cpp - package interface for process creation in Linux (Unix/POSIX compliant)  //                                                           //
// Language:     gcc/g++ v7.4.0                                                          //
// Platform:     Dell Precision Mobile Workstation m7200                                 //
//               Linux Mint 19.3 (64-bit)                                                //
///////////////////////////////////////////////////////////////////////////////////////////

#include "Process.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

namespace CSE384
{
  static char** VecToArray(const std::vector<std::string>& toks)
  {
    char** args = new char*[toks.size() + 1];
    int i;
    for(i = 0; i < toks.size(); ++i)
    {
      args[i] = new char[toks[i].length()];
      strcpy(args[i], toks[i].c_str());
    }
    args[i] = NULL;
    return args;
  }

  static void DeleteArgsArgsArray(char** args, int size)
  {
     if(args != NULL)
     {
       for(int i = 0; i < size; ++i)
       {
         delete(args[i]);
       }  
     
       delete args;
    }
  }

  Process::Process()
  {}

  void Process::Start(const std::vector<std::string>& toks, bool wait)
  {
     if(toks.size())
     {
        char** args = NULL;
	     try
	     {
          //check to see if toks[0] is executable
          struct stat statbuf;
	        if( (stat(toks[0].c_str(), &statbuf) == 0) && statbuf.st_mode & S_IXUSR)
	        {	   
             c_pid = fork(); // "spawn of a new process (copy of this process) ->  "man fork" to get details for fork

             // check the return value from fork(), 0 is the child, non zero is the parent
             if(c_pid == 0)
             {
	            // execv needs c style array, so convert the vector
		          args = VecToArray(toks);
                 
		          // this is the child: so exec (load) the command entered by the user
              // this overwrites the child process image with that of the command specified in cmdline   
		          // load the process image 
		          execv(args[0], args);
	         
		          // only get here if exec failed
              std::cerr << "error loading (execv) process image: " << args[0] << std::endl;
	          }
            else
            {
              if(wait)
              {
                Wait();
              }
            }
	       }
	       else
	         std::cerr << toks[0] << " not executable" << std::endl;
      }
      catch(std::exception& ex)
      {
         std::cerr << "Exepected error: " << ex.what() << std::endl;
      }	       
      
      //free up (delete) the resources allocated by VectoArray
      DeleteArgsArgsArray(args, toks.size());
   }	   
 }

 void Process::Wait()
 {
    if(c_pid  > 0)
    {
       // return from fork is non zero, so this is parent
       // all we do in the parent is wait for the child to process to exit
       if( (pid = wait(&status)) < 0)
       {
          // get  the message referenced by global errno
          std::cerr << strerror(errno) << std::endl;      
          //_exit(1);  terminate the parent immediately (lets not do that!)
       } 
    }
 }
};

#ifdef TEST_PROCESS
#include "CommandManager.h"
int main()
{
      std::cout << "Process Test Stub " << std::endl;
      std::vector<std::string> command_tokens {"ls", "-l", "/bin"};
      CSE384::Process p;
      
      // search path variable if set
      CSE384::CommandManager commandManager;
      commandManager.SetShellVariable("path", "./:/usr/bin:/bin");
      std::string absolute_path = commandManager.SearchPath(command_tokens[0]);
      
      // if command is found in the search path (e.g. ls), then execute the command
      if(absolute_path != "")
      {
         // make a copy of the command tokens, and and set the command (tok[0]) 
         // to its absolute path. i.e. "ls" -> "/bin/ls" 
         std::vector<std::string> temp_tokens(command_tokens);
         temp_tokens[0] = absolute_path;
         
         // start the process, (passing the command and the argument tokens)
         p.Start(temp_tokens, true);
      }
      else
      {
         std::cerr << command_tokens[0] << " command not found" << std::endl;
         if(commandManager.GetShellVariable("path") == "")
	          std::cerr << "path variable not set" << std::endl;
      }

      std::cout << "*** End Process Test Stub *** " << std::endl;  

      return 0;
}

#endif
