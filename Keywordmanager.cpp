#include"node.h"
#define DEFAULT_T_MIN_SIM 0.78
#define MINI_PAPER_SIZE 10

#define Ksimi_threshold2 0.65
#define MINI_PAPER_SIZE2 300

extern NameManager nm;
extern AuthorManager am;
extern VenueManager vm;
typedef vpair spair;

KeywordManager::KeywordManager() {
	MINSIM = DEFAULT_T_MIN_SIM;
	names = NULL;
	kid = 0;
}

void KeywordManager::init(double TThres, const string & stopwordPath, const string & stopword2Path, const string & map_path,const string &fpath) {
	MINSIM = TThres;
	readstopwords(stopwordPath, stopword2Path);
	kws2id = new map<string, int>();

	readfile(map_path);


	arraylize();

	vector<spair> kps;
	readsimi(fpath, kps);;

	//adding self simi
	for (int i = 0; i<ksize; i++) {
		spair p1;
		p1.vi = i;
		p1.vj = i;
		p1.score = 1.0;
		kps.push_back(p1);
	}

	// vector 2 array and sort by id
	pairsimi = new spair[kps.size()];
	for (int i = 0; i<kps.size(); i++)
		pairsimi[i] = kps[i];
	//cout<<pairsimi<<endl;
	sort(pairsimi, pairsimi + kps.size());
	// record the begin index of pairsimi
	indexs = new int[ksize];
	int begin = 0;
	for (int index = 0; index<ksize; index++) {
		for (; begin<kps.size(); begin++)
			if (pairsimi[begin].vi>index) {
				indexs[index] = begin;
				break;
			}
	}

	indexs[ksize - 1] = kps.size();

	//normlize for each keywod=rd
	begin = 0;
	for (int index = 0; index<ksize; index++) {
		float sum = 0;
		for (int i = begin; i<indexs[index]; i++)
			sum += pairsimi[i].score;
		for (int i = begin; i<indexs[index]; i++)
			pairsimi[i].score /= sum;
		begin = indexs[index];
	}

	int max = 0;
	int maxindex = -1;
	for (int i = 1; i<ksize; i++) {
		if (indexs[i] - indexs[i - 1]>max) {
			max = indexs[i] - indexs[i - 1];
			maxindex = i;
		}
	}
	cout << maxindex << endl;
	cout << "max kwd..\t" << names[maxindex] << "\t" << max << endl;

	cout << "kwd size\t" << ksize << "\tkwd simi pair size\t" << kps.size() << endl;
	cout << "initial km ok\r";

}

void KeywordManager::readfile(const string & fpath) {
	ifstream fin(fpath.c_str());
	if (!fin) {
		cout << fpath<<" connot find" << endl;
		exit(1);
	}
	string temp;
	set<int> ids;
	int maxid = -1;
	while (!fin.eof()) {
		getline(fin, temp);
		if (temp.length()<2)
			continue;
		vector<string> ss;
		split(ss, temp, '\t');
		string name = norm(ss[1]);
		int id = atoi(ss[0].c_str());
		kws2id->insert(map<string, int>::value_type(name, id));
		id2name.insert(map<int, string>::value_type(id, name));
		if (ss.size() <= 1) {
			cout << temp << "\tERROR id2kw.txt" << endl;
			exit(1);
		}
		ids.insert(id);
		if (id >= maxid)
			maxid = id;
	}
	for (int i = 0; i<maxid; i++) {
		if (ids.find(i) == ids.end()) {
			string unseen = "unseen";
			unseenid = i;
			kws2id->insert(map<string, int>::value_type(unseen, i));
			id2name.insert(map<int, string>::value_type(i, unseen));
			if (i >= maxid)
				maxid = i + 1;
			ids.insert(i);
			break;
		}
	}
	sizes = new int[maxid + 1];
	memset(sizes, 0, sizeof(int)*(maxid + 1));
	ksize = ids.size();
	fin.close();
}

void KeywordManager::readsimi(const string & fpath, vector<spair>& sps) {
	ifstream fin(fpath.c_str());

	//DEBUG
	cout << "read kwd similarity from\t" << fpath << endl;
	if (!fin) {
		cout << fpath << "not found" << endl;
		return;
	}

	string temp;
	int count = 0;
	while (!fin.eof()) {
		getline(fin, temp);
		if (temp.length()<2)
			continue;

		vector<string> ss;
		split(ss, temp, '\t');

		float simi = atof(ss[2].c_str());

		if (simi<MINSIM)
			continue;

		map<string, int>::iterator ite = kws2id->find(ss[0]);

		if (stopwords2.find(ss[0]) != stopwords2.end() || ite == kws2id->end()/* || sizes[ite->second]<MINI_PAPER_SIZE*/)
			continue;

		int kid1 = ite->second;
		ite = kws2id->find(ss[1]);
		if (stopwords2.find(ss[1]) != stopwords2.end() || ite == kws2id->end()/* || sizes[ite->second]<MINI_PAPER_SIZE*/)
			continue;

		int kid2 = ite->second;
		spair p1;
		p1.vi = kid1;
		p1.vj = kid2;
		p1.score = simi;
		sps.push_back(p1);
		p1.vi = kid2;
		p1.vj = kid1;
		sps.push_back(p1);
	}
	fin.close();
	//fout1.close();
	cout << "similar keyword pair size:\t" << sps.size() << "------------------" << endl;
}


int KeywordManager::getID(string name) {
	if (stopwords.find(name) != stopwords.end()) {
		return -1;
	}
	map<string, int>::iterator ite = kws2id->find(name);
	if (ite == kws2id->end()) {
		return -1;
	}
	sizes[ite->second]++;
	return ite->second;
}

int KeywordManager::getSize(string name) {
	map<string, int>::iterator ite = kws2id->find(name);
	if (ite == kws2id->end()) {
		return -1;
	}
	return sizes[ite->second];

}

void KeywordManager::arraylize() {

	names = new string[id2name.size()];
	for (int i = 0; i<id2name.size(); i++) {
		names[i] = id2name[i];
	}

	cout << "id2names size\t" << id2name.size() << endl;
	map<int,string>().swap(id2name);

}

void KeywordManager::readstopwords(const string & stopwordPath, const string & stopword2Path) {
	ifstream fin(stopwordPath);
	string tmp;
	while (!fin.eof()) {
		getline(fin, tmp);
		if (tmp.length()<1)
			continue;
		stopwords.insert(tmp);
	}
	fin.close();

	ifstream fin1(stopword2Path);
	while (!fin1.eof()) {
		getline(fin1, tmp);
		if (tmp.length()<1)
			continue;
		stopwords2.insert(tmp);
	}
	fin1.close();

}

void KeywordManager::clear() {
	set<string>().swap(stopwords);
	set<string>().swap(stopwords2);


	if (kws2id != NULL) {
		delete kws2id;
		kws2id = NULL;
	}
	cout << "\t\t\t\t km kws2id clear ok\r";

	//i don't known why it break down!
	//if(names!=NULL){
	//	delete names;
	//	names=NULL;
	//}

	cout << "\t\t\t\t km names clear ok\r";

	if (pairsimi != NULL) {
		free(pairsimi);
		pairsimi = NULL;
	}

	cout << "\t\t\t\t km pairsim clear ok\r";



	if (sizes != NULL) {
		free(sizes);
		sizes == NULL;
	}

	cout << "\t\t\t\t km SIZE clear ok\r";

	if (id2name.size()>0)
		map<int,string>().swap(id2name);

	MINSIM = DEFAULT_T_MIN_SIM;
	names = NULL;
	kid = 0;

}
KeywordManager::~KeywordManager() {
	clear();
}

int KeywordManager::size() {
	return kws2id->size();
}


string KeywordManager::getName(int id) {
	if (names == NULL)
		return id2name[id];
	else
		return names[id];
}


void KeywordManager::addsimi(map<int, float> &t2s, map<int, float> &t2s_simi) {

	for (map<int, float>::iterator ite = t2s.begin(); ite != t2s.end(); ite++) {
		int kid = ite->first;
		int i = 0;
		if (kid>0)
			i = indexs[kid - 1];
		int count = 0;
		for (; i<indexs[kid]; i++) {
			int sid = pairsimi[i].vj;
			if (sid >= 0) {
				t2s_simi[sid] += (ite->second*pairsimi[i].score);
				count++;
			}
		}
	}
}