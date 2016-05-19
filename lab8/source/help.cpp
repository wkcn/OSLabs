#include "io.h"

const char *HELP_INFO = "\
Input a 1~5 for parallel running. A stream nums for serial running \n\r\
Commands:\n\r\
r        Go to look user processes\n\r\
ls       list all programs\n\r\
cls      Clear Screen\n\r\
uid		 See the id of current user\n\r\
top      View running processes of current user\n\r\
topa     View running processes of all user\n\r\
wake     Wake a process, ex: wake 1\n\r\
suspend  Suspend a process, ex: suspend 2\n\r\
pr       Set the priority of a task ex: pr [pid] [priority]\n\r\
kill     Kill a process, ex: kill 3\n\r\
killall  Kill all Processes\n\r\
open     Open a file\n\r\
file     See the state of the file\n\r\
write    Write data to the file\n\r\
read     read data from the file\n\r\
seekg    Set the pointer of getting file\n\r\
seekp    Set the pointer of putting file\n\r\
uname    Show os info\n\r\
Keys:\n\r\
Esc      Back to Shell but not kill processes\n\r\
Ctrl+C   Clear Screen\n\r\
Ctrl+Z   Back to Shell and kill all processes\n\r\
Alt+Num  Tab User: Alt+1 ALt+2 Alt+3 Alt+4\n\r\
";

int main(){
	PrintStr(HELP_INFO,WHITE);
	return 0;
}
