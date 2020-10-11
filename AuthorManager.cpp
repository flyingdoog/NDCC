#include"node.h"
#include"Evaluation.h"
#include<queue>
#define USESQRT false
extern KeywordManager km;
extern PaperManager pm;
extern AuthorManager am;
extern VenueManager vm;
extern Evaluation elv;
extern NameManager nm;
extern AffiliationManager afm;
extern bool outputflag;
bool vsimioutputflag;
#define DEFAULT_MINSIM 1e-7
#define DEFAULT_MAXSIM 1e-3
#define DEFAULT_THRES1 0.25
#define DEFAULT_THRES2 0.1
#define NAMESIZE 4100000
#define max(x,y) (x)>(y)? (x):(y)
int nrule1 = 0, nrule2 = 0, nrule3 = 0, nrule4 = 0, nrule5 = 0, nrule6 = 0, nrule7 = 0, nrule8 = 0, nrule9 = 0;

/*
* case 0 : top k; k = size-amr
* case 1 : mean*amr
* case 2 : top 1
* case 3 : gap.
*/
int thresholdCase = 0;
enum ClusterCase { CASEMCL, CASEHIE, CASEAP };
ClusterCase clusterCase = CASEHIE;


AuthorManager::AuthorManager() {
	MINSIM = DEFAULT_MINSIM;
	MAXSIM = DEFAULT_MAXSIM;
	Thres1 = DEFAULT_THRES1;
	outputclusters = false;
	AID = 0;
	for (int i = 0; i<1024*8; i++)
		logs[i] = log((double)(i + 1)) + 1;
	
}

void AuthorManager::init() {

	namesize = nm.getNameSize();
	//getFiles(elv.elvationDir,fnames);
	int reservesize = max(NAMESIZE, 5 * namesize);
	
	author_forests = new vector<AuthorForestLayer>[reservesize];
	outputLayers = new AuthorForestLayer[reservesize];
	leafAuthorVectors = new vector<Author*>[reservesize];
	paperNumber = new int[reservesize];
	memset(paperNumber, 0, sizeof(int)*reservesize);
	MINSIM = DEFAULT_MINSIM;
	MAXSIM = DEFAULT_MAXSIM;
	Thres1 = DEFAULT_THRES1;
	outputclusters = false;
}

void AuthorManager::clear() {
	for (int i = 0; i < namesize; i++) {
		for (auto author : leafAuthorVectors[i]) {
			if (author != NULL)
				delete author;
		}
		vector<AuthorForestLayer> &author_forest = author_forests[i];
		for (auto &layer : author_forest) {
			delete[] layer.apairs;
			vector<NonLeafAuthor*>().swap(layer.authors);
			layer.apairs = NULL;
			layer.apairLen = -1;
		}
		author_forest.clear();
		leafAuthorVectors[i].clear();
	}
	delete [] leafAuthorVectors;
	delete [] author_forests;
	cout << "\t\t\t\t\t am author_vector_lists clear ok\r";
	leafAuthorVectors = NULL;
	author_forests = NULL;
	delete [] paperNumber;
	paperNumber = NULL;
	cout << "\t\t\t\t\t am paperNumber clear ok\r";
	MINSIM = DEFAULT_MINSIM;
	MAXSIM = DEFAULT_MAXSIM;
	Thres1 = DEFAULT_THRES1;
	outputclusters = false;
	AID = 0;
}

void AuthorManager::init(double minsim, double maxsim) {
	init();
	MINSIM = minsim;
	MAXSIM = maxsim;
}

void AuthorManager::init(bool flag) {
	outputclusters = flag;
	init();
}

AuthorManager::~AuthorManager() {

}

void AuthorManager::setAlltoOld() {
	for (int nid = 0; nid < namesize; nid++) {
		if (author_forests[nid].size() == 0)
			continue;
		for (auto &layer : author_forests[nid]) {
			for (auto author : layer.authors) {
				author->updated = false;
				author->coauthor_update = false;
				author->del = false;
			}
		}
	}
}

//add author to paper
Author* AuthorManager::addAuthor(const string &name, Paper * pa) {
	int nid = nm.getNameId(name);
	if (nid == -1) {
		return NULL;
	}
	vector<Author*> & authorVector = leafAuthorVectors[nid];
	paperNumber[nid]++;
	AUTHOR_NUMBER_TYPE aid = AID++;

	Author *au = new Author(nid, aid);
	au->addPaper(pa);
	authorVector.push_back(au);

	return au;
}

int AuthorManager::currentSize() {
	int sum = 0;
	for (int ite = 0; ite<namesize; ite++)
		if (author_forests[ite].size()>0)
			sum += author_forests[ite][author_forests[ite].size() - 1].authors.size();
	return sum;
}

int AuthorManager::initSize() {
	int sum = 0;
	for (int ite = 0; ite<namesize; ite++)
		if (leafAuthorVectors[ite].size()>0)
			sum += leafAuthorVectors[ite].size();
	return sum;
}

void AuthorManager::createAtom() {
	namesize = nm.getNameSize();
	int dels = 0, total = 0, dels1 = 0;
	int max = 0;
	int assignLength = 1024*8;
	bool * assigned = new bool[assignLength];
	memset(assigned, 0, sizeof(bool) * assignLength);

	calInitAuthorNumbers();

	for (int nid = 0; nid < namesize; nid++) {
		string now_name = nm.getName(nid);
		if (leafAuthorVectors[nid].size() <= 1)
			continue;

		vector<Author*> &authorVector = leafAuthorVectors[nid];

		while (authorVector.size() > assignLength) {
			delete[] assigned;
			assignLength *= 2;
			assigned = new bool[assignLength];
		}
		//reassign
		memset(assigned, 0, sizeof(bool) * assignLength);

		if (author_forests[nid].size() == 0) {
			AuthorForestLayer layer;
			layer.apairLen = 0;
			layer.apairs = NULL;
			author_forests[nid].push_back(layer);
			//cout << "nid " << nid << "\t" << now_name << "push a new layer" << endl;
		}
		AuthorForestLayer &layer = author_forests[nid][0];

		for(int aindex1=0;aindex1<authorVector.size(); aindex1++){
			if (assigned[aindex1]) {
				continue;
			}
			assigned[aindex1] = true;// assigned

			set<int> set1;// author set of paper1
			int begin = -1;
			int end = -1;
			set<int> veset;
			int yeari = -1;


			NonLeafAuthor *non_leaf_author = new NonLeafAuthor(nid, AID++);
			non_leaf_author->merge(authorVector[aindex1]);
			layer.authors.push_back(non_leaf_author);

			Paper *paper1 = authorVector[aindex1]->paper;
			for (int i = 0; i < paper1->authorsize; i++) {
				if (paper1->authors[i]->nid != nid)
					set1.insert(paper1->authors[i]->nid);
			}
			begin = paper1->year;
			end = paper1->year;
			if (paper1->vid != -1)
				veset.insert(paper1->vid);
			yeari = paper1->year;

			for (int aindex2 = aindex1 + 1; aindex2 < authorVector.size();aindex2++) {
				if (assigned[aindex2]) {
					continue;
				}

				Paper *paper2 = authorVector[aindex2]->paper;
				Author ** authors2 = paper2->authors;
				int vid2 = paper2->vid;
				int yearj = paper2->year;

				int samecoauthorcount = 0;
				//Coauthor
				bool merged = false;

				if(!nm.isCn(nid) && min_author_nums[nid]==1)
					merged = true;
				else
					for (int i = 0; i < paper2->authorsize; i++) {
						int nidtemp = paper2->authors[i]->nid;

						if (paperNumber[nidtemp] >= 500)
							continue;

						if (set1.find(nidtemp) != set1.end()) {//find a name
							samecoauthorcount++;
							if (nm.getAmb(nidtemp) < 5)
								samecoauthorcount++;
							if (vid2 != -1 && veset.find(vid2) != veset.end() && abs(yeari - yearj) < 3
								) {//has two same name coauthor, combine them
								merged = true;
							}
							else if (samecoauthorcount == 3 || nm.getAmb(nid) < 4) {
								merged = true;
							}
						}
					}

				int count = 0;
				if (merged) {
					assigned[aindex2] = true;
					non_leaf_author->merge(authorVector[aindex2]);
					dels++;
					Author ** authors2 = (paper2)->authors;
					for (int ai = 0; ai < paper2->authorsize; ai++) {
						if (authors2[ai]->nid != nid)
							set1.insert(authors2[ai]->nid);
					}
					if (vid2 != 0)
						veset.insert(vid2);
					if (paper2->year > end)
						end = paper2->year;
					else if (paper2->year < begin)
						begin = paper2->year;
				}
			}
		}
	}
	delete[] assigned;
}

/*
*/
void AuthorManager::inc_createAtom() {
	namesize = nm.getNameSize();
	int assignLength = 1024;
	bool * assigned = new bool[assignLength];
	memset(assigned, 0, sizeof(bool) * assignLength);
	calInitAuthorNumbers();

	for (int nid = 0; nid < namesize; nid++) {
		string now_name = nm.getName(nid);
		if (leafAuthorVectors[nid].size() <= 1)
			continue;

		vector<Author*> &authorVector = leafAuthorVectors[nid];
		int splitNmb = 0;
		for (splitNmb = 0; splitNmb <authorVector.size(); splitNmb++) {
			if (authorVector[splitNmb]->updated)
				break;
		}

		if (splitNmb == authorVector.size())
			continue;

		//[0,splitNmb) old, (splitNmb, ] new

		while (authorVector.size() > assignLength) {
			delete[] assigned;
			assignLength *= 2;
			assigned = new bool[assignLength];
		}
		//reassign
		memset(assigned, 0, sizeof(bool) * assignLength);

		if (author_forests[nid].size() == 0) {
			AuthorForestLayer layer;
			layer.apairLen = 0;
			layer.apairs = NULL;
			author_forests[nid].push_back(layer);
			//cout << "nid " << nid << "\t" << now_name << "push a new layer" << endl;
		}
		AuthorForestLayer &layer = author_forests[nid][0];


		set<NonLeafAuthor*> non_leaf_authors_has_been_calculated;
		for (int aindex1 = 0; aindex1 < splitNmb; aindex1++) {
			set<int> set1;// author set of paper1
			int begin = -1;
			int end = -1;
			set<int> veset;
			int yeari = -1;
			NonLeafAuthor * author = authorVector[aindex1]->father;
			if (non_leaf_authors_has_been_calculated.find(author) == non_leaf_authors_has_been_calculated.end())
				non_leaf_authors_has_been_calculated.insert(author);
			else
				continue;
			if (author == NULL) {
				cout << "ERROR HERE author father is lost!!!!" << endl;
				exit(1);
			}
			for (auto paper : author->papers) {
				for (int i = 0; i < paper->authorsize; i++) {
					if (paper->authors[i]->nid != nid)
						set1.insert(paper->authors[i]->nid);
				}
				if (paper->vid != -1)
					veset.insert(paper->vid);
			}
			yeari = author->papers[0]->year;
			begin = author->begin;
			end = author->end;
			for (int aindex2 = splitNmb; aindex2 < authorVector.size(); aindex2++) {
				if (assigned[aindex2]) {
					continue;
				}
				Paper *paper2 = authorVector[aindex2]->paper;
				Author ** authors2 = paper2->authors;
				int vid2 = paper2->vid;
				int yearj = paper2->year;

				int samecoauthorcount = 0;
				//Coauthor
				bool merged = false;

				if(!nm.isCn(nid) && min_author_nums[nid]<=1)
					merged = true;
				else
					for (int i = 0; i < paper2->authorsize; i++) {
						int nidtemp = paper2->authors[i]->nid;

						if (paperNumber[nidtemp] >= 500)
							continue;

						if (set1.find(nidtemp) != set1.end()) {//find a name
							samecoauthorcount++;
							if (nm.getAmb(nidtemp) < 5)
								samecoauthorcount++;
							if (vid2 != -1 && veset.find(vid2) != veset.end() && abs(yeari - yearj) < 3
								) {//has two same name coauthor, combine them
								merged = true;
							}
							else if (samecoauthorcount == 3 || nm.getAmb(nid) < 4) {
								merged = true;
							}
						}
					}

				int count = 0;
				if (merged) {
					assigned[aindex2] = true;
					NonLeafAuthor temp(nid,AID++);
					temp.merge(authorVector[aindex2]);
					temp.calRs();
					author->merge(&temp);
					authorVector[aindex2]->father = author;
					Author ** authors2 = (paper2)->authors;
					for (int ai = 0; ai < paper2->authorsize; ai++) {
						if (authors2[ai]->nid != nid)
							set1.insert(authors2[ai]->nid);
					}
					if (vid2 != 0)
						veset.insert(vid2);
					if (paper2->year > end)
						end = paper2->year;
					else if (paper2->year < begin)
						begin = paper2->year;
				}
			}
		}


		for (int aindex1 = splitNmb; aindex1<authorVector.size(); aindex1++) {
			if (assigned[aindex1]) {
				continue;
			}
			assigned[aindex1] = true;// assigned

			set<int> set1;// author set of paper1
			int begin = -1;
			int end = -1;
			set<int> veset;
			int yeari = -1;

			NonLeafAuthor *non_leaf_author = new NonLeafAuthor(nid, AID++);
			non_leaf_author->merge(authorVector[aindex1]);
			layer.authors.push_back(non_leaf_author);

			Paper *paper1 = authorVector[aindex1]->paper;
			for (int i = 0; i < paper1->authorsize; i++) {
				if (paper1->authors[i]->nid != nid)
					set1.insert(paper1->authors[i]->nid);
			}
			begin = paper1->year;
			end = paper1->year;
			if (paper1->vid != -1)
				veset.insert(paper1->vid);
			yeari = paper1->year;

			for (int aindex2 = aindex1 + 1; aindex2 < authorVector.size(); aindex2++) {
				if (assigned[aindex2]) {
					continue;
				}

				Paper *paper2 = authorVector[aindex2]->paper;
				Author ** authors2 = paper2->authors;
				int vid2 = paper2->vid;
				int yearj = paper2->year;

				int samecoauthorcount = 0;
				//Coauthor
				bool merged = false;

				if(!nm.isCn(nid) && min_author_nums[nid]<=1)
					merged = true;
				else				
					for (int i = 0; i < paper2->authorsize; i++) {
						int nidtemp = paper2->authors[i]->nid;

						if (paperNumber[nidtemp] >= 500)
							continue;

						if (set1.find(nidtemp) != set1.end()) {//find a name
							samecoauthorcount++;
							if (nm.getAmb(nidtemp) < 5)
								samecoauthorcount++;
							if (vid2 != -1 && veset.find(vid2) != veset.end() && abs(yeari - yearj) < 3
								) {//has two same name coauthor, combine them
								merged = true;
							}
							else if (samecoauthorcount == 3 || nm.getAmb(nid) < 4) {
								merged = true;
							}
						}
					}

				int count = 0;
				if (merged) {
					assigned[aindex2] = true;
					non_leaf_author->merge(authorVector[aindex2]);
					Author ** authors2 = (paper2)->authors;
					for (int ai = 0; ai < paper2->authorsize; ai++) {
						if (authors2[ai]->nid != nid)
							set1.insert(authors2[ai]->nid);
					}
					if (vid2 != 0)
						veset.insert(vid2);
					if (paper2->year > end)
						end = paper2->year;
					else if (paper2->year < begin)
						begin = paper2->year;
				}
				non_leaf_author->calRs();
			}
		}
	}
	delete[] assigned;

	//delete the author node
}


void AuthorManager::set1stLayerNbrsUpdated() {
	int count = 0;
	for (int nid = 0; nid < nm.getNameSize(); nid++) {
		if (author_forests[nid].size() == 0)
			continue;
		AuthorForestLayer &layer = author_forests[nid][0];
		for (auto author : layer.authors) {
			if(author->updated)
				for (map<NonLeafAuthor*, two_int>::iterator aite = author->a2s->begin(); aite != author->a2s->end(); aite++) {
					if (nm.getAmb(aite->first->nid) < 1.0 / Thres1) {
						aite->first->coauthor_update = true;
						count++;
					}
				}
		}
	}
}

/*
*Initialize the similarity between authors with the same name and store the top K.
*/
void AuthorManager::initSim() {
	for (int ite = 0; ite < namesize; ite++) {
		if (author_forests[ite].size() == 0)
			continue;
		AuthorForestLayer &layer = author_forests[ite][0];
		vector<NonLeafAuthor *> &authors = layer.authors;
		int authorNumber = layer.authors.size();
		if (authorNumber <=1)
			continue;

		float w1 = 1.0 / nm.getAmb(ite);
		vector<author_pair_sim> allpairs;
		for (int i = 0; i != authorNumber; i++) {
			for (int j = i + 1; j != authorNumber; j++) {
				float simi = calSim(authors[i], authors[j], w1);
				if (simi > 0) {
					author_pair_sim aij(i, j, simi);
					allpairs.push_back(aij);
				}
			}
		}
		sort(allpairs.begin(), allpairs.end());
		int apairLen = authorNumber;
		if (apairLen > allpairs.size())
			apairLen = allpairs.size();
		layer.apairLen = apairLen;
		author_pair_sim * apairs = new author_pair_sim[apairLen];
		for (int i = 0; i < apairLen; i++) {
			apairs[i] = allpairs[i];
			//cout << "apairss[ite][i]\t" << i<<"\t"<<apairss[ite][i].ai << "\t" << apairss[ite][i].aj << "\t" << apairss[ite][i].score << endl;
		}
		layer.apairs = apairs;
	}
}

void AuthorManager::outputInitAuthorNumber(string & fpath) {
	ofstream fout(fpath);
	for (int i = 1; i < namesize; i++) {
		string name = nm.getName(i);
		int nid = i;
		if (leafAuthorVectors[i].size()==0) {
			continue;
		}
		fout << name << "\t" << leafAuthorVectors[i].size()<< endl;
	}
	fout.close();
}

/*
* calculate the relation strength between each reference and othe type nodes(T,V,A)
*/
void AuthorManager::calRs() {
	int count = 0;
	int update_count = 0;
	cout << "begin to calculate relatedness score" << endl;
	cout << "nameszie\t" << namesize << endl;
	for (int ite = 0; ite<namesize; ite++) {
		if (author_forests[ite].size() == 0)
			continue;
		AuthorForestLayer & layer = author_forests[ite][0];
		if (layer.authors.size()<= 1)
			continue;
		for (int i = 0; i < layer.authors.size(); i++) {
			if (layer.authors[i]->updated) {
				layer.authors[i]->calRs();
				update_count++;
			}
		}
		if (++count % 100 == 0) {
			cout << "calculate relationship strength\t" << count <<"\t"<< update_count<<"\r";
		}
	}
	cout << "calculate relationship strength\t" << count <<"\t" << update_count<<endl;
}

void AuthorManager::setThres(double thres1) {
	Thres1 = thres1;
}

/*
* Merge the node in an iterative way
* Store the internal process
*/
void AuthorManager::iterate() {
	int count = 0;
	bool * flags = (bool*)malloc(sizeof(bool)*namesize);
	memset(flags, 0, sizeof(bool)*namesize);


	long clustertime = 0, updatetime = 0, simitime = 0, threstime = 0;
	time_t begin, end, tbegin, tcurrent;
	time(&tbegin);
	queue<int> que;
	for (int nid = 0; nid<namesize; nid++)
		que.push(nid);

	while (!que.empty()) {
		int nid = que.front();
		que.pop();
		++count;

		if (author_forests[nid].size() == 0){
			continue;
		}

		float amr = nm.getAmb(nid);

		AuthorForestLayer &layer = author_forests[nid][author_forests[nid].size() - 1];
		int authorsize = layer.authors.size();
		if (authorsize == 1 || authorsize<=amr) {
			layer.authors[0]->updated = false;
			continue;
		}


		// if(nid==406){
		// 	int sum=0,sum1=0;
		// 	for (int ai = 0; ai < authorsize; ai++) {
		// 		sum1 += layer.authors[ai]->papers.size();
		// 		if (layer.authors[ai]->del)
		// 			continue;
		// 		sum += layer.authors[ai]->papers.size();
		// 	}
		// 	cout<<"Bing Liu "<< sum<< "\t"<< sum1 <<endl;
		// }

		if (authorsize>500 || count % 10000 == 0)
			cout << nm.getName(nid) << "\tsize\t" << authorsize << "\tqueue size\t" << que.size() << "\t\n";

		time(&begin);
		float w1 = 1.0 / amr;
		time(&begin);
		float Tcluster = MINSIM * amr;
		//---------------------------------threshold---------------------------------
		int diffamr = authorsize - amr;
		if (diffamr <= 0)
			diffamr = 1;
		if (diffamr>4)
			diffamr = (int)(diffamr / 2);
		int psize = authorsize * (authorsize - 1) / 2;

		author_pair_sim * apairs = layer.apairs;
		int apairLen = layer.apairLen;

		set<int> updated;
		vector<author_pair_sim> allpairs;
		vector<NonLeafAuthor *> &authors = layer.authors;
		for(int ai = 0;ai<authorsize;ai++){
			if (authors[ai]->updated) {
				updated.insert(ai);
				for (int aj = ai+1; aj <  authorsize; aj++) {
					float sim = calSim(authors[ai], authors[aj], w1);
					if (sim > 0)
						allpairs.push_back(author_pair_sim(ai, aj, sim));
				}
				authors[ai]->updated = false;
			}
		}

		// put the ones without update
		for (int api = 0; api < apairLen; api++) {
			allpairs.push_back(apairs[api]);
		}

		if (updated.size() != 0) {
			sort(allpairs.begin(), allpairs.end());
		}

		int index = diffamr;
		if (index >= allpairs.size())
			index = allpairs.size() - 1;


		//norm by Tcluster
		while (index >= 0 && allpairs[index].score < Tcluster) {
			index--;
		}

		if (index == -1) {
			flags[nid] = true;
			continue;
		}
		if (allpairs[index].score > Tcluster)
			index++;


		//norm by MAXSIM
		while (index < allpairs.size() && allpairs[index].score > MAXSIM) {
			index++;
		}

		if (index == allpairs.size())
			index--;

		int * assigned = new int[authorsize];
		for (int ai = 0; ai < authorsize; ai++)
			assigned[ai] = ai;

		for (int i = 0; i < index; i++)
			assigned[allpairs[i].ai] = allpairs[i].aj;

		uniFind(assigned, authorsize);

		apairLen = authorsize;
		if (apairLen > allpairs.size())
			apairLen = allpairs.size();
		if (apairLen == 0) {
			layer.apairLen = 0;
			layer.apairs = NULL;
		}
		else if (layer.apairLen != apairLen) {
			delete[] layer.apairs;
			layer.apairs = new author_pair_sim[apairLen];
			layer.apairLen = apairLen;
		}
		for (int i = 0; i < apairLen; i++)
			layer.apairs[i] = allpairs[i];

		time(&begin);

		//------------------------------clustering-------------------------------
		map<int,int> label2size;
		for (int assi = 0; assi < authorsize; assi++) {
			if (assigned[assi] == -1) {
				cout << "ERROR in iteration assigned -1" << endl;
				exit(1);
			}
			int label = assigned[assi];
			if (label2size.find(label) == label2size.end())
				label2size[label] = 1;
			else
				label2size[label] += 1;
		}
		int new_authorsize = label2size.size();

		if (new_authorsize != authorsize){
			//---------------------------------update---------------------------------
			// reassign the assignment from 0 to n
			time(&begin);
			AuthorForestLayer new_layer;

			for (auto label2sizePair : label2size) {
				int label = label2sizePair.first;
				NonLeafAuthor *author = NULL;
				author = new NonLeafAuthor(nid, AID++);
				for (int assi = 0; assi < authorsize; assi++) {
					if (assigned[assi] == label) {
						author->merge(authors[assi]);
						assigned[assi] = -1;
					}
				}
				if (label2sizePair.second > 1) {// no need to create new node
					update(que, flags, author);
				}
				new_layer.authors.push_back(author);
			}

			time(&end);
			updatetime += difftime(end, begin);

			//update all pairs
			apairLen = allpairs.size();
			vector<author_pair_sim> newallpair;
			for (int api = 0; api < allpairs.size(); api++) {
				int oindex1 = allpairs[api].ai;
				int oindex2 = allpairs[api].aj;
				int nindex1 = assigned[oindex1];
				int nindex2 = assigned[oindex2];
				if (nindex1 == -1 || nindex2 == -1) // nodes has been updated.
					continue;
				newallpair.push_back(author_pair_sim(nindex1, nindex2, allpairs[api].score));
			}
			apairLen = newallpair.size();
			if (apairLen > authorsize)
				apairLen = authorsize;

			new_layer.apairLen = apairLen;
			if (apairLen > 0) {
				new_layer.apairs = new author_pair_sim[apairLen];
				for (int j = 0; j < apairLen; j++) {
					new_layer.apairs[j] = newallpair[j];
				}
			}
			else
				new_layer.apairs = NULL;
			que.push(nid);
			author_forests[nid].push_back(new_layer);
		}
		else {
			flags[nid] = true;
		}
		delete[] assigned;
	}
	delete flags;

}


/*
* Merge the node in an iterative way
* Store the internal process
*/
void AuthorManager::inc_iterate() {
	int count = 0;

	bool * flags = new bool[namesize];
	memset(flags, 0, sizeof(bool)*namesize);
	int * current_layers = new int[namesize];
	memset(current_layers, 0, sizeof(int)*namesize);

	queue<int> que;
	set<int> update_set;
	for (int nid = 0; nid < namesize; nid++) {
		//flags[nid] = true;//debug HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (author_forests[nid].size() == 0)
			continue;
		for (auto author : author_forests[nid][0].authors) {
			if (author->updated) {//delete author->coauthor_update 
				que.push(nid);
				update_set.insert(nid);
				//flags[nid] = false;
				break;
			}
		}
	}
	int count1 = 0; int count2 = 0;
	cout << "initial que size "<<que.size() << endl;
  
	while (!que.empty()) {
		int nid = que.front();
		que.pop();
		++count;

		if (author_forests[nid].size() == 0) {
			continue;
		}

		float amr = nm.getAmb(nid);
		int current_layer = current_layers[nid];
		if (current_layer >= author_forests[nid].size()) {
			cout << "BUG HERE layer out of range" << endl;
			flags[nid] = true;
			continue;
		}
		AuthorForestLayer &layer = author_forests[nid][current_layer];
		int authorsize = layer.authors.size();
		if (authorsize <= amr ) {
			flags[nid] = false;
			continue;
		}


		float w1 = 1.0 / amr;
		float Tcluster = MINSIM * amr;
		//---------------------------------threshold---------------------------------
		int diffamr = authorsize - amr;
		if (diffamr <= 0)
			diffamr = 1;
		if (diffamr > 4)
			diffamr = (int)(diffamr / 2);
		int psize = authorsize * (authorsize - 1) / 2;

		author_pair_sim * apairs = layer.apairs;
		int apairLen = layer.apairLen;
		set<int> updated;

		vector<author_pair_sim> allpairs;
		vector<NonLeafAuthor *> &authors = layer.authors;

		// put the ones without update
		for (int api = 0; api < apairLen; api++) {
			if (authors[apairs[api].ai]->updated || authors[apairs[api].aj]->updated || authors[apairs[api].ai]->coauthor_update || authors[apairs[api].aj]->coauthor_update)
				continue;
			allpairs.push_back(apairs[api]);
		}

		// if diffamr > allpairs then need to recalculate the whole to find topK to merge
		//if (diffamr > allpairs.size()) {
		//	allpairs.clear();
		//	for (int ai = 0; ai < authorsize; ai++) {
		//		authors[ai]->coauthor_update = true;
		//	}
		//}

		for (int ai = 0; ai < authorsize; ai++) {
			if (authors[ai]->del)
				continue;
			if (authors[ai]->updated || authors[ai]->coauthor_update) {
				updated.insert(ai);
				for (int aj = ai + 1; aj < authorsize; aj++) {
					if (authors[aj]->del)
						continue;
					float sim = calSim(authors[ai], authors[aj], w1);
					if (sim > Tcluster)
						allpairs.push_back(author_pair_sim(ai, aj, sim));
				}
			}
		}

		if (updated.size() == 0) {
			flags[nid] = true;
			continue;
		}
		if (authorsize > 500 || count % 10000 == 0)
			cout << nm.getName(nid) << "\tsize\t" << authorsize << "\tqueue size\t" << que.size() << "\t" << current_layers[nid] << "\tupdated size"<< updated.size()<<"\n";


		//debug
		if(nid==406){
			int sum=0,sum1=0;
			for (int ai = 0; ai < authorsize; ai++) {
				sum1 += authors[ai]->papers.size();
				if (authors[ai]->del)
					continue;
				sum += authors[ai]->papers.size();
			}
			cout<<"Bing Liu "<< sum<< "\t"<< sum1 <<endl;
		}

		if (allpairs.size() == 0) {
			for (int ai = 0; ai <authorsize; ai++) {
				if (authors[ai]->del)
					continue;
				authors[ai]->updated = false;
				authors[ai]->coauthor_update = false;
				layer.apairLen = 0;
				layer.apairs = NULL;
			}
			flags[nid] = true;
			continue;
		}

		sort(allpairs.begin(), allpairs.end());

		int index = diffamr;
		if (index >= allpairs.size())
			index = allpairs.size() - 1;

		//update author_pair_sim * apairs = layer.apairs; and ;
		apairLen = authorsize;
		if (apairLen > allpairs.size())
			apairLen = allpairs.size();

		if (apairLen <= 0) {
			layer.apairLen = 0;
			layer.apairs = NULL;
		}
		else if (layer.apairLen!=apairLen){
			delete[] layer.apairs;
			layer.apairs = new author_pair_sim[apairLen];
			layer.apairLen = apairLen;
		}
		for (int i = 0; i < apairLen; i++)
			layer.apairs[i] = allpairs[i];
		//-----------------------------------------------------------------------------------


		//norm by MAXSIM
		while (index < allpairs.size() && allpairs[index].score > MAXSIM) {
			index++;
		}

		if (index == allpairs.size())
			index--;

		int * assigned = new int[authorsize];
		for (int ai = 0; ai < authorsize; ai++)
			assigned[ai] = ai;


		for (int i = 0; i <= index; i++)
			assigned[allpairs[i].ai] = allpairs[i].aj;

		uniFind(assigned, authorsize);
		map<NonLeafAuthor*, int> author2label;

		for(int ai=0;ai<authorsize;ai++){
			if(authors[ai]->del)
				assigned[ai]=-1;
		}

		for (int i = 0; i < authorsize; i++)
			author2label[authors[i]] = assigned[i];

		//------------------------------clustering-------------------------------
		map<int,set<int>> label_2_author_clusters;
		for (int assi = 0; assi < authorsize; assi++) {
			if (assigned[assi] == -1) {
				continue;
			}
			int label = assigned[assi];
			if (label_2_author_clusters.find(label) == label_2_author_clusters.end())
				label_2_author_clusters[label] = set<int>();
			label_2_author_clusters[label].insert(assi);
		}



		int new_authorsize = label_2_author_clusters.size();
		if (new_authorsize != authorsize) {
			// updated
			//---------------------------------update---------------------------------
			int next_layer = current_layer + 1;
			if (author_forests[nid].size() <= next_layer) {
				AuthorForestLayer new_layer;
				new_layer.apairLen = 0;
				new_layer.apairs = NULL;

				for (auto label_2_authors_pair : label_2_author_clusters) {
					int label = label_2_authors_pair.first;
					NonLeafAuthor *author = NULL;
					author = new NonLeafAuthor(nid, AID++);
					for (int assi = 0; assi < authorsize; assi++) {
						if (assigned[assi] == label) {
							author->merge(authors[assi]);
							assigned[assi] = -1;
						}
					}
					if (label_2_authors_pair.second.size() > 1) {// no need to create new node
						update(que, flags, author);
					}
					new_layer.authors.push_back(author);
				}

				vector<author_pair_sim> newallpair;
				for (int api = 0; api < allpairs.size(); api++) {
					int oindex1 = allpairs[api].ai;
					int oindex2 = allpairs[api].aj;
					int nindex1 = assigned[oindex1];
					int nindex2 = assigned[oindex2];
					if (nindex1 == -1 || nindex2 == -1) // nodes has been updated.
						continue;
					newallpair.push_back(author_pair_sim(nindex1, nindex2, allpairs[api].score));
				}
				apairLen = newallpair.size();
				if (apairLen > authorsize)
					apairLen = authorsize;

				new_layer.apairLen = apairLen;

				if (apairLen > 0) {
					new_layer.apairs = new author_pair_sim[apairLen];
					for (int j = 0; j < apairLen; j++) {
						new_layer.apairs[j] = newallpair[j];
					}
				}
				else
					new_layer.apairs = NULL;
				author_forests[nid].push_back(new_layer);
			}
			else {
				AuthorForestLayer & new_layer = author_forests[nid][next_layer];

				for (auto author : new_layer.authors) {
					int label = -1;
					for (auto son : author->authors) {
						if (son->updated) {
							author->del = true;
							author->updated = true;
							break;
						}
						NonLeafAuthor* non_leaf_son = (NonLeafAuthor*)son;
						if (non_leaf_son->coauthor_update)
							author->coauthor_update = true;
						if (label == -1) {
							label = author2label[non_leaf_son];
						}
						else if (label != author2label[non_leaf_son]) {
							author->del = true;
							author->updated = true;
							break;
						}
					}
				}

				if(nid==406){
					int sum=0,sum1=0;
					for (int ai = 0; ai < new_layer.authors.size(); ai++) {
						sum1 += new_layer.authors[ai]->papers.size();
						if (new_layer.authors[ai]->del)
							continue;
						sum += new_layer.authors[ai]->papers.size();
					}
					cout<<"Bing Liu in the middle update\t"<< sum<< "\t"<< sum1 <<endl;
					sum=0;
					for (auto label_2_authors_pair : label_2_author_clusters) {
						for(auto aid: label_2_authors_pair.second){
							sum += authors[aid]->papers.size();
						}
					}
					cout<<"Bing Liu in the before label update\t"<< sum<< "\t"<< sum1 <<endl;


				}

				/*
				* In these cases, new father is needed. and old fathers are deleted.
				*1. one of the sons are updated.
				*2. sons have different labels.
				*3. have new sons.
				*/
				for (auto label_2_authors_pair : label_2_author_clusters) {
					int label = label_2_authors_pair.first;
					NonLeafAuthor *father_author = NULL;
					bool need_new_father = false;
					//detect if need new father
					for (auto iid : label_2_authors_pair.second) {
						if (authors[iid]->father != NULL) {
							if (authors[iid]->father->del) {
								father_author = authors[iid]->father;
								need_new_father = true;
								count1++;
								break;
							}
							if (father_author == NULL)
								father_author = authors[iid]->father;
							else if (father_author != authors[iid]->father) {
								need_new_father = true;
								count2++;
								break;
							}
						}
					}
					if (need_new_father) {
						//cout << "-----------------"<<father_author->authors.size() << "\t" << label_2_authors_pair.second.size() << endl;
						NonLeafAuthor *father_author = new NonLeafAuthor(nid, AID++);
						for (auto iid : label_2_authors_pair.second) {
							if (authors[iid]->father != NULL) {
								authors[iid]->father->del = true;
								authors[iid]->father->updated = true;
								father_author->merge(authors[iid]);
							}
						}
						update(que, flags, father_author);
						new_layer.authors.push_back(father_author);
					}
				}

				if(nid==406){
					int sum=0,sum1=0;
					for (int ai = 0; ai < new_layer.authors.size(); ai++) {
						sum1 += new_layer.authors[ai]->papers.size();
						if (new_layer.authors[ai]->del)
							continue;
						sum += new_layer.authors[ai]->papers.size();
					}
					cout<<"Bing Liu after update\t"<< sum<< "\t"<< sum1 <<endl;
				}

			}
			que.push(nid);
			current_layers[nid] += 1;
		}
		else {
			flags[nid] = true;
		}
		for (int ai = 0; ai < author_forests[nid][current_layer].authors.size(); ai++) {
			if (author_forests[nid][current_layer].authors[ai]->del)
				continue;
			author_forests[nid][current_layer].authors[ai]->updated = false;
			author_forests[nid][current_layer].authors[ai]->coauthor_update = false;
		}

		delete[] assigned;
	}

	for(auto nid : update_set){
		while(author_forests[nid].size()>current_layers[nid]+1)
			author_forests[nid].pop_back();
	}
	delete[] current_layers;
	delete [] flags;
}

void AuthorManager::clusterSingle() {
	cout << "deal with single author" << endl;
	bool intest = false;
	int allcount = 0, testcount = 0;
	for (int nid = 0; nid<namesize; nid++) {

		vector<AuthorForestLayer> & authorForest = author_forests[nid];
		if (authorForest.size() == 0)
			continue;
		AuthorForestLayer &layer = authorForest[authorForest.size()-1];
		//if (layer.authorsize <= 1)
		//	continue;
		vector<NonLeafAuthor *> &authors = layer.authors;
		int authorsize = authors.size();


		int *assigned = new int[authorsize];
		for (int i = 0; i < authorsize; i++)
			assigned[i] = i;
		for (int ai = 0; ai < authorsize;ai++) {
			float amr = nm.getAmb(nid);
			float w1 = 1.0 / amr;
			if (authors[ai]->a2s->size() == 0) {
				double maxsimi = 0;
				int maxauthorIndex = -1;
				for (int aj = 0; aj < authorsize;aj++) {
					if (authors[aj]->a2s->size() == 0)
						continue;
					double simi = calSimSingle(authors[ai], authors[aj], w1);
					if (simi>maxsimi) {
						maxsimi = simi;
						maxauthorIndex = aj;
					}
				}
				if (maxsimi>MINSIM*amr) {
					assigned[ai]= maxauthorIndex;
					allcount++;
					if (intest)
						testcount++;
				}
			}
		}

		uniFind(assigned, authorsize);

		map<int, int> label2size;
		for (int i = 0; i < authorsize; i++) {
			int label = assigned[i];
			if (label < 0) {
				cout << "error in clustering single -1" << endl;
				exit(1);
			}
			if (label2size.find(label) == label2size.end())
				label2size[label] = 1;
			else
				label2size[label] += 1;
		}
		AuthorForestLayer &outputLayer = outputLayers[nid];
		outputLayer.authors.reserve(label2size.size());

		for (auto label2sizePair : label2size) {
			int label = label2sizePair.first;
			NonLeafAuthor *nauthor = new NonLeafAuthor(nid, AID++);
			for (int ai = 0; ai < authorsize; ai++) {
				if (assigned[ai] == label) {
					nauthor->merge(authors[ai]);
				}
			}
			outputLayer.authors.push_back(nauthor);
		}
	}
	cout << "allcont\t" << allcount << "\tintest\t" << testcount << endl;
}

void AuthorManager::clusterSingle(int nid) {
	vector<AuthorForestLayer> & authorForest = author_forests[nid];
	if (authorForest.size() == 0)
		return;
	AuthorForestLayer &layer = authorForest[authorForest.size() - 1];
	vector<NonLeafAuthor *> &authors = layer.authors;
	int authorsize = authors.size();
	int sum =0;
	// for(int ai=0;ai<authorsize;ai++){
	// 	if(authors[ai]->del)
	// 		continue;
	// 	sum += authors[ai]->papers.size();
	// }
	// cout<<nm.getName(nid)<<"\t"<<sum<<endl;

	int *assigned = new int[authorsize];
	for (int i = 0; i < authorsize; i++)
		assigned[i] = i;
	for (int ai = 0; ai < authorsize; ai++) {
		float amr = nm.getAmb(nid);
		float w1 = 1.0 / amr;
		if (authors[ai]->a2s->size() == 0) {
			double maxsimi = 0;
			int maxauthorIndex = -1;
			for (int aj = 0; aj < authorsize; aj++) {
				if (authors[aj]->a2s->size() == 0 || authors[aj]->del)
					continue;
				double simi = calSimSingle(authors[ai], authors[aj], w1);
				if (simi>maxsimi) {
					maxsimi = simi;
					maxauthorIndex = aj;
				}
			}
			if (maxsimi>MINSIM*amr) {
				assigned[ai] = maxauthorIndex;
			}
		}
	}

	uniFind(assigned, authorsize);
	for(int ai=0;ai<authorsize;ai++){
		if(authors[ai]->del)
			assigned[ai]=-1;
	}

	map<int, int> label2size;
	for (int i = 0; i < authorsize; i++) {
		int label = assigned[i];
		if (label < 0) {
			continue;
		}
		if (label2size.find(label) == label2size.end())
			label2size[label] = 1;
		else
			label2size[label] += 1;
	}

	AuthorForestLayer &outputLayer = outputLayers[nid];
	outputLayer.authors.clear();
	for (auto label2sizePair : label2size) {
		int label = label2sizePair.first;
		NonLeafAuthor *nauthor = new NonLeafAuthor(nid, AID++);
		for (int ai = 0; ai < authorsize; ai++) {
			if (assigned[ai] == label) {
				nauthor->merge(authors[ai]);
			}
		}
		outputLayer.authors.push_back(nauthor);
	}

	// sum =0;
	// for(int ai=0;ai<outputLayer.authors.size();ai++){
	// 	if(outputLayer.authors[ai]->del)
	// 		continue;
	// 	sum += outputLayer.authors[ai]->papers.size();
	// }
	// cout<<nm.getName(nid)<<"\t after clustering single"<<sum<<endl;

}


/*
* calculate the similarity of two authors
*/
float AuthorManager::calSim(NonLeafAuthor *ai, NonLeafAuthor *aj, double w1) {

	if (w1 >= 1)
		return 1.0;

	//year different
	int yd = 1;
	if (ai->begin>aj->end)
		yd = (ai->begin + 1 - aj->end);
	else if (aj->begin>ai->end)
		yd = (aj->begin + 1 - ai->end);
	int ybegin = MAX(ai->begin, aj->begin);
	int yend = MAX(ai->end, aj->end);
	int psize_i = ai->papers.size();
	int psize_j = aj->papers.size();
	size_t size_a2s_i = ai->a2s->size();
	size_t size_a2s_j = aj->a2s->size();
	four_float vsimi, tsimi, asimi, nsimi;
	vsimi.init();
	nsimi.init();
	tsimi.init();
	asimi.init();

	int vcount = 0, acount = 0, ncount = 0;
	four_int tcount;
	tcount.init();



	if (size_a2s_i == 0 || size_a2s_j == 0) {
		asimi.setValue(0);
	}
	else {
		map<NonLeafAuthor*, two_int>::iterator itei = ai->a2s->begin();
		map<NonLeafAuthor*, two_int>::iterator itej = aj->a2s->begin();
		while (itei != ai->a2s->end() && itej != aj->a2s->end()) {
			if (itei->first == itej->first) {
				two_float s1 = itei->second / (float)size_a2s_i;
				two_float s2 = itej->second / (float)size_a2s_j;
				int paper_size = itei->first->papers.size();
				if (paper_size>0) {
					asimi.f11 += MINI(s1.f1, s2.f1) / paper_size;
					asimi.f22 += MINI(s1.f2, s2.f2) / paper_size;
					asimi.f12 += MINI(s1.f1, s2.f2) / paper_size;
					asimi.f21 += MINI(s1.f2, s2.f1) / paper_size;
					if (s1.f1>0 && s2.f1>0) {
						acount += 1;
					}
				}

				if (3 * asimi.f11 >= MAXSIM || asimi.f11 + asimi.f12 + asimi.f21 >= MAXSIM)
					return MAXSIM + 1e-10;

				itei++;
				itej++;
			}
			else if (itei->first<itej->first)
				itei++;
			else itej++;
		}

	}
	size_t size_n2s_i = 0;
	size_t size_n2s_j = 0;
	int ch_name_size_i = 0, ch_name_size_j = 0;
	double ratio_i = 0, ratio_j = 0;
	int counti = 0, countj = 0;
	if (size_a2s_i == 0 || size_a2s_j == 0) {

	}
	else {
		int nbegin = ai->a2s->begin()->first->nid;
		two_int sum;
		sum.init();

		for (map<NonLeafAuthor*, two_int>::iterator iteni = ai->a2s->begin(); iteni != ai->a2s->end(); iteni++) {
			int nid = iteni->first->nid;
			if (nid != nbegin) {
				size_n2s_i++;
				if (sum.f1>0) {
					counti++;
					if (nm.isCn(nbegin)) {
						ch_name_size_i++;
					}
				}
				nbegin = nid;
				sum.init();
			}
			sum += (iteni)->second;
		}
		size_n2s_i++;
		if (sum.f1>0) {
			counti++;
			if (nm.isCn(nbegin))
				ch_name_size_i++;
		}
		sum.init();

		nbegin = aj->a2s->begin()->first->nid;
		for (map<NonLeafAuthor*, two_int>::iterator itenj = aj->a2s->begin(); itenj != aj->a2s->end(); itenj++) {
			int nid = itenj->first->nid;
			if (nid != nbegin) {
				size_n2s_j++;
				if (sum.f1>0) {
					countj++;
					if (nm.isCn(nbegin))
						ch_name_size_j++;
				}
				nbegin = nid;
				sum.init();
			}
			sum += (itenj)->second;
		}
		size_n2s_j++;
		if (sum.f1>0) {
			countj++;
			if (nm.isCn(nbegin))
				ch_name_size_j++;
		}
		if (counti>0)
			ratio_i = 1.0 - ch_name_size_i * 1.0 / (counti);
		if (countj>0)
			ratio_j = 1.0 - ch_name_size_j * 1.0 / (countj);



		map<NonLeafAuthor*, two_int>::iterator iteni = ai->a2s->begin();
		map<NonLeafAuthor*, two_int>::iterator itenj = aj->a2s->begin();
		while (iteni != ai->a2s->end() && itenj != aj->a2s->end()) {
			int nid = (iteni->first)->nid;
			if (iteni->first->nid == itenj->first->nid) {
				two_int n2s1, n2s2;
				n2s1.init();
				n2s2.init();
				for (; iteni != ai->a2s->end() && (iteni->first->nid) == nid; iteni++) {
					n2s1 += iteni->second;
				}
				for (; itenj != aj->a2s->end() && itenj->first->nid == nid; itenj++) {
					n2s2 += itenj->second;
				}
				two_float s1 = n2s1 / (float)size_n2s_i;
				two_float s2 = n2s2 / (float)size_n2s_j;
				int nsize = nm.getAmb(nid);
				if (nsize>0) {
					nsimi.f11 += (MINI(s1.f1, s2.f1)) / nsize;
					nsimi.f22 += (MINI(s1.f2, s2.f2)) / nsize;
					nsimi.f12 += (MINI(s1.f1, s2.f2)) / nsize;
					nsimi.f21 += (MINI(s1.f2, s2.f1)) / nsize;
					if (s1.f1>0 && s2.f1>0)
						ncount += 1;
				}
			}
			else if (iteni->first->nid<itenj->first->nid) {
				int tnid = itenj->first->nid;
				do {
					iteni++;
				} while (iteni != ai->a2s->end() && iteni->first->nid<tnid);
			}
			else {
				int tnid = iteni->first->nid;
				do {
					itenj++;
				} while (itenj != aj->a2s->end() && itenj->first->nid<tnid);
			}
		}
	}



	size_t size_t2s_i = ai->t2s->size();
	size_t size_t2s_j = aj->t2s->size();
	if (size_t2s_i == 0 || size_t2s_j == 0)
		tsimi.setValue(0);
	else {
		if (ai->ksize + aj->ksize == 0) {
			cout << size_t2s_i << "\t" << size_t2s_j << "\t" << ai->ksize << "\t" << aj->ksize << endl;
			system("pause");
		}
		map<int, float>::iterator iteti = ai->t2s->begin();
		map<int, float>::iterator itetj = aj->t2s->begin();
		map<int, float>::iterator iteti_s = ai->t2s_simi->begin();
		map<int, float>::iterator itetj_s = aj->t2s_simi->begin();


		iteti = ai->t2s->begin();
		itetj = aj->t2s->begin();
		while (iteti != ai->t2s->end() && itetj_s != aj->t2s_simi->end()) {
			if (iteti->first == itetj_s->first) {
				float s1 = iteti->second / size_t2s_i;///ai->ksize;//
				float s2 = itetj_s->second / size_t2s_j;//aj->ksize;//
				tsimi.f12 += MINI(s1, s2) / km.sizes[iteti->first];

				tcount.i12++;

				iteti++;
				itetj_s++;

			}
			else if (iteti->first<itetj_s->first)
				iteti++;
			else
				itetj_s++;
		}
		itetj_s = aj->t2s_simi->begin();
		while (iteti_s != ai->t2s_simi->end() && itetj != aj->t2s->end()) {
			if (iteti_s->first == itetj->first) {
				float s1 = 1.0*iteti_s->second / size_t2s_i;//ai->ksize;/
				float s2 = 1.0*itetj->second / size_t2s_j;//aj->ksize;
				tsimi.f21 += MINI(s1, s2) / km.sizes[iteti_s->first];

				tcount.i21++;

				iteti_s++;
				itetj++;
			}
			else if (iteti_s->first<itetj->first)
				iteti_s++;
			else
				itetj++;
		}

	}


	size_t size_v2s_i = ai->v2s->size();
	size_t size_v2s_j = aj->v2s->size();

	if (size_v2s_i == 0 || size_v2s_j == 0) {
		/*vsimi=tsimi*5*/;
	}
	else {
		if (ai->vsize + aj->vsize == 0) {
			cout << "VVVV\t" << size_v2s_i << "\t" << size_v2s_j << "\t" << ai->vsize << "\t" << aj->vsize << endl;
			system("pause");
		}
		map<int, float>::iterator itevi = ai->v2s->begin();
		map<int, float>::iterator itevj = aj->v2s->begin();
		map<int, float>::iterator itevi_s = ai->v2s_simi->begin();
		map<int, float>::iterator itevj_s = aj->v2s_simi->begin();

		while (itevi != ai->v2s->end() && itevj_s != aj->v2s_simi->end()) {
			if (itevi->first == itevj_s->first) {
				float s1 = itevi->second / size_v2s_i;//ai->vsize;
				float s2 = itevj_s->second / size_v2s_j;//aj->vsize;
				int vsize = vm.getsize(itevi->first);
				if (vsize>0)
					vsimi.f12 += MINI(s1, s2) / vsize;
				itevi++;
				itevj_s++;
			}
			else if (itevi->first<itevj_s->first)
				itevi++;
			else
				itevj_s++;
		}


		itevi_s = ai->v2s_simi->begin();
		itevj = aj->v2s->begin();
		while (itevi_s != ai->v2s_simi->end() && itevj != aj->v2s->end()) {
			if (itevi_s->first == itevj->first) {
				float s1 = itevi_s->second / size_v2s_i;//ai->vsize;//
				float s2 = itevj->second / size_v2s_j;//aj->vsize;/
				int vsize = vm.getsize(itevi_s->first);
				if (vsize>0)
					vsimi.f21 += MINI(s1, s2) / vsize;
				itevi_s++;
				itevj++;
			}
			else if (itevi_s->first<itevj->first)
				itevi_s++;
			else
				itevj++;
		}
	}



	if (size_v2s_i == 0 || size_v2s_j == 0) {
		vsimi = tsimi * 5;
	}

	if (!nm.isCn(ai->nid) || w1 >= 1)
		return asimi.f11 + asimi.f12 + asimi.f21
		+ nsimi.f11 + nsimi.f12 + nsimi.f21
		+ vsimi.f12 + vsimi.f21
		+ tsimi.f12 + tsimi.f21;

	//if (!nm.isCn(ai->nid) || w1 >= 1)
	//	return asimi.f11 + asimi.f12 + asimi.f21
	//	+ nsimi.f11
	//	+ vsimi.f12 + vsimi.f21
	//	+ tsimi.f12 + tsimi.f21;


	double res = 0;
	if (w1 > Thres1) {
		double simia = asimi.f11 + asimi.f12 + asimi.f21;
		double simin = nsimi.f11 + nsimi.f12 + nsimi.f21;
		double simiv = vsimi.f12 + vsimi.f21;
		double simit = tsimi.f12 + tsimi.f21;
		res = sqrt(simia*simin + simia * simit + simia * simiv + simiv * simit + simiv * simin + simit * simin);
		return res;
	}


	double simia = 3 * asimi.f11;
	double simin = nsimi.f21 + nsimi.f12 + nsimi.f11;



	double simiv = vsimi.f12 + vsimi.f21;
	double simit = tsimi.f12 + tsimi.f21;
	res = sqrt(simia*simin + simia * simit + simia * simiv + simiv * simit + simiv * simin + simit * simin);

#if COMPILE_LINUX
	if (isfinite(res))
		return res;
#else
	if (_finite(res))
		return res;
#endif
	cout << "ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	return 0;
}

float AuthorManager::calSimSingle(NonLeafAuthor *ai, NonLeafAuthor *aj, double w1) {

	//year different
	int yd = 1;
	if (ai->begin>aj->end)
		yd = (ai->begin + 1 - aj->end);
	else if (aj->begin>ai->end)
		yd = (aj->begin + 1 - ai->end);
	int ybegin = MAX(ai->begin, aj->begin);
	int yend = MAX(ai->end, aj->end);

	four_float vsimi, tsimi;
	vsimi.init();
	tsimi.init();
	int vcount = 0;
	four_int tcount;
	tcount.init();

	size_t size_t2s_i = ai->t2s->size();
	size_t size_t2s_j = aj->t2s->size();
	map<int, float>::iterator iteti = ai->t2s->begin();
	map<int, float>::iterator itetj = aj->t2s->begin();
	map<int, float>::iterator iteti_s = ai->t2s_simi->begin();
	map<int, float>::iterator itetj_s = aj->t2s_simi->begin();

	while (iteti != ai->t2s->end() && itetj != aj->t2s->end()) {
		if (iteti->first == itetj->first) {
			float s1 = iteti->second / size_t2s_i;
			float s2 = itetj->second / size_t2s_j;
			tsimi.f11 += MINI(s1, s2) / km.sizes[iteti->first];
			tcount.i11++;
			iteti++;
			itetj++;
		}
		else if (iteti->first<itetj->first)
			iteti++;
		else
			itetj++;
	}
	iteti = ai->t2s->begin();
	itetj = aj->t2s->begin();
	while (iteti != ai->t2s->end() && itetj_s != aj->t2s_simi->end()) {
		if (iteti->first == itetj_s->first) {
			float s1 = iteti->second / size_t2s_i;
			float s2 = itetj_s->second / size_t2s_j;
			tsimi.f12 += MINI(s1, s2) / km.sizes[iteti->first];

			tcount.i12++;

			iteti++;
			itetj_s++;

		}
		else if (iteti->first<itetj_s->first)
			iteti++;
		else
			itetj_s++;
	}

	itetj_s = aj->t2s_simi->begin();
	while (iteti_s != ai->t2s_simi->end() && itetj != aj->t2s->end()) {
		if (iteti_s->first == itetj->first) {
			float s1 = iteti_s->second / size_t2s_i;
			float s2 = itetj->second / size_t2s_j;
			tsimi.f21 += MINI(s1, s2) / km.sizes[iteti_s->first];

			tcount.i21++;

			iteti_s++;
			itetj++;
		}
		else if (iteti_s->first<itetj->first)
			iteti_s++;
		else
			itetj++;
	}
	iteti_s = ai->t2s_simi->begin();
	while (iteti_s != ai->t2s_simi->end() && itetj_s != aj->t2s_simi->end()) {
		if (iteti_s->first == itetj_s->first) {
			float s1 = iteti_s->second / size_t2s_i;
			float s2 = itetj_s->second / size_t2s_j;
			tsimi.f22 += MINI(s1, s2) / km.sizes[iteti_s->first];

			tcount.i22++;
			iteti_s++;
			itetj_s++;
		}
		else if (iteti_s->first<itetj_s->first)
			iteti_s++;
		else
			itetj_s++;
	}

	size_t size_v2s_i = ai->v2s->size();
	size_t size_v2s_j = aj->v2s->size();

	if (size_v2s_i>0 && size_v2s_j>0) {
		map<int, float>::iterator itevi = ai->v2s->begin();
		map<int, float>::iterator itevj = aj->v2s->begin();
		map<int, float>::iterator itevi_s = ai->v2s_simi->begin();
		map<int, float>::iterator itevj_s = aj->v2s_simi->begin();

		while (itevi != ai->v2s->end() && itevj_s != aj->v2s_simi->end()) {
			if (itevi->first == itevj_s->first) {
				float s1 = itevi->second / size_v2s_i;
				float s2 = itevj_s->second / size_v2s_j;
				vsimi.f12 += MINI(s1, s2) / vm.getsize(itevi->first);
				itevi++;
				itevj_s++;
			}
			else if (itevi->first<itevj_s->first)
				itevi++;
			else
				itevj_s++;
		}

		itevi_s = ai->v2s_simi->begin();
		itevj = aj->v2s->begin();
		while (itevi_s != ai->v2s_simi->end() && itevj != aj->v2s->end()) {
			if (itevi_s->first == itevj->first) {
				float s1 = itevi_s->second / size_v2s_i;
				float s2 = itevj->second / size_v2s_j;
				vsimi.f21 += MINI(s1, s2) / vm.getsize(itevi_s->first);
				itevi_s++;
				itevj++;
			}
			else if (itevi_s->first<itevj->first)
				itevi_s++;
			else
				itevj++;
		}
		itevi_s = ai->v2s_simi->begin();
		itevj_s = aj->v2s_simi->begin();
		while (itevi_s != ai->v2s_simi->end() && itevj_s != aj->v2s_simi->end()) {
			if (itevi_s->first == itevj_s->first) {
				float s1 = itevi_s->second / size_v2s_i;
				float s2 = itevj_s->second / size_v2s_j;
				vsimi.f22 += MINI(s1, s2) / vm.getsize(itevi_s->first);
				itevi_s++;
				itevj_s++;
			}
			else if (itevi_s->first<itevj_s->first)
				itevi_s++;
			else
				itevj_s++;
		}
	}

	if (size_v2s_i == 0 || size_v2s_j == 0) {
		vsimi = tsimi * 5;
	}

	double res = (vsimi.f12 + vsimi.f21 + tsimi.f12 + tsimi.f21);

#if COMPILE_LINUX
	if (isfinite(res))
		return res;
#else
	if (_finite(res))
		return res;
#endif
	cout << "res  ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	cout << vsimi.f12 << "\t" << vsimi.f21 << "\t" << tsimi.f12 << "\t" << tsimi.f21;
	return 0;
}

void AuthorManager::getcluster(list<Author *>* authors, const int *assigned, vector<vector<Author *>*> &clusters) {
	size_t size = authors->size();
	for (int i = 0; i < size; i++) {
		if (assigned[i] == i) {
			vector<Author*> *temp = new vector<Author*>();
			clusters.push_back(temp);
			list<Author*>::iterator ite = authors->begin();
			for (int j = 0; j < size; j++, ite++) {
				if (assigned[j] == i)
					temp->push_back(*ite);
			}
		}
	}
}

/*
* flag the coauthors to be updated
*
*/
void AuthorManager::update(queue<int> & que, bool * flags, NonLeafAuthor * author) {
	for(map<NonLeafAuthor*, two_int>::iterator aite = author->a2s->begin();aite!=author->a2s->end();aite++){
		if (flags[aite->first->nid]) {
			flags[aite->first->nid] = false;
				que.push(aite->first->nid);
		}
	}
}


void AuthorManager::outputCluster(ofstream& fout, int nid) {
	AuthorForestLayer & layer = outputLayers[nid];
	for(int ai=0;ai<layer.authors.size();ai++){
		fout << layer.authors[ai]->id << ":::";
		vector<Paper *>::iterator ite = layer.authors[ai]->papers.begin();
		for (; ite != layer.authors[ai]->papers.end(); ite++) {
			fout << (*ite)->id << "\t";
		}
		fout << endl;
	}
	fout << "-------------------------------------------------------------------------------------" << endl;
}

AuthorForestLayer* AuthorManager::getClusters(string name) {
	int nid = nm.getNameId(name);
	if (nid == -1)
		return NULL;
	clusterSingle(nid);
	return &outputLayers[nid];
}

bool AuthorManager::outpuCoauthorSize(string name) {
	string nname = norm(name);
	int nid = nm.getNameId(nname);
	if (nid == -1)
		return false;
	set<int> coaus;
	AuthorForestLayer &layer = outputLayers[nid];
	vector<NonLeafAuthor *> &authors = layer.authors;
	int authorsize = layer.authors.size();
	for (int ai = 0; ai < authorsize; ai++) {
		NonLeafAuthor *author = authors[ai];
		for (map<NonLeafAuthor*, two_int>::iterator aite = author->a2s->begin(); aite != author->a2s->end(); aite++) {
			if (aite->second.f1 > 0) {
				coaus.insert(aite->first->nid);
			}
		}
	}
	cout << name << "\tsize\t" << coaus.size() << endl;
	return true;
}

int AuthorManager::getInitAuthorNumber(int nid) {
	if (author_forests[nid].size() == 0)
		return 1;
	AuthorForestLayer &layer = author_forests[nid][0];
	return layer.authors.size();
}


void AuthorManager::outputAllCluster(string path) {
	ofstream fout(path.c_str());
	for (int nid = 0; nid < namesize; nid++) {
		fout << nid;
		if (author_forests[nid].size()>0) {
			AuthorForestLayer & last_layer = author_forests[nid][author_forests[nid].size() - 1];
			int aid = 0;
			for (; aid < last_layer.authors.size();aid++) {
				NonLeafAuthor * author = last_layer.authors[aid];
				for (auto paper : author->papers)
					fout << "\t" << paper->id << ":" << aid;
			}
		}
		fout << endl;
	}
}

void AuthorManager::calInitAuthorNumbers() {
	int *assigned = new int[1024*1024];//BUG for MAG
	min_author_nums.clear();
	for (int nid = 0; nid < namesize; nid++) {
		string now_name = nm.getName(nid);
		if (leafAuthorVectors[nid].size() <= 1)
			min_author_nums.push_back(1);

		vector<Author*> &authorVector = leafAuthorVectors[nid];
		for (int aid = 0; aid < authorVector.size(); aid++)
			assigned[aid] = -1;
		for (int aid = 0; aid < authorVector.size(); aid++){
			if (assigned[aid] != -1)
				continue;
			assigned[aid] = aid;
			for (int aid_j = aid + 1; aid_j < authorVector.size(); aid_j++) {
				if (assigned[aid_j] != -1)
					continue;
				if (isSim(authorVector[aid], authorVector[aid_j]))
					assigned[aid_j] = aid;
			}
		}
		uniFind(assigned, authorVector.size());
		set<int> labels;
		for (int aid = 0; aid < authorVector.size(); aid++)
			labels.insert(assigned[aid]);
		if (labels.size()>0)
			min_author_nums.push_back(labels.size());
		else
			min_author_nums.push_back(1);
	}
	delete [] assigned;
}

int AuthorManager::getMinAuthorNums(int nid) {
	return min_author_nums[nid];
}

/*
* calculate the similarity of two authors
*/
bool AuthorManager::isSim(Author *ai, Author *aj) {
	int nid = ai->nid;
	// ai and aj has more than one same coauthor name
	set<int> i_coauthors;
	for (int i = 0; i < ai->paper->authorsize; i++) {
		if (ai->paper->authors[i]->nid != nid)
			i_coauthors.insert(ai->paper->authors[i]->nid);
	}
	
	//same coauthor
	for (int j = 0; j < aj->paper->authorsize; j++) {
		if (i_coauthors.find(aj->paper->authors[j]->nid) != i_coauthors.end())
			return true;
	}

	//same keyword
	set<int> i_kws;
	for (int ki = 0; ki < ai->paper->kwdsize; ki++) {
		i_kws.insert(ai->paper->kws[ki]);
	}

	for (int kj = 0; kj < aj->paper->kwdsize; kj++) {
		if (i_kws.find(aj->paper->kws[kj]) == i_kws.end())
			return true;
	}


	//same venue
	if (ai->paper->vid == -1 && ai->paper->vid == aj->paper->vid &&vm.getsize(ai->paper->vid) < 10000)
		return true;

	if (vm.isSim(ai->paper->vid, aj->paper->vid))
		return true;

	//same co_2 hop author
	for (int j = 0; j < aj->paper->authorsize; j++) {
		int cid = aj->paper->authors[j]->nid;
		if (nid == cid)
			continue;
		for (auto i_cid : i_coauthors) {
			if (nm.id_2_coauthor_ids[i_cid].find(cid) != nm.id_2_coauthor_ids[i_cid].end())
				return true;
		}
	}
	return false;
}