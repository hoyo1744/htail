
//c header
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

//c++header
#include <string>
#include <algorithm>
#include <vector>
#include <stack>
#include <map>
#include <queue>
using namespace std;




#define FILE_NAME_SIZE 256
#define FILE_BUFF 1024
#define TRUE 1
#define FALSE 0

#define SET_COLOR_GREEN "\033[1;32m"
#define SET_COLOR_INIt "\033[1;0m"

#define COLOR_YELLOW 33
#define COLOR_INIT 0

string searchInit="Init";
string searchClose="exit";

string colorGreenStart="\033[1;32m";
string colorGreenEnd="\033[1;0m";

