#include"node.h"

extern KeywordManager km;
Paper::Paper(int id) :id(id) {

}

void Paper::addAuthor(Author* au) {
	v_authors.push_back(au);
}

void Paper::addKeyword(int kid) {
	v_kws.push_back(kid);
}

/*
* set year of this paper,
* set year of this paper's authors
*/
void Paper::setyear(int year1) {
	year = year1;
}

void Paper::setVenue(int vid2) {
	vid = vid2;
}

void Paper::setKeywordSize() {
	kwdsize = v_kws.size();
}


void Paper::arraylize() {
	authorsize = v_authors.size();
	authors = (Author**)malloc(sizeof(Author*)*authorsize);
	vector<Author*>::iterator vite = v_authors.begin();
	for (int i = 0; i<authorsize; i++, vite++) {
		authors[i] = *vite;
	}
	vector<Author*>().swap(v_authors);

	kwd_fos_size = v_kws.size();
	kws = (int*)malloc(sizeof(int)*kwd_fos_size);
	vector<int>::iterator iite = v_kws.begin();
	for (int i = 0; i<kwd_fos_size; i++, iite++) {
		kws[i] = *iite;
	}
	vector<int>().swap(v_kws);
}

void Paper::updateAuthor(Author* au1) {

	for (int i = 0; i<authorsize; i++) {
		if (authors[i]->nid == au1->nid) {
			authors[i] = au1;//¸ü¸Ä
		}
	}
	return;
}


void Paper::addCitation(int c) {
	//	cout<<"add citation\t"<<id<<"\t"<<c<<endl;
	citations.insert(c);
}
