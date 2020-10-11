#include"node.h"
#include"tools.h"
#define threshold 10
#define Using_Alg_Simi  false
#define DEFAULT_V_MIN_SIM 0.1
#define scale 5//3.25
bool outputflag = false;
#define DEBUG false
VenueManager::VenueManager() {
	VMINSIM = DEFAULT_V_MIN_SIM;
	vs2id = NULL;
	vsize = 0;
	maxid = 0;
	//missvenue = new set<string>();
	//fmiss = NULL;
	//if (DEBUG){
	//	string fpath = DATA_DIR;
	//	fpath.append("mapid/missVenue.tsv");
	//	fmiss = new ofstream(fpath);
	//}
}

/*
* initiate the Venuemanager
-read map file
-read simi file
*/
void VenueManager::init(string &mapid, string &simipath) {
	vs2id = new map<string, int>();
	cout << "initial vm\r";
	readfile(mapid);

	vector<vpair> vps;
	readsimi(simipath, vps);
	cout << "maxid\t" << maxid << endl;
	//adding self simi
	for (int i = 0; i<maxid; i++) {
		vpair p1;
		p1.vi = i;
		p1.vj = i;
		p1.score = 1.0;
		vps.push_back(p1);
	}

	// vector 2 array and sort by id
	pairsimi = (vpair*)malloc(sizeof(vpair)*vps.size());
	memset(pairsimi, 0, sizeof(vpair)*vps.size());
	for (int i = 0; i<vps.size(); i++)
		pairsimi[i] = vps[i];
	//cout<<pairsimi<<endl;
	sort(pairsimi, pairsimi + vps.size());
	// record the begin index of pairsimi
	indexs = new int[maxid];
	int begin = 0;
	for (int index = 0; index<maxid; index++) {
		for (; begin<vps.size(); begin++)
			if (pairsimi[begin].vi>index) {
				indexs[index] = begin;
				break;
			}
	}

	sizes = (int*)malloc(sizeof(int)*maxid);
	memset(sizes, 0, sizeof(int)*maxid);


	indexs[maxid - 1] = vps.size();

	//normlize for each keywod=rd
	begin = 0;
	for (int index = 0; index<maxid; index++) {
		float sum = 0;
		for (int i = begin; i<indexs[index]; i++)
			sum += pairsimi[i].score;

		for (int i = begin; i<indexs[index]; i++)
			pairsimi[i].score /= sum;
		begin = indexs[index];
	}
	int max = 0;
	int maxindex = -1;
	for (int i = 1; i<maxid; i++) {
		if (indexs[i] - indexs[i - 1]>max) {
			max = indexs[i] - indexs[i - 1];
			maxindex = i;
		}
	}
	cout << "max venue\t" << id2name.find(maxindex)->second << "\t" << max << endl;
	cout << "venue size" << maxid << "\tvenue simi pair size\t" << vps.size() << endl;
}

void VenueManager::init(double VThres, string &mapid, string &simipath) {
	VMINSIM = scale * VThres;
	//cout<<VMINSIM<<endl;
	init(mapid, simipath);
}

int VenueManager::getID(string name) {
	if (name.size()<2)
		return -1;
	string nname = norm(name);
	if (nname == "corr")
		return -1;
	map<string, int>::iterator ite = vs2id->find(nname);
	if (ite == vs2id->end()) {
		return unseenid;
	}
	sizes[ite->second]++;
	return ite->second;
}

void VenueManager::fileClose() {
	//if(fmiss!=NULL)
	//	fmiss->close();
}

bool vpair::operator< (const vpair &c) const {
	//cout<<this<<endl;
	//cout<<&c<<endl;
	if (vi<c.vi)
		return true;
	else if (vi>c.vi)
		return false;

	if (score<c.score)
		return false;
	if (score>c.score)
		return true;
	return false;
}

VenueManager::~VenueManager() {
	clear();
}

int VenueManager::size() {
	return vsize;
}

void VenueManager::clear() {
	if (vs2id != NULL) {
		delete vs2id;
		vs2id = NULL;
	}
	map<int, string>().swap(id2name);

	if (pairsimi != NULL) {
		delete pairsimi;
		pairsimi = NULL;
	}

	cout << "\t\t\t\t\t\t VenueManager pairsimi cleared\r";

	if (indexs != NULL) {
		delete indexs;
		indexs = NULL;
	}

	cout << "\t\t\t\t\t\t VenueManager indexs cleared\r";


	if (sizes != NULL) {
		free(sizes);
		sizes = NULL;
	}
	cout << "\t\t\t\t\t\t VenueManager sizes cleared\r";

	VMINSIM = DEFAULT_V_MIN_SIM;
	vs2id = NULL;
	vsize = 0;
	maxid = 0;
	//fmiss = NULL;

}

/*
* read the map file
*/
void VenueManager::readfile(string &fpath) {
	ifstream fin(fpath);
	if (!fin) {
		cout << "\"G:\\GraduateThesis\\data\\mapid\\id2venue.tsv\" connot find" << endl;
		exit(1);
	}
	string temp;
	set<int> ids;
	while (!fin.eof()) {
		getline(fin, temp);
		if (temp.length()<2)
			continue;
		vector<string> ss;
		split(ss, temp, '\t');
		string name = norm(ss[1]);
		int id = atoi(ss[0].c_str());
		vs2id->insert(map<string, int>::value_type(name, id));
		id2name.insert(map<int, string>::value_type(id, name));
		if (ss.size() <= 1) {
			cout << temp << endl;
			system("pause");
		}
		ids.insert(id);
		if (id >= maxid)
			maxid = id + 1;
	}
	for (int i = 0; i<maxid; i++) {
		if (ids.find(i) == ids.end()) {
			string unseen = "unseen";
			unseenid = i;
			vs2id->insert(map<string, int>::value_type(unseen, i));
			id2name.insert(map<int, string>::value_type(i, unseen));
			if (i >= maxid)
				maxid = i + 1;
			ids.insert(i);
			break;
		}
	}


	vsize = ids.size();
	fin.close();
}
/*
* read the similarity between venues
*/
void VenueManager::readsimi(string &fpath, vector<vpair> &vvp) {
	ifstream fin(fpath);
	if (!fin) {
		cout << fpath << "not found" << endl;
		return;
	}
	//cout<<"VThreshold =\t"<<VMINSIM<<endl;
	string temp;
	while (!fin.eof()) {
		getline(fin, temp);
		if (temp.length()<2)
			continue;
		vector<string> ss;
		split(ss, temp, '\t');
		int vid1 = atoi(ss[0].c_str());
		int vid2 = atoi(ss[1].c_str());
		if (id2name.find(vid1) == id2name.end() || id2name.find(vid2) == id2name.end())
			continue;
		float simi = scale * atof(ss[2].c_str());

		// a threshold. only consider the similarity bigger than MINSIM
		if (simi<VMINSIM)
			continue;

		vpair p1;
		p1.vi = vid1;
		p1.vj = vid2;
		p1.score = simi;
		if (vid1<maxid&&vid2<maxid)
			vvp.push_back(p1);

		p1.vi = vid2;
		p1.vj = vid1;
		vvp.push_back(p1);
		if (vid1<maxid&&vid2<maxid)
			vvp.push_back(p1);
	}
	fin.close();
}

int VenueManager::getsize(int id) {
	if (id == unseenid)
		return 5;
	return sizes[id];
}

void VenueManager::addsimi(map<int, float> &v2s, map<int, float> &v2s_simi) {
	for (map<int, float>::iterator ite = v2s.begin(); ite != v2s.end(); ite++) {
		int vid = ite->first;
		int count = 0;
		int i = 0;
		if (vid>0)
			i = indexs[vid - 1];
		for (; i<indexs[vid]; i++) {
			int vjid = pairsimi[i].vj;
			if (vjid >-1) {
				count += 1;
				v2s_simi[vjid] += (ite->second*pairsimi[i].score);// / (1 + (vt->papers.size()));
				count++;
			}
		}
	}



}

string VenueManager::getname(int id) {
	return id2name.find(id)->second;
}

bool VenueManager::isSim(int id1, int id2) {
	if (id1 == -1 || id2 == -1)
		return false;
	int i = 0;
	if (id1>0)
		i = indexs[id1 - 1];
	for (; i<indexs[id1]; i++)
		if (id2 == pairsimi[i].vj)
			return true;
	
	return 0;
}