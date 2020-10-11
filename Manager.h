#ifndef _manager_head
#define _manager_head
#include"tools.h"
#include<queue>

class NonLeafAuthor;
class Paper;
class node;
class Author;
class Affiliation;
class Author;
class Venue;
class Manager;
class AuthorManager;
class PaperManager;
class KeywordManager;
class VenueManager;
class Manager {

};

typedef unsigned int AUTHOR_NUMBER_TYPE ;

struct vpair {
	int vi;
	int vj;
	float score;
	bool operator <(const vpair &) const;
	vpair(int i,int j, float s) {
		vi = i;
		vj = j;
		score = s;
	}
	vpair() {
		vi = -1;
		vj = -1;
		score = -1;
	}
};
typedef struct vpair spair;


typedef struct {
	Author *ai, *aj;
	float simi;
}aps, apair;

struct author_pair_sim {
	int ai;
	int aj;
	float score;
	bool operator <(const author_pair_sim &c) const {
		if (score<c.score)
			return false;
		if (score>c.score)
			return true;
		return false;
	}
	author_pair_sim(int i, int j, float s) {
		ai = i;
		aj = j;
		score = s;
	}
	author_pair_sim() {
		ai = -1;
		aj = -1;
		score = -1;
	}
};

struct AuthorForestLayer{
	//NonLeafAuthor ** authors;
	//int authorsize;
	author_pair_sim* apairs;
	int apairLen;
	vector<NonLeafAuthor *> authors;

};

class AuthorManager :public  Manager {
private:
	AUTHOR_NUMBER_TYPE AID;

	vector<string> fnames;
	float logs[1024*8];
	float asum, vsum, tsum;
	int namesize;
	vector<Author*>* leafAuthorVectors;
	vector<AuthorForestLayer>* author_forests;//each name corresponding to a forest. each layer of the forests are stored by list.
	AuthorForestLayer *outputLayers; // output after clustering_single
	double MINSIM, MAXSIM;
	//map<int,ofstream> s2fout;
	int *paperNumber;
	bool outputclusters;// if output cluster nor not
	double Thres1;

	vector<int> min_author_nums;
	float calSimSingle(NonLeafAuthor *, NonLeafAuthor *, double);
	float calSim(NonLeafAuthor *, NonLeafAuthor *, double);
	void update(queue<int> & que, bool * flags, NonLeafAuthor *);
	bool isSim(Author *ai, Author *aj);

public:
	int getMinAuthorNums(int nid);
	void calInitAuthorNumbers();

	//debug
	void outputInitAuthorNumber(string & fpath);
	void initSim();
	int getInitAuthorNumber(int id);
	void outputAllCluster(string path);

	void init();
	void init(bool);
	void init(double, double);
	void setThres(double);
	void clear();
	AuthorManager();
	~AuthorManager();
	Author* addAuthor(const string &name, Paper * pa);
	bool outpuCoauthorSize(string);
	void getcluster(list<Author *>*, const int*, vector<vector<Author *>*> &);
	void createAtom();
	void setAlltoOld();
	void calRs();
	void iterate();
	void clusterSingle();
	void clusterSingle(int);

	int currentSize();
	int initSize();
	AuthorForestLayer* getClusters(string name);
	void outputCluster(ofstream&, int nid);

	// for incremental
public:
	void set1stLayerNbrsUpdated();
	void inc_iterate();
	void inc_createAtom();
};


class KeywordManager : public Manager {
private:
	//temp
	map<int,string> id2name;


	double MINSIM;
	set<string> stopwords, stopwords2;
	map<string, int> *kws2id;
	string *names;
	int unseenid;
	//store the similairty
	int kid;
	int ksize;
	spair * pairsimi;
	int *indexs;


public:
	int *sizes;

private:	
	void readstopwords(const string&, const string&);
	void readsimi(const string &, vector<spair>&);
	void readfile(const string &);

public:
	int getID(string);
	void arraylize();
	void init(double, const string&, const string&, const string&, const string&);
	int getSize(string);
	void addsimi(map<int, float> &, map<int, float> &);
	string getName(int id);
	KeywordManager();
	~KeywordManager();
	void clear();
	int size();
};

class PaperManager : public Manager {
public:
	map<string, int> title2id;
	map<int, Paper*> id2paper;
	vector<string> titles;
	unsigned int pid;
	static int index;
	vector<Paper*> allpaper;
public:
	void clear();
	~PaperManager();
	void init(const string & fpath);
	void initMAG(const string & fpath);
	void addPaper(Paper * pa);

private:
	int MAGID;
	void readIEEE(const string &);
	void readCitation(const string &);
	void readHomepage(const string &);
	void readMAG(const string &, const string &);
	void readfile(const string &);
};

class VenueManager : public Manager {
private:
	//ofstream *fmiss;
	map<string, int> *vs2id;
	map<int, string> id2name;
	//Venue ** ves;
	vpair * pairsimi;
	int vsize;
	int maxid;
	void readfile(string &);
	void readsimi(string &, vector<vpair>&);
	int *indexs;
	int* sizes;
	int unseenid;
	double VMINSIM;


public:
	//set<string>* missvenue;

	void clear();
	void fileClose();
	int getID(string);
	void init(double, string &, string &);
	void init(string &, string &);
	VenueManager();
	~VenueManager();
	//Venue* getVenue(string name, Paper * pa);
	int size();
	int getsize(int);
	//void addsimi(map<int, float>&);
	void addsimi(map<int, float>&, map<int, float>&);
	string getname(int id);
	bool isSim(int id1, int id2);
};

class NameManager {
private:
	//ofstream *fmiss;
	//scale2 for maxnum
	double scale, scale2;
	map<string, int> name2id;
	vector<int> id2size;
	vector<float> id2amr;
	vector<string> id2name;
	int id;
	bool *flags;
	set<string> chnames;
	vector<bool> id2cn;
	void readfile(string &, string &);

public:
	vector<set<int>> id_2_coauthor_ids;


	void fileClose();
	NameManager();
	~NameManager();
	void estimateAuthorNumber(AuthorManager &am);
	void addCoauthor(Paper*);
	void init(string &, string &);
	int getNameSize();
	int getNameId(string);
	float getAmb(int);
	bool isCn(int);
	string getName(int);
	void output();
	void combineAmr();
	int size();
	void clear();
};

class AffiliationManager {
public:
	void init(string &);
	int getcount(string&);
private:
	map<string, int> aff2count;
};

#endif