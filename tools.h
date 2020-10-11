#ifndef _tools_head
#define _tools_head
#include<string>
#include<vector>
#include<time.h>
#include <fstream>
#include<algorithm>
#include<vector>
#include <string>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include <list>
#include<map>
#include<set>
#include<math.h>
#include<stack>
#include<sstream>
#include<queue>
#include "float.h"
#define COMPILE_LINUX 1
using namespace std;
#define MINI(x,y) (((x)<(y))? (x):(y))
#define MAX(x,y) (((x)>(y))? (x):(y))
#define DATA_DIR "/home/luods/Desktop/NameDis/data/"
void getcontext(string& origin);
void split(vector<string>&, string &origin, char c);
void split(vector<string>&, string &origin, set<char> &sps);
void split(set<string>&, string &origin, char c);
void delNumber(string &);
string norm(string &origin);
bool equals(string &s1, string& s2);
void getFiles(string path, vector<string> &names);
string normtitle(string &tword);
void uniFind(int *, int);

#endif
