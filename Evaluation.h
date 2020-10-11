#ifndef _Evaluation_head
#define _Evaluation_head
#include"tools.h"
#include"node.h"
class Evaluation {
private:
	ofstream * fout;

	string gtDir;
	map<string, ofstream*>* name2fout;
	map<string, map<int, int>*> *name2gt;
	vector<string> *names;

public:
	Evaluation();
	void clear();
	void init(const string & i_gtDir, const string &resultPath, const string & resultsDir);
	~Evaluation();
	string elvationDir;
	void readgt(string&, map<int, int>&);
	void evaluate();
	void evaluate(string, AuthorForestLayer *layer);
	void evaluate(string, AuthorForestLayer *layer, double);
};
#endif