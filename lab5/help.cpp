#include "include/io.h"

const char *HELP_INFO = "\
Input a 1~5 for parallel running. A stream nums for serial running \n\r\
Commands:\n\r\
r        Go to look user processes\n\r\
ls       list all programs\n\r\
cls      Clear Screen\n\r\
top      View all running processes\n\r\
wake     Wake a process, ex: wake 1\n\r\
suspend  Suspend a process, ex: suspend 2\n\r\
kill     Kill a process, ex: kill 3\n\r\
killall  Kill all Processes\n\r\
uname    Show os info\n\r\
Keys:\n\r\
Esc      Back to Shell but not kill processes\n\r\
Ctrl+C   Clear Screen\n\r\
Ctrl+Z   Back to Shell and kill all processes\n\r\
";

int main(){
	PrintStr(HELP_INFO,WHITE);
	return 0;
}
