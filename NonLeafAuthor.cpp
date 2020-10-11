#include"node.h"
extern NameManager nm;
extern VenueManager vm;
extern KeywordManager km;
#define DEBUG true
NonLeafAuthor::NonLeafAuthor(int nid1, AUTHOR_NUMBER_TYPE id1) :Author(nid1, id1) {
	begin = 0;
	end = 0;
	a2s = NULL;
	t2s = NULL;
	v2s = NULL;
	t2s_simi = NULL;
	v2s_simi = NULL;
	vsize = 0;
	ksize = 0;
	del = false;
	coauthor_update = false;
}


NonLeafAuthor::~NonLeafAuthor() {
	if (a2s != NULL)
		delete a2s;
	if (t2s != NULL)
		delete t2s;
	if (v2s != NULL)
		delete v2s;
	if (t2s_simi != NULL)
		delete t2s_simi;
	if (v2s_simi != NULL)
		delete v2s_simi;
}

void NonLeafAuthor::calRs() {
	if (a2s == NULL)
		a2s = new map<NonLeafAuthor*, two_int>();
	else
		map<NonLeafAuthor*, two_int>().swap(*a2s);
	if (t2s == NULL)
		t2s = new map<int, float>();
	else
		map<int, float>().swap(*t2s);
	if (v2s == NULL)
		v2s = new map<int, float>();
	else
		map<int, float>().swap(*v2s);
	if (t2s_simi == NULL)
		t2s_simi = new map<int, float>();
	else
		map<int, float>().swap(*t2s_simi);
	if (v2s_simi == NULL)
		v2s_simi = new map<int, float>();
	else
		map<int, float>().swap(*v2s_simi);
	cal_A();
	cal_T();
	cal_V();
}

//calculate APA, APAPA
void NonLeafAuthor::cal_A() {
	vector<Paper*>::iterator ite = papers.begin();
	while (ite != papers.end()) {
		Paper* p1 = (*ite);
		if (p1->authorsize == 0) {
			ite++;
			continue;
		}
		Author ** authors = p1->authors;
		int aite = 0;
		int score = 1;
		while (aite<p1->authorsize) {
			NonLeafAuthor* a1 = authors[aite]->father;
	
			if (a1==NULL || a1->nid == nid) {
				aite++;
				continue;
			}
			map<NonLeafAuthor*, two_int>::iterator mite = a2s->find(a1);
			if (mite == a2s->end()) {//not found
				two_int scores;
				scores.f1 = score;
				scores.f2 = 0;
				a2s->insert(map<NonLeafAuthor*, two_int>::value_type(a1, scores));
			}
			else {
				(*a2s)[a1].f1 += score;
			}

			//20170724
			if (nm.getAmb(a1->nid)>100 || a1->papers.size() >= 300)
				;
			else
				//APAPA
				for (vector<Paper*>::iterator pite2 = a1->papers.begin(); pite2 != a1->papers.end(); pite2++) {
					if (*pite2 == p1)
						continue;
					Paper * p2 = *pite2;
					Author ** authors2 = p2->authors;
					for (int i = 0; i<p2->authorsize; i++) {
						if (authors2[i]->father==NULL||authors2[i]->nid == a1->nid || authors2[i]->nid == nid)
							continue;
						map<NonLeafAuthor*, two_int>::iterator mite2 = a2s->find(authors2[i]->father);
						if (mite2 == a2s->end()) {//not found
							two_int  scores;
							scores.f2 = score;
							scores.f1 = 0;
							a2s->insert(map<NonLeafAuthor*, two_int>::value_type(authors2[i]->father, scores));
						}
						else {
							(mite2->second).f2 += score;
						}
					}
				}
			aite++;
		}
		ite++;
	}
}


void NonLeafAuthor::cal_T() {
	vector<Paper*>::iterator ite = papers.begin();
	while (ite != papers.end()) {
		Paper *p1 = *ite;
		if (p1->kwd_fos_size == 0) {
			ite++;
			continue;
		}
		ksize += p1->kwd_fos_size;
		int * kws = p1->kws;
		double score = 1.0;//p1->kwd_fos_size;
		for (int ki = 0; ki<p1->kwd_fos_size; ki++) {
			map<int, float>::iterator mite = t2s->find(kws[ki]);
			if (mite == t2s->end()) {//not found
				t2s->insert(map<int, float>::value_type(kws[ki], score));
			}
			else {
				(*t2s)[kws[ki]] += score;
			}
		}
		ite++;
	}
	if (t2s->size()>0) {
		km.addsimi(*t2s, *t2s_simi);
	}
}

//APV_SIMI
void NonLeafAuthor::cal_V() {
	vector<Paper*>::iterator ite = papers.begin();
	while (ite != papers.end()) {
		Paper *p1 = *ite;
		int vid = p1->vid;
		if (vid == -1) {
			ite++;
			continue;
		}
		vsize++;
		int score = 1;
		map<int, float>::iterator mite = v2s->find(vid);
		if (mite == v2s->end()) {//not found
			v2s->insert(map<int, float>::value_type(vid, score));
		}
		else {
			(*v2s)[vid] += score;
		}
		ite++;
	}
	if (v2s->size()>0) {
		vm.addsimi(*v2s, *v2s_simi);
		//norm_V();
	}
}


void NonLeafAuthor::merge(Author* aj) {

	if(a2s ==NULL)	a2s = new map<NonLeafAuthor*, two_int>();
	if (t2s == NULL)	t2s = new map<int, float>();
	if (v2s == NULL)	v2s = new map<int, float>();
	if (t2s_simi == NULL)	t2s_simi = new map<int, float>();
	if (v2s_simi == NULL)	v2s_simi = new map<int, float>();


	//update the begin and end year
	if (aj->paper->year<begin || begin ==0)
		begin = aj->paper->year;
	if (aj->paper->year >end || end==0)
		end = aj->paper->year;


	//add paper & update Author
	papers.push_back(aj->paper);
	authors.push_back(aj);
	aj->father = this;
	aj->updated = false;
	updated = true;
}


void NonLeafAuthor::merge(NonLeafAuthor* aj) {
	if (DEBUG && aj->papers.size() == 0) {
		cout << "ERROR MERGE @NonLeafAuthor " << endl;
		return;
	}
	authors.push_back(aj);

	if (a2s == NULL)	a2s = new map<NonLeafAuthor*, two_int>();
	if (t2s == NULL)	t2s = new map<int, float>();
	if (v2s == NULL)	v2s = new map<int, float>();
	if (t2s_simi == NULL)	t2s_simi = new map<int, float>();
	if (v2s_simi == NULL)	v2s_simi = new map<int, float>();


	//update the begin and end year
	if (aj->begin<begin)
		begin = aj->begin;
	if (aj->end>end)
		end = aj->end;


	//add paper
	for (vector<Paper*>::iterator ite = aj->papers.begin(); ite != aj->papers.end(); ite++) {
		papers.push_back(*ite);
	}

	if (nm.getAmb(nid)>50 || papers.size()>200)
		;
	else
		//include new 2 hop coauthors 
		for (map<NonLeafAuthor*, two_int>::iterator iteai = a2s->begin(); iteai != a2s->end(); iteai++) {
			if (iteai->second.f1>0) {
				NonLeafAuthor *au1 = iteai->first;
				if (au1->a2s == NULL || au1->nid == nid)
					continue;
				for (map<NonLeafAuthor*, two_int>::iterator iteaj = aj->a2s->begin(); iteaj != aj->a2s->end(); iteaj++)
					if (iteaj->second.f1>0) {
						NonLeafAuthor *au2 = iteaj->first;
						if (au1->nid == au2->nid || au2->nid == nid)
							continue;
						if (au1->a2s->find(au2) == au1->a2s->end()) {
							two_int scores1;
							scores1.f2 = iteaj->second.f1*iteai->second.f1;
							scores1.f1 = 0;
							au1->a2s->insert(map<NonLeafAuthor*, two_int>::value_type(iteaj->first, scores1));
						}
						else
							(*au1->a2s)[au2].f2 += iteaj->second.f1 + iteaj->second.f1*iteai->second.f1;
						if (au2->a2s != NULL)
							(*au2->a2s)[au1] = (*au1->a2s)[au2];
					}
			}
		}

	//a2s
	for (map<NonLeafAuthor*, two_int>::iterator iteai = aj->a2s->begin(); iteai != aj->a2s->end(); iteai++) {
		NonLeafAuthor* s1 = (iteai->first);
		if (s1->a2s != NULL) {
			map<NonLeafAuthor*, two_int>::iterator iteam = s1->a2s->find(aj);
			//update this author in others 
			if (iteam != s1->a2s->end())
				s1->updatea2s(aj, this, iteam->second);
		}

		map<NonLeafAuthor*, two_int>::iterator iteaj = a2s->find(iteai->first);
		if (iteaj == a2s->end()) {
			(*a2s)[(iteai->first)] = iteai->second;
		}
		else {
			(*a2s)[iteai->first] += (iteai->second);
		}
	}
	//t2s
	for (map<int, float>::iterator iteti = aj->t2s->begin(); iteti != aj->t2s->end(); iteti++) {
		map<int, float>::iterator itetj = t2s->find(iteti->first);
		if (itetj == t2s->end()) {
			(*t2s)[(iteti->first)] = iteti->second;
		}
		else {
			(*t2s)[iteti->first] += iteti->second;
		}

	}

	//t2s_simi
	for (map<int, float>::iterator iteti = aj->t2s_simi->begin(); iteti != aj->t2s_simi->end(); iteti++) {
		map<int, float>::iterator itetj = t2s_simi->find(iteti->first);
		if (itetj == t2s_simi->end()) {
			(*t2s_simi)[(iteti->first)] = iteti->second;
		}
		else {
			(*t2s_simi)[iteti->first] += iteti->second;
		}

	}

	vsize += aj->vsize;
	//v2s
	for (map<int, float>::iterator itevi = aj->v2s->begin(); itevi != aj->v2s->end(); itevi++) {
		map<int, float>::iterator itevj = v2s->find(itevi->first);
		if (itevj == v2s->end()) {
			(*v2s)[(itevi->first)] = itevi->second;
		}
		else {
			(*v2s)[itevi->first] += itevi->second;
		}
	}

	//v2s_simi
	for (map<int, float>::iterator itevi = aj->v2s_simi->begin(); itevi != aj->v2s_simi->end(); itevi++) {
		map<int, float>::iterator itevj = v2s_simi->find(itevi->first);
		if (itevj == v2s_simi->end()) {
			(*v2s_simi)[(itevi->first)] = itevi->second;
		}
		else {
			(*v2s_simi)[itevi->first] += itevi->second;
		}
	}

	ksize += aj->ksize;

	//remark all neibors to update
	for (map<NonLeafAuthor*, two_int>::iterator iteai = a2s->begin(); iteai != a2s->end(); iteai++) {
		NonLeafAuthor* s1 = (iteai->first);
		if (!s1->updated) {
			s1->coauthor_update = true;
		}
	}

	updated = true;
	aj->updated = false;
	aj->father = this;
	//aj->papers.clear();//  do we need this?????
}


void NonLeafAuthor::updatea2s(NonLeafAuthor *old, NonLeafAuthor *new1, two_int score) {

	if (a2s->find(old) != a2s->end()) {
		a2s->erase(old);
	}
	else {
		NonLeafAuthor *old_father = old->father;
		if (old_father != NULL && a2s->find(old_father) != a2s->end()) {
			//need to delete the 2hop generated from old_father
			int this_and_father_coauthor_times = a2s->find(old_father)->second.f1;//x+z
			if (this_and_father_coauthor_times != 0) {
				int this_and_old_coauthor_times = score.f1;//z
				int this_and_others_coauthor_times = this_and_father_coauthor_times - this_and_old_coauthor_times;//x
				for (map<NonLeafAuthor*, two_int>::iterator aite = father->a2s->begin(); aite != father->a2s->end(); aite++) {
					if (aite->second.f1 == 0)
						continue;
					if (a2s->find(aite->first) == a2s->end()) {
						cout << "MAYBE A BUG APPEARED." << endl;
						system("pause");
						continue;
					}

					int father_and_nb_coauthor_times = aite->second.f1;
					int old_and_nb_coauthor_times = 0;
					if (old->a2s->find(aite->first) != old->a2s->end() && old->a2s->find(aite->first)->second.f1 > 0)
						old_and_nb_coauthor_times = old->a2s->find(aite->first)->second.f1;
					int other_and_nb_coauthor_times = father_and_nb_coauthor_times - old_and_nb_coauthor_times;
					//split
					two_int nb_scores = a2s->find(aite->first)->second;
					nb_scores.f2 -= this_and_others_coauthor_times * old_and_nb_coauthor_times + this_and_old_coauthor_times * this_and_others_coauthor_times;
					if (nb_scores.f2 <= 0)
						nb_scores.f2 = 0;
					if (nb_scores.f1 == 0 && nb_scores.f2 == 0)
						a2s->erase(aite->first);
				}
			}

			(*a2s)[old_father].f1 -= score.f1;
			(*a2s)[old_father].f2 -= score.f2;
			if ((*a2s)[old_father].f1 == 0 && (*a2s)[old_father].f2 == 0)
				a2s->erase(old_father);
		}
	}

	map<NonLeafAuthor*, two_int>::iterator iteak = a2s->find(new1);
	if (iteak == a2s->end()) {
		a2s->insert(map<NonLeafAuthor*, two_int>::value_type(new1, score));
	}
	else
		(*a2s)[new1] += score;

	coauthor_update = true;
}

void NonLeafAuthor::norm_V() {
	double sum = 0;
	for (map<int, float>::iterator mite = v2s->begin(); mite != v2s->end(); mite++)
		sum += mite->second;
	for (map<int, float>::iterator mite = v2s->begin(); mite != v2s->end(); mite++)
		mite->second = mite->second / sum;

	sum = 0;
	for (map<int, float>::iterator mite = v2s_simi->begin(); mite != v2s_simi->end(); mite++)
		sum += mite->second;
	for (map<int, float>::iterator mite = v2s_simi->begin(); mite != v2s_simi->end(); mite++)
		mite->second = mite->second / sum;
}

void NonLeafAuthor::norm_T() {
	double sum = 0;
	for (map<int, float>::iterator mite = t2s->begin(); mite != t2s->end(); mite++)
		sum += mite->second;
	for (map<int, float>::iterator mite = t2s->begin(); mite != t2s->end(); mite++)
		mite->second = mite->second / sum;
}
