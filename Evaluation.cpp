#include"Evaluation.h"
extern AuthorManager am;
extern NameManager nm;
Evaluation::Evaluation() {
	name2fout = NULL;
	name2gt = NULL;
	names = NULL;
}

void Evaluation::init(const string & i_gtDir, const string &resultPath, const string & resultsDir) {

	gtDir = i_gtDir;

	cout << "init evaluation" << "\r";
	names = new vector<string>();
	name2gt = new map<string, map<int, int>*>();
	name2fout = new map<string, ofstream*>();
	fout = new ofstream(resultPath);

	getFiles(gtDir, *names);
	cout << "names->size()\t" << names->size() << endl;
	for (int ni = 0; ni<names->size(); ni++) {
		map<int, int> *gt = new map<int, int>();
		string temp = gtDir + (*names)[ni];
		readgt(temp, *gt);
		string name = (*names)[ni].substr(0, (*names)[ni].length() - 4);
		string nname = norm(name);
		(*name2gt)[nname] = gt;
		string apath = resultsDir + (*names)[ni];
		ofstream *oftemp = new ofstream(apath);
		(*name2fout)[nname] = oftemp;
	}
	cout << "init evaluation compelted" << endl;
}

Evaluation::~Evaluation() {
	//clear();
}

void Evaluation::clear() {
	if (name2fout != NULL) { delete names; name2fout = NULL; };
	if (name2gt != NULL) { delete name2gt; name2gt = NULL; };
	if (names = NULL) { delete names; names = NULL; }
}

void Evaluation::evaluate() {

	int ttp = 0, tfp = 0, ttn = 0, tfn = 0;
	float sumf1=0, sumpre=0, sumrec = 0;
	for (int ni = 0; ni<names->size(); ni++) {
		//getresults from am
		map<int, int> res;
		string name = (*names)[ni].substr(0, (*names)[ni].length() - 4);
		AuthorForestLayer *layer = am.getClusters(norm(name));
		if (layer == NULL)
			continue;
		vector<NonLeafAuthor *> &authors = layer->authors;
		int authorsize = authors.size();

		for (int ai=0; ai<authorsize; ai++) {
			NonLeafAuthor *author = authors[ai];
			int aid = author->id;
			for (vector<Paper *>::iterator pite = author->papers.begin(); pite != author->papers.end(); pite++) {
				(res)[(**pite).id] = aid;
			}
		}


		string tempname = (*names)[ni].substr(0, (*names)[ni].length() - 4);
		string nname = norm(tempname);
		ofstream &fou = *(*name2fout)[nname];
		map<int, int> *gt = (*name2gt)[nname];
		int tp = 0, fp = 0, tn = 0, fn = 0, total = 0;
		float pre = 0, recall = 0, f1 = 0;
		vector<string> fps;
		vector<string> fns;
		for (map<int, int>::iterator ite = gt->begin(); ite != gt->end(); ite++) {
			map<int, int>::iterator itej = ite;
			int i = (ite->first);
			int gti = ite->second;
			if (res.find(i) == res.end())
				continue;
			int resi = (res.find(i)->second);
			for (itej++; itej != gt->end(); itej++) {
				int j = itej->first;
				int gtj = itej->second;
				if (res.find(j) == res.end())
					continue;
				int resj = (res.find(j)->second);
				if (gti == gtj && resi == resj)
					tp += 1;
				else if (gti != gtj && resi != resj) {
					tn += 1;
					//fou<<"tn\t"<<i<<"\t"<<j<<"\t"<<endl;
				}
				else if (gti != gtj && resi == resj) {
					fp += 1;
					char temp[128];
#if COMPILE_LINUX
					sprintf(temp, "%d\t%d", i, j);


#else
					sprintf_s(temp, "%d\t%d", i, j);
#endif

					string s(temp);
					fps.push_back(s);
				}
				else {
					fn += 1;
					//fou<<"fn\t"<<i<<"\t"<<j<<"\t"<<endl;
					char temp[128];
#if COMPILE_LINUX
					sprintf(temp, "%d\t%d", i, j);


#else
					sprintf_s(temp, "%d\t%d", i, j);
#endif

					string s(temp);
					fns.push_back(s);
				}
			}
		}
		//cout<<"output f1"<<endl;
		total = fn + fp + tn + tp;
		if (tp == 0 && fp == 0) {
			pre = 1;
			recall = 0;
			f1 = 0;
		}
		else if (tp == 0) {
			pre = 0;
			recall = 0;
			f1 = 0;
		}
		else {
			pre = tp * 1.0 / (tp + fp);
			recall = tp * 1.0 / (tp + fn);
			f1 = 2 * pre*recall / (pre + recall);

		}
		std::cout <<(*names)[ni] << "\t" << res.size()<<"\t"<< pre << "\t" << recall << "\t" << f1 << "\n";
		(*fout) << (*names)[ni] << "\t" << res.size()<<"\t" << pre << "\t" << recall << "\t" << f1 << "\n";
		sumf1+=f1;
		sumpre += pre;
		sumrec += recall;
	}
	(*fout) <<"average f1 "<<(sumf1/names->size())<<endl;
	cout<<(sumpre/names->size())<<"\t"<<(sumrec/names->size())<<"\t"<<(sumf1/names->size())<<endl;
	fout->close();
}

void Evaluation::evaluate(string name, AuthorForestLayer *layer, double threshold) {

	map<int, int>* gt = (*name2gt)[name];
	map<int, int> res;
	
	vector<NonLeafAuthor *> &authors = layer->authors;
	int authorsize = authors.size();

	for (int ai = 0; ai<authorsize; ai++) {
		NonLeafAuthor *author = authors[ai];
		int aid = author->id;
		for (vector<Paper *>::iterator pite = author->papers.begin(); pite != author->papers.end(); pite++) {
			(res)[(**pite).id] = aid;
		}
	}
	int tp = 0, fp = 0, tn = 0, fn = 0, total = 0;
	float pre = 0, recall = 0, f1 = 0;
	for (map<int, int>::iterator ite = gt->begin(); ite != gt->end(); ite++) {
		map<int, int>::iterator itej = ite;
		int i = (ite->first);
		int gti = ite->second;
		if (res.find(i) == res.end())
			continue;
		int resi = (res.find(i)->second);
		for (itej++; itej != gt->end(); itej++) {
			int j = itej->first;
			int gtj = itej->second;
			if (res.find(j) == res.end())
				continue;
			int resj = (res.find(j)->second);
			if (gti == gtj && resi == resj)
				tp += 1;
			else if (gti != gtj && resi != resj)
				tn += 1;
			else if (gti != gtj && resi == resj)
				fp += 1;
			else
				fn += 1;
		}
	}
	total = fn + fp + tn + tp;
	if (tp == 0 && fp == 0) {
		pre = 1;
		recall = 0;
		f1 = 0;
	}
	else if (tp == 0) {
		pre = 0;
		recall = 0;
		f1 = 0;
	}
	else {
		pre = tp * 1.0 / (tp + fp);
		recall = tp * 1.0 / (tp + fn);
		f1 = 2 * pre*recall / (pre + recall);

	}
	(*fout) << name << "\t" << pre << "\t" << recall << "\t" << f1 << "\tthreshold\t" << threshold << endl;
	cout << "tp:" << tp << "\ttn:" << tn << "\tfp:" << fp << "\tfn:" << fn << endl;
	std::cout << name << "\t" << pre << "\t" << recall << "\t" << f1 << "\tthreshold\t" << threshold << endl;
	(*fout).flush();

	//if(name=="wei wang"&&pre<0.5){
	//	system("pause");
	//}

}

void Evaluation::evaluate(string name, AuthorForestLayer *layer) {
	map<int, int>* gt = (*name2gt)[name];
	map<int, int> res;
	vector<NonLeafAuthor *> &authors = layer->authors;
	int authorsize = authors.size();

	for (int ai = 0; ai<authorsize; ai++) {
		NonLeafAuthor *author = authors[ai];
		int aid = author->id;
		for (vector<Paper *>::iterator pite = author->papers.begin(); pite != author->papers.end(); pite++) {
			(res)[(**pite).id] = aid;
		}
	}
	int tp = 0, fp = 0, tn = 0, fn = 0, total = 0;
	float pre = 0, recall = 0, f1 = 0;
	for (map<int, int>::iterator ite = gt->begin(); ite != gt->end(); ite++) {
		map<int, int>::iterator itej = ite;
		int i = (ite->first);
		int gti = ite->second;
		if (res.find(i) == res.end())
			continue;
		int resi = (res.find(i)->second);
		for (itej++; itej != gt->end(); itej++) {
			int j = itej->first;
			int gtj = itej->second;
			if (res.find(j) == res.end())
				continue;
			int resj = (res.find(j)->second);
			if (gti == gtj && resi == resj)
				tp += 1;
			else if (gti != gtj && resi != resj)
				tn += 1;
			else if (gti != gtj && resi == resj)
				fp += 1;
			else
				fn += 1;
		}
	}
	total = fn + fp + tn + tp;
	if (tp == 0 && fp == 0) {
		pre = 1;
		recall = 0;
		f1 = 0;
	}
	else if (tp == 0) {
		pre = 0;
		recall = 0;
		f1 = 0;
	}
	else {
		pre = tp * 1.0 / (tp + fp);
		recall = tp * 1.0 / (tp + fn);
		f1 = 2 * pre*recall / (pre + recall);

	}
	(*fout) << name << "\t" << pre << "\t" << recall << "\t" << f1 << endl;
	std::cout << name << "\t" << pre << "\t" << recall << "\t" << f1 << "\n";
	(*fout).flush();
}

void Evaluation::readgt(string& path, map<int, int>& gt) {
	ifstream fin(path);
	if(!fin){
		cout<<path<<"not found"<<endl;
		return;
	}
	int label = 0;
	while (!fin.eof()) {
		string temp;
		getline(fin, temp);
		vector<string> ids;
		split(ids, temp, '\t');
		for (int i = 0; i<ids.size(); i++)
			gt[atoi(ids[i].c_str())] = label;
		label++;
	}
	fin.close();
}