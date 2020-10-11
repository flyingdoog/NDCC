#include"node.h"
struct vpair;
extern VenueManager vm;
extern KeywordManager km;
extern NameManager nm;

struct author_pair_simi {
	int id1, id2;
	float sim;


};

Author::Author(int nid1, AUTHOR_NUMBER_TYPE id1) {
	nid = nid1;
	//cout<<name<<endl;
	id = id1;
	updated = true;// new
	father = NULL;
}

//
//int Author::getcoauthorsNum(){
//	size_t num=0;
//	for(vector<Paper *>::iterator ite=papers.begin();ite!=papers.end();ite++)
//		num+=(*ite)->authorsize-1;
//	return (int)num;
//}

void Author::addPaper(Paper *pa) {
	paper = pa;
}

