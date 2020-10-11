#include"Manager.h"
#include"node.h"
#define DEBUG false
NameManager::NameManager() {
	id = 0;
	//missname = new set<string>();
}
void NameManager::clear() {
	name2id.clear();
	map<string, int>().swap(name2id);
	id2size.clear();
	vector<int>().swap(id2size);
	id2amr.clear();
	vector<float>().swap(id2amr);
	id2name.clear();
	vector<string>().swap(id2name);
	set<string>().swap(chnames);
	vector<bool>().swap(id2cn);
	id = 0;
}
NameManager::~NameManager() {
	//clear();
}

void NameManager::addCoauthor(Paper *pa) {
	for (int i = 0; i < pa->authorsize; i++) {
		int id_i = pa->authors[i]->nid;
		for (int j = 0; j < pa->authorsize; j++) {
			if (i == j)
				continue;
			id_2_coauthor_ids[id_i].insert(pa->authors[j]->nid);
		}
	}
}


void NameManager::init(string &chienseNamepath, string &fpath) {
	scale = 0.9;//for aminer 0.3, ACM 0.2
	scale2 = 0.7;//default 0.2
	readfile(chienseNamepath, fpath);
}

void NameManager::estimateAuthorNumber(AuthorManager & am) {
 	time_t begin, end;
 	int index =0;
	int namenumber = id;
	int *maxnum = new int[namenumber];
	int *minnum = new int[namenumber];
	for (int i = 0; i < id; i++) {
		if (id2cn[i]) {
			maxnum[i] = am.getInitAuthorNumber(i)*scale2;
		}
		minnum[i] = am.getMinAuthorNums(i);
		if (maxnum[i] < minnum[i])
			maxnum[i] = minnum[i];
	}
	
	//initiate all numbers to  minnum[i]
	float *authorNumber = new float[namenumber];
	for (int i = 0; i < namenumber; i++)
		authorNumber[i] = minnum[i];

	//F is the distribution of First name and L is the distribution of the last name
	map<string, int> F, L;
	string * fs = new string[namenumber];
	string * ls = new string[namenumber];
	for (int i = 0; i < namenumber; i++) {
		if (id2cn[i]) {
			string name = id2name[i];
			vector<string> fl;
			split(fl, name, ' ');
			string f = fl[0];
			string l = fl[fl.size() - 1];
			fs[i] = f;
			ls[i] = l;
			F[f] = 0;
			L[l] = 0;
		}
	}

	int ite = 0;
	while (ite++ < 10) {
		float allauthorNumber = 0;

		//clear F, L
		for (map<string, int>::iterator fite = F.begin(); fite != F.end(); fite++)
			fite->second = 0;
		for (map<string, int>::iterator lite = F.begin(); lite != F.end(); lite++)
			lite->second = 0;

		//update F and L
		for (int i = 0; i < namenumber; i++) {
			if (id2cn[i]) {
				allauthorNumber += authorNumber[i];
				string f = fs[i];
				string l = ls[i];
				F[f] += authorNumber[i];
				L[l] += authorNumber[i];
			}
		}
		//cout << "wei\t" << F["wei"] << endl;
		//cout << "wang\t" << L["wang"] << endl;
		//cout << "allauthorNumber\t" << allauthorNumber << endl;
		//update authorNumber
		for (int i = 0; i < namenumber; i++) {
			if (id2cn[i]) {
				string f = fs[i];
				string l = ls[i];
				authorNumber[i] = F[f]/allauthorNumber*L[l];
				if (authorNumber[i] <  minnum[i])
					authorNumber[i] = minnum[i];
				if (authorNumber[i] > maxnum[i])
					authorNumber[i] = maxnum[i];
			}
		}
	}
	
	ofstream fname("./log.txt");
	for (int i = 0; i < namenumber; i++) {
		if (id2cn[i]) {
			//cout << id2name[i] << "\t" << id2amr[i] << "\t" << authorNumber[i] << "\t"<< am.getAuthorNumber(i)<<endl;
			id2amr[i] = authorNumber[i] * scale;
			fname << id2name[i] << "\t" << id2amr[i] << "\t" << authorNumber[i] << "\t" << maxnum[i] << "\t" << minnum[i] << endl;

		}
		else {
			id2amr[i] = minnum[i];
			fname << id2name[i] << "\t" << minnum[i] << endl;
		}

	}
	fname.close();

	delete[] fs;
	delete[] ls;
	delete [] maxnum;
	delete [] authorNumber;
}

void NameManager::combineAmr() {
	string temp;

	string fpath = DATA_DIR;
	fpath.append("ambiguity/c_name_am.tsv");
	ifstream pafile(fpath);
	while (!pafile.eof()) {
		getline(pafile, temp);
		if (temp.length()<1)
			break;
		vector<string> ss;
		split(ss, temp, '\t');
		string name = ss[0];
		name2id.insert(map<string, int>::value_type(name, id));
		double amr = atof(ss[1].c_str())*scale;
		if (amr<1.0)
			amr = 1.0;
		id2amr.push_back(amr);
		id2name.push_back(name);
		id++;
	}
	pafile.close();


	flags = (bool*)malloc(sizeof(bool)*id);
	memset(flags, 0, sizeof(bool)*id);
}

void NameManager::readfile(string &fpath, string & chineseNamesPath) {
	string temp;
	ifstream fchinesename(fpath);
	while (!fchinesename.eof()) {
		getline(fchinesename, temp);
		chnames.insert(norm(temp));
	}
	fchinesename.close();

	ifstream pafile(fpath);
	while (!pafile.eof()) {
		getline(pafile, temp);
		if (temp.length()<1)
			break;
		vector<string> ss;
		split(ss, temp, '\t');
		string name = ss[0];
		name2id.insert(map<string, int>::value_type(name, id));
		double amr = atof(ss[1].c_str())*scale;
		bool chiflag = false;
		if (chnames.find(name) == chnames.end()) {
			id2cn.push_back(false);
		}
		else {
			id2cn.push_back(true);
			chiflag = true;
		}

		if (chiflag&&name[1] == ' ')
			amr *= 3;
		if (amr<1)
			amr = 1;

		id2amr.push_back(amr);
		id2name.push_back(name);
		id_2_coauthor_ids.push_back(set<int>());
		id++;
	}
	pafile.close();
	cout << "name size with ambiguity scores\t" << id << endl;
}

int NameManager::getNameId(string name) {
	map<string, int>::iterator mite = name2id.find(name);
	if (mite == name2id.end()) {
		name2id.insert(map<string, int>::value_type(name, id));
		double amr = 2 * scale;
		bool chiflag = false;
		if (chnames.find(name) == chnames.end()) {
			id2cn.push_back(false);
		}
		else {
			id2cn.push_back(true);
			chiflag = true;
		}

		if (chiflag&&name[1] == ' ')
			amr *= 3;
		id2amr.push_back(amr);
		id2name.push_back(name);
		id_2_coauthor_ids.push_back(set<int>());
		return id++;
	}
	else {
		return mite->second;
	}
}

void NameManager::fileClose() {
	//if(fmiss!=NULL)
	//	fmiss->close();
}

float NameManager::getAmb(int nid) {
	//debug 10.0
	//cout<<"----------------------DEBUG MODEL SETTING ALL AUTHOR NUMBERS TO 10.0-------------------------"<<endl;
	// return 10.0;

	if (nid<id) {
		double w1 = id2amr[nid];
		return w1;
	}
	else {
		cout << "not found\t" << nid << endl;
		system("pause");
		return -1;
	}
}

bool NameManager::isCn(int nid) {
	if (nid<id)
		return id2cn[nid];
	else {
		cout << "not found\t" << nid << endl;
		system("pause");
		return -1;
	}
}

string NameManager::getName(int nid) {
	if (nid<id)
		return id2name[nid];
	else {
		cout << "not found\t" << nid << endl;
		system("pause");
		return "";
	}
}

int NameManager::getNameSize() {
	return id;
}

void NameManager::output() {
	string fpath = DATA_DIR;
	fpath.append("ambiguity/c_name_am.tsv");
	ofstream cnames(fpath);
	for (int i = 0; i<id; i++) {
		if (flags[i])
			cnames << id2name[i] << "\t" << id2amr[i] << endl;
	}
	cnames.close();
}

int NameManager::size() {
	return name2id.size();
}
