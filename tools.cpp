#include"tools.h"
#if COMPILE_LINUX
#else
#include<io.h>
#endif

void getcontext(string& origin) {
	const char * chs = origin.c_str();
	int begin = 0, end = 0;
	for (unsigned int i = 0; i<origin.length(); i++)
		if (chs[i] == '>') {
			begin = i + 1;
			break;
		}
	for (unsigned int i = begin; i<origin.length(); i++)
		if (chs[i] == '<') {
			end = i;
			break;
		}
	origin = string(origin.substr(begin, (end - begin)));
}

//vector<string> split(string origin, char c){
//	int begin=0;
//	int next=0;
//	vector<string> result;
//	int end = origin.length();
//	while(next<end){
//		if(origin[next]==c){
//			result.push_back(origin.substr(begin,(next-begin)));
//			begin =next+1;
//		}
//		next++;
//	}
//	result.push_back(origin.substr(begin,(next-begin)));
//	return result;
//}

void split(vector<string>& result, string &origin, char c) {
	if (origin.length() <= 1)
		return;
	int begin = 0;
	int next = 0;
	int end = origin.length();
	while (next<end) {
		if (origin[next] == c) {
			if (next - begin>0)
				result.push_back(origin.substr(begin, (next - begin)));
			begin = next + 1;
		}
		next++;
	}
	if (next - begin>0)
		result.push_back(origin.substr(begin, (next - begin)));
}

void split(set<string>& result, string &origin, char c) {
	if (origin.length() <= 1)
		return;
	int begin = 0;
	int next = 0;
	int end = origin.length();
	while (next<end) {
		if (origin[next] == c) {
			if (next - begin>0)
				result.insert(origin.substr(begin, (next - begin)));
			begin = next + 1;
		}
		next++;
	}
	if (next - begin>0)
		result.insert(origin.substr(begin, (next - begin)));
}


void split(vector<string> &result, string& origin, set<char> &sps) {
	int begin = 0;
	int next = 0;
	int end = origin.length();
	while (next<end) {
		if (sps.find(origin[next]) != sps.end()) {
			result.push_back(origin.substr(begin, (next - begin)));
			begin = next + 1;
		}
		next++;
	}
	result.push_back(origin.substr(begin, (next - begin)));
}

string normtitle(string &origin) {
	const char * chs = origin.c_str();
	char * res = new char[origin.length() + 1];
	int j = 0, i = 0;
	for (; i<origin.length(); i++) {
		if ((chs[i] >= 'a'&&chs[i] <= 'z') || chs[i] == '-')
			res[j++] = chs[i];
		else if ((chs[i] >= 'A'&&chs[i] <= 'Z'))
			res[j++] = chs[i] - 'A' + 'a';
	}
	for (j--; j>0; j--) {
		if (res[j] != 's')
			break;
	}
	res[j + 1] = 0;
	string re = string(res);
	delete[] res;
	return re;
}

string norm(string &origin) {
	const char * chs = origin.c_str();
	char * res = new char[origin.length() + 1];
	int j = 0, i = 0;
	for (i = 0; i<origin.length(); i++) {
		if ((chs[i] >= 'a'&&chs[i] <= 'z') || (chs[i] >= 'A'&&chs[i] <= 'Z'))
			break;
	}
	for (; i<origin.length(); i++) {
		if (chs[i] >= 'a'&&chs[i] <= 'z')
			res[j++] = chs[i];
		else if ((chs[i] >= 'A'&&chs[i] <= 'Z'))
			res[j++] = chs[i] - 'A' + 'a';
		else if (chs[i] == ' '&&res[j - 1] != ' ')
			res[j++] = ' ';
	}
	if (j == 0)
		return "";
	for (j--; j>0; j--) {
		if (res[j] != ' ')
			break;
	}
	res[j + 1] = 0;
	string re = string(res);
	delete[] res;
	return re;
}

bool equals(string &s1, string &s2) {
	if (s1.empty() || s2.empty())
		return false;
	if (s1.length() != s2.length())
		return false;
	unsigned int index = 0;
	while (index++<s1.length())
		if (s1[index] != s2[index])
			return false;
	return true;


}

#if COMPILE_LINUX
void getFiles(string path, vector<string> &names) {
names.push_back("Ajay Gupta.xml");names.push_back("Alok Gupta.xml");names.push_back("Barry Wilkinson.xml");names.push_back("Bing Liu.xml");names.push_back("Bin Li.xml");names.push_back("Bin Yu.xml");names.push_back("Bin Zhu.xml");names.push_back("Bob Johnson.xml");names.push_back("Bo Liu.xml");names.push_back("Charles Smith.xml");names.push_back("Cheng Chang.xml");names.push_back("Daniel Massey.xml");names.push_back("David Brown.xml");names.push_back("David Cooper.xml");names.push_back("David C. Wilson.xml");names.push_back("David E. Goldberg.xml");names.push_back("David Jensen.xml");names.push_back("David Levine.xml");names.push_back("David Nelson.xml");names.push_back("Eric Martin.xml");names.push_back("Fan Wang.xml");names.push_back("Fei Su.xml");names.push_back("Feng Liu.xml");names.push_back("Feng Pan.xml");names.push_back("Frank Mueller.xml");names.push_back("Gang Chen.xml");names.push_back("Gang Luo.xml");names.push_back("Hao Wang.xml");names.push_back("Hiroshi Tanaka.xml");names.push_back("Hong Xie.xml");names.push_back("Hui Fang.xml");names.push_back("Hui Yu.xml");names.push_back("Jeffrey Parsons.xml");names.push_back("Jianping Wang.xml");names.push_back("Jie Tang.xml");names.push_back("Jie Yu.xml");names.push_back("Jim Gray.xml");names.push_back("Jing Zhang.xml");names.push_back("Ji Zhang.xml");names.push_back("John Collins.xml");names.push_back("John F. McDonald.xml");names.push_back("John Hale.xml");names.push_back("Jose M. Garcia.xml");names.push_back("Juan Carlos Lopez.xml");names.push_back("Kai Tang.xml");names.push_back("Kai Zhang.xml");names.push_back("Ke Chen.xml");names.push_back("Keith Edwards.xml");names.push_back("Koichi Furukawa.xml");names.push_back("Kuo Zhang.xml");names.push_back("Lei Chen.xml");names.push_back("Lei Fang.xml");names.push_back("Lei Jin.xml");names.push_back("Lei Wang.xml");names.push_back("Li Shen.xml");names.push_back("Lu Liu.xml");names.push_back("Manuel Silva.xml");names.push_back("Mark Davis.xml");names.push_back("Michael Lang.xml");names.push_back("Michael Siegel.xml");names.push_back("Michael Smith.xml");names.push_back("Michael Wagner.xml");names.push_back("Ning Zhang.xml");names.push_back("Paul Brown.xml");names.push_back("Paul Wang.xml");names.push_back("Peter Phillips.xml");names.push_back("Philip J. Smith.xml");names.push_back("Ping Zhou.xml");names.push_back("Qiang shen.xml");names.push_back("Rafael Alonso.xml");names.push_back("Rakesh Kumar.xml");names.push_back("Richard Taylor.xml");names.push_back("Robert Allen.xml");names.push_back("Robert Schreiber.xml");names.push_back("Sanjay Jain.xml");names.push_back("Satoshi Kobayashi.xml");names.push_back("Shu lin.xml");names.push_back("Steve King.xml");names.push_back("Thomas D. Taylor.xml");names.push_back("Thomas Hermann.xml");names.push_back("Thomas Meyer.xml");names.push_back("Thomas Tran.xml");names.push_back("Thomas Wolf.xml");names.push_back("Thomas Zimmermann.xml");names.push_back("Wei Xu.xml");names.push_back("Wen Gao.xml");names.push_back("William H. Hsu.xml");names.push_back("Xiaoming Wang.xml");names.push_back("Xiaoyan Li.xml");names.push_back("Yang Wang.xml");names.push_back("Yang Yu.xml");names.push_back("Yan Tang.xml");names.push_back("Yi Deng.xml");names.push_back("Yong Chen.xml");names.push_back("Yoshio Tanaka.xml");names.push_back("Young Park.xml");names.push_back("Yue Zhao.xml");names.push_back("Yun Wang.xml");names.push_back("Yu Zhang.xml");
// names.push_back("Bing Liu.xml");names.push_back("David E. Goldberg.xml");names.push_back("Gang Chen.xml");names.push_back("Hao Wang.xml");names.push_back("Jing Zhang.xml");names.push_back("Lei Chen.xml");names.push_back("Lei Wang.xml");names.push_back("Wen Gao.xml");names.push_back("Yang Wang.xml");names.push_back("Yu Zhang.xml");
}

#else
void getFiles(string path, vector<string> &names) {
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	int count = 0;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			// delete . and ..
			if (count++>1)
				names.push_back(fileinfo.name);
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
#endif

void delNumber(string &orgin) {
	const char* chs = orgin.c_str();
	char * newchs = new char[orgin.length() + 1];
	int leng = orgin.length();
	int j = 0;
	for (int i = 0; i<leng; i++) {
		while (chs[i] <= '9'&&chs[i] >= '0') i++;
		newchs[j++] = chs[i];
	}
	newchs[j] = 0;
	orgin = string(newchs);
	delete[] newchs;
}

int _uniFind(int * arr, int i) {
	if (arr[i] == i)
		return i;
	return arr[i] = _uniFind(arr, arr[i]);
}


void uniFind(int * arr, int len) {
	for (int i = 0; i < len; i++) {
		_uniFind(arr, i);
	}
}
