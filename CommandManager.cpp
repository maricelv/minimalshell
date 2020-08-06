/////////////////////////////////////////////////////////////////////////////
// CommandMananger.cpp - package to create and manage shell variables      //                                                         //
// Language:    gcc/g++ v7.4.0                                             //
// Platform:    Dell Precision Mobile Workstation m7200                    //
//              Linux Mint 19.3 (64-bit)                                   //
/////////////////////////////////////////////////////////////////////////////

#include "CommandManager.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace CSE384 {

void CommandManager::SetShellVariable(const std::string& var_name, const std::string value)
{
  varTable[var_name] = value;
  if(var_name == "path")
  {
     pathList = ParseString(value, ':');
  }
}

std::string CommandManager::GetShellVariable(const std::string& var_name)
{
  //if shell variable name not found in map iterator to end is returned
  auto val_iter = varTable.find(var_name);
  if(val_iter == varTable.end())
     return "";
  return val_iter->second;
}

std::vector<std::string> CommandManager::ParseString(const std::string& str, char delim)
{
   std::vector<std::string> toks;
   try
   {
     std::stringstream ss(str);
     std::string token;

     while(getline(ss, token, delim))
     {
        toks.push_back(token);
     }
   }
   catch(...)
   {
     // swallow any execption, return empty vector on error
     toks.clear();
   }

   return toks;
}

bool CommandExists(const std::string& command)
{
   std::ifstream f(command.c_str());
   if(f.good())
   {
       f.close();
       return true;
   }
   return false;  
}


std::string getcwd(const std::string& dir)
{
  long size = pathconf(".", _PC_PATH_MAX);
  char* buf = new char[size];
  char* cwd_basepath = ::getcwd(buf, size);
  // std::cout << "RAW PATH: " << cwd_basepath << std::endl;
  std::string cwd_path = std::string(buf);
  delete buf;
  std::string target_dir = "";
  
  if(dir == ".")
    target_dir = cwd_path + "/"; // + command.substr(2, (command.length()-2));
  else if(dir == "..")
  {
     std::size_t found = cwd_path.find_last_of("/");	
     if(found != std::string::npos)
	     target_dir =  cwd_path.substr(0, found+1);   
  }
  // std::cout << "TARGET PATH: " << target_dir << std::endl;
  return target_dir;
}

// this is help function: static keyword cause the function to be visible only within this file
static std::string HandleRelativePath(const std::string& command)
{
     // if command begins with a "/" then assume an absolute path 
     // given, if the file exists and return   
     if(command[0] == '/' )
        if(CommandExists(command))
           return command;

     // if command begins with "./" or "../" expand to current working directory
     if(command.length() >= 3)
     {
       bool isCwdorParentDir = false;
       std::string target_dir = ".";
       int start_index = 2; // assume "./" prefix 
       if(command[0] == '.' && command[1] == '/')
       {  
	       isCwdorParentDir = true;
	       target_dir = ".";
       }
       //else if command begins with "../" expand to the parent of current working directory
       else if(command[0] == '.' && command[1] == '.' && command[2] == '/')
       {
         start_index = 3;
         isCwdorParentDir = true;
	       target_dir = "..";
       }
      
       // assuming either "." or ".." expland to the directory and builf the absolute path
       if(isCwdorParentDir)
       {
	       //build the absolute path
	       std::string absolute_path = getcwd(target_dir) + 
		                         command.substr(start_index, (command.length()-start_index)); 
        //std::cout << "ABSOLUTE PATH: " << absolute_path << std::endl;
         if(CommandExists(absolute_path))
            return absolute_path;
         else
           return "notfound";
        
       }
     }

     return "";  //default program doesn't exist or command doesn't begin with "." or ".."
}

std::string CommandManager::SearchPath(const std::string& command)
{
     // check if specified program exists in relative path (commands that begin with "." or "..")
     std::string relative_to_absolute_path = HandleRelativePath(command);
     if(relative_to_absolute_path != "" && relative_to_absolute_path != "notfound")
        return relative_to_absolute_path;
     else if(relative_to_absolute_path == "notfound")
        return "";
                  
     // command does not begin with "." or ".." so search path
     if(command.length() > 0)
     {
       // otherwise search the path
       for(int i = 0; i < pathList.size(); ++i)
       {
         std::string command_dir = pathList[i];
         //path statement should begin with a / or .
         if(command_dir[0] == '/')
         {
           if(command_dir[command_dir.length()-1] != '/')
              command_dir += '/';
	         std::string absolute_path = command_dir + command; 
           if(CommandExists(absolute_path))
              return absolute_path;
         }
         else if(command_dir[0] == '.')
         {
           command_dir =  getcwd(command_dir);
           std::string absolute_path = command_dir + command;
	      
	        if(CommandExists(absolute_path))
             return absolute_path;
	       }
      }
    }

     return "";
}

};


#ifdef TEST_COMMAND
#include<iostream>
using namespace CSE384;
int main()
{
   std::cout << "test command manager" << std::endl;   
   CommandManager  commandManager;
   // test set a shell variable: name=Mike Corley
   commandManager.SetShellVariable("name", "Mike corley");
  
   // test get the shell variable
   std::string val = commandManager.GetShellVariable("name");
   std::cout << "value is: " << val << std::endl;
   
   std::cout << "Test searching path for command: \"ls\" no path set " << std::endl;
   // search for the specified  "ls" executable in the search path
   std::string absolute_path = commandManager.SearchPath("ls");
   //if the path variable is not set or the executable ("ls") is not found,
  // SearchPath returns the empty string "".  If the executable is found,
  //the then the abosolute path is returned. e.g. /bin/ls 
   if(absolute_path == "")
      std::cout << "Path variable or executable not found" << std::endl;
   else
     std::cout << "absolute path of  \"ls\" is: " << absolute_path << std::endl; 

   commandManager.SetShellVariable("path", "./:/usr/bin:/bin");

   std::cout << "Test searching path for command: \"ls\" path=./:/usr/bin:/bin" << std::endl;
   absolute_path = commandManager.SearchPath("ls");
   if(absolute_path == "")
      std::cout << "Path variable or executable not found" << std::endl;
   else
     std::cout << "absolute path of  \"ls\" is: " << absolute_path << std::endl;


   std::cout << "Test searching path for command: \"/bin/ls\" " << std::endl;
   absolute_path = commandManager.SearchPath("/bin/ls");
   if(absolute_path == "")
      std::cout << "Path variable or executable not found" << std::endl;
   else
     std::cout << "absolute path of  \"ls\" is: " << absolute_path << std::endl;


   std::cout << "Test searching path for command: \"lss\" -- a command that doesn't exist" << std::endl;
   absolute_path = commandManager.SearchPath("lss");
   if(absolute_path == "")
      std::cout << "Path variable or executable not found" << std::endl;
   else
     std::cout << "absolute path of  \"ls\" is: " << absolute_path << std::endl;

   return 0;
}

#endif
