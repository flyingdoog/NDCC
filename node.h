#ifndef _head_node

#define _head_node
#include"tools.h"
#include"Manager.h"
using namespace std;
class Paper;
class node;
class Author;
class node {

};

typedef struct two_float {
	float f1;
	float f2;

	void init() {
		f1 = 0;
		f2 = 0;
	}
	struct two_float operator +(const two_float& t2) const {
		two_float res;
		res.f1 = f1 + t2.f1;
		res.f2 = f2 + t2.f2;
		return res;
	}

	void operator +=(const two_float& t2) {
		f1 += t2.f1;
		f2 += t2.f2;
	}

	struct two_float operator /(const two_float& t2) const {
		two_float res;
		res.f1 = f1 / t2.f1;
		res.f2 = f2 / t2.f2;
		return res;
	}

	struct two_float operator /(const float& t2) const {
		two_float res;
		res.f1 = f1 / t2;
		res.f2 = f2 / t2;
		return res;
	}

}two_float;

typedef struct two_int {
	int f1;
	int f2;

	void init() {
		f1 = 0;
		f2 = 0;
	}
	struct two_int operator +(const two_int& t2) const {
		two_int res;
		res.f1 = f1 + t2.f1;
		res.f2 = f2 + t2.f2;
		return res;
	}

	void operator +=(const two_int& t2) {
		f1 += t2.f1;
		f2 += t2.f2;
	}

	struct two_int operator /(const two_int& t2) const {
		two_int res;
		res.f1 = f1 / t2.f1;
		res.f2 = f2 / t2.f2;
		return res;
	}

	struct two_int operator /(const int& t2) const {
		two_int res;
		res.f1 = f1 / t2;
		res.f2 = f2 / t2;
		return res;
	}

	struct two_float operator /(const float& t2) const {
		two_float res;
		res.f1 = 1.0*f1 / t2;
		res.f2 = 1.0*f2 / t2;
		return res;
	}


}two_int;

typedef struct four_float {
	float f11, f12, f21, f22;

	void init() {
		setValue(0);
	}

	void setValue(float val) {
		f11 = val;
		f12 = val;
		f21 = val;
		f22 = val;
	}

	struct four_float operator +(const four_float& t2) const {
		four_float res;
		res.f11 = f11 + t2.f11;
		res.f12 = f12 + t2.f12;
		res.f21 = f21 + t2.f21;
		res.f22 = f22 + t2.f22;
		return res;
	}

	void operator +=(const four_float& t2) {
		f11 += t2.f11;
		f12 += t2.f12;
		f21 += t2.f21;
		f22 += t2.f22;
	}

	void operator *=(const float t1) {
		f11 *= t1;
		f12 *= t1;
		f21 *= t1;
		f22 *= t1;
	}

	void operator /=(const float t1) {
		f11 /= t1;
		f12 /= t1;
		f21 /= t1;
		f22 /= t1;
	}


	struct four_float operator /(const four_float& t2) const {
		four_float res;
		res.f11 = f11 / t2.f11;
		res.f12 = f12 / t2.f12;
		res.f21 = f21 / t2.f21;
		res.f22 = f22 / t2.f22;
		return res;
	}

	struct four_float operator /(const float& t2) const {
		four_float res;
		res.f11 = f11 / t2;
		res.f12 = f12 / t2;
		res.f21 = f21 / t2;
		res.f22 = f22 / t2;
		return res;
	}

	struct four_float operator *(const float& t2) const {
		four_float res;
		res.f11 = f11 * t2;
		res.f12 = f12 * t2;
		res.f21 = f21 * t2;
		res.f22 = f22 * t2;
		return res;
	}

	float sum() const {
		return f11 + f12 + f21 + f22;
	}


}four_float;


typedef struct four_int {
	int i11, i12, i21, i22;

	void init() {
		setValue(0);
	}

	void setValue(int val) {
		i11 = val;
		i12 = val;
		i21 = val;
		i22 = val;
	}

	struct four_int operator +(const four_int& t2) const {
		four_int res;
		res.i11 = i11 + t2.i11;
		res.i12 = i12 + t2.i12;
		res.i21 = i21 + t2.i21;
		res.i22 = i22 + t2.i22;
		return res;
	}

	void operator +=(const four_int& t2) {
		i11 += t2.i11;
		i12 += t2.i12;
		i21 += t2.i21;
		i22 += t2.i22;
	}

	struct four_int operator /(const four_int& t2) const {
		four_int res;
		res.i11 = i11 / t2.i11;
		res.i12 = i12 / t2.i12;
		res.i21 = i21 / t2.i21;
		res.i22 = i22 / t2.i22;
		return res;
	}

	struct four_int operator /(const int& t2) const {
		four_int res;
		res.i11 = i11 / t2;
		res.i12 = i12 / t2;
		res.i21 = i21 / t2;
		res.i22 = i22 / t2;
		return res;
	}

	struct four_int operator *(const int& t2) const {
		four_int res;
		res.i11 = i11 * t2;
		res.i12 = i12 * t2;
		res.i21 = i21 * t2;
		res.i22 = i22 * t2;
		return res;
	}



}four_int;

class Author;

class Author : public node {

public:
	AUTHOR_NUMBER_TYPE id;
	int nid;
	bool updated;
	Paper* paper;
	Author(int, AUTHOR_NUMBER_TYPE);
	void addPaper(Paper *pa);
	NonLeafAuthor *father;
	bool operator <(const Author & b) const{
			if (nid < b.nid)
				return true;
			else if (nid > b.nid)
				return false;
			else
				return id < b.id;
	}
};

class Paper : public node {
public:
	string aff;
	vector<int> v_kws;
	vector<Author*> v_authors;
	int authorsize, kwdsize, kwd_fos_size;

	set<int> citations;
	set<string> homepages;
	map<string, string> author2aff;


	Author ** authors;
	int * kws;

	//Affiliation *aff;
	int vid;
	//string title;
	int id;
	int year;
	//int label;
	Paper(int id);
	void addAuthor(Author* au);
	void setyear(int);
	void setVenue(int);
	void addKeyword(int);
	void splitTitle(string, set<char> &sps);
	void arraylize();
	void updateAuthor(Author*);
	void outPutKeywords();
	void setKeywordSize();
	void addCitation(int);
};

class NonLeafAuthor : public Author {
private:

	void cal_T();
	void cal_V();
	void norm_V();
	void norm_T();
	void updatea2s(NonLeafAuthor *, NonLeafAuthor *, two_int);
public:
	~NonLeafAuthor();
	int vsize, ksize;
	short begin, end;
	bool del, coauthor_update;
	map<NonLeafAuthor*, two_int> *a2s;
	map<int, float> *t2s;
	map<int, float> *t2s_simi;
	map<int, float> *v2s;
	map<int, float> *v2s_simi;
	vector<Paper*> papers;
	vector<Author*> authors;
public:
	NonLeafAuthor(int nid1, AUTHOR_NUMBER_TYPE id1);
	void cal_A();
	void calRs();
	void merge(Author*);
	void merge(NonLeafAuthor*);

};

#endif