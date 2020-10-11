#include"node.h"


extern KeywordManager km;
extern PaperManager pm;
extern AuthorManager am;
extern VenueManager vm;
extern AffiliationManager afm;
extern NameManager nm;

void PaperManager::clear() {
	map<string, int>().swap(title2id);
	map<int, Paper*>().swap(id2paper);
	vector<string>().swap(titles);

	for (vector<Paper*>::iterator vite = allpaper.begin(); allpaper.end() != vite;) {
		vector<Paper*>::iterator vdelete = vite++;
		Paper * pa = *vdelete;
		delete pa;
	}
	vector<Paper*>().swap(allpaper);
	cout << "\t\t\t\t\t\t pm all paper cleared\r";
}

PaperManager::~PaperManager() {
	//clear();
}

void PaperManager::init(const string & fpath) {
	readfile(fpath);
}

void PaperManager::initMAG(const string & fpath) {

	MAGID = 0;

	time_t begin, end;
	int index = 0;

	ifstream pafile(fpath);
	if (!pafile)
		cout << fpath<<"\t not found @ readfile @ PaperManager" << endl;
	string tmp;
	int count = 0;
	time(&begin);
	int aumax = 0;
	while (!pafile.eof()) {
		if (++count % 10000 == 0) {
			time(&end);
			std::cout << count << "\t" << difftime(end, begin) << " \r";
			break;
		}

		getline(pafile, tmp);//word
		vector<string> kws;
		split(kws, tmp, ',');

		getline(pafile, tmp);
		vector<string> aus;
		split(aus, tmp, (','));

		getline(pafile, tmp);
		string venue = tmp;

		getline(pafile, tmp);
		int year = std::atoi(tmp.c_str());

		int id = MAGID++;

		//skip the paper who's author number is bigger than 100
		if (aus.size()>100) {
			getline(pafile, tmp);
			getline(pafile, tmp);
			continue;
		}
		Paper *pa = new Paper(id);
		index++;
		pa->setyear(year);

		for (vector<string>::iterator ite = aus.begin(); ite != (aus).end(); ite++) {
			string nname = norm((*ite));
			if (nname.length()>2 && nname.find("staff") == -1) {
				pa->addAuthor(am.addAuthor(nname, pa));
				//am.addAuthor(nname,pa);
			}
		}

		// add keyword
		for (vector<string>::iterator ite = kws.begin(); ite != (kws).end(); ite++) {
			if ((*ite).length()>2) {
				int kid = km.getID(*ite);
				if (kid != -1)
					pa->addKeyword(kid);
			}
		}
		pa->setKeywordSize();

		title2id.insert(map<string, int>::value_type("", id));
		id2paper.insert(map<int, Paper*>::value_type(id, pa));
		int vid = vm.getID(venue);
		pa->setVenue(vid);
		pa->arraylize();
		nm.addCoauthor(pa);
		pid++;
		addPaper(pa);
		getline(pafile, tmp);
		getline(pafile, tmp);
	}

	time(&end);
	std::cout << count << "\t" << difftime(end, begin) << endl;
	pafile.close();
}

void PaperManager::addPaper(Paper * pa) {
	allpaper.push_back(pa);
}

void PaperManager::readfile(const string &fpath) {
	time_t begin, end;
	int index = 0;
	ifstream pafile(fpath);
	if (!pafile)
		cout << fpath << "\t not found @ readfile @ PaperManager" << endl;
	string tmp;
	while (true) {
		getline(pafile, tmp);
		if (tmp.find("<publication>") != -1)
			break;
	}
	int count = 0;
	time(&begin);
	int aumax = 0;
	while (!pafile.eof()) {
		if (++count % 100000 == 0) {
			time(&end);
			std::cout << count << "\t" << difftime(end, begin) << "\r";
		}

		string title;
		getline(pafile, title);
		getcontext(title);

		title = norm(title);
		getline(pafile, tmp);//word
		vector<string> kws;
		getcontext(tmp);

		split(kws, tmp, ',');

		getline(pafile, tmp);
		getcontext(tmp);
		int year = std::atoi(tmp.c_str());
		getline(pafile, tmp);
		
		vector<string> aus;
		getcontext(tmp);
		split(aus, tmp, (','));
		getline(pafile, tmp);
		getcontext(tmp);
		string venue = tmp;
		//id
		getline(pafile, tmp);
		getcontext(tmp);
		int id = std::atoi(tmp.c_str());
		titles.push_back(title);
		//skip the paper who's author number is bigger than 100
		if (aus.size()>10 || aus.size() <= 1) {
			getline(pafile, tmp);
			getline(pafile, tmp);
			continue;
		}
		Paper *pa = new Paper(id);
		index++;
		pa->setyear(year);

		for (vector<string>::iterator ite = aus.begin(); ite != (aus).end(); ite++) {
			string nname = norm((*ite));
			if (nname.length()>2 && nname.find("staff") == -1) {
				pa->addAuthor(am.addAuthor(nname, pa));
			}
		}

		// add keyword
		for (vector<string>::iterator ite = kws.begin(); ite != (kws).end(); ite++) {
			if ((*ite).length()>2) {
				int kid = km.getID(*ite);
				if (kid != -1)
					pa->addKeyword(kid);
			}
		}
		pa->setKeywordSize();
		
		title2id.insert(map<string, int>::value_type(title, id));
		id2paper.insert(map<int, Paper*>::value_type(id, pa));
		int vid = vm.getID(venue);
		pa->setVenue(vid);
		pa->arraylize();
		pid++;
		addPaper(pa);
		getline(pafile, tmp);
		getline(pafile, tmp);
	}

	time(&end);
	std::cout << count << "\t" << difftime(end, begin) << endl;
	pafile.close();
}

void PaperManager::readMAG(const string &fpath, const string & paaPath) {
	map<string, int> magid2id;
	ifstream pafile(fpath);
	if (!pafile)
		cout << fpath << "not found" << endl;
	string tmp;
	int count = 0;
	while (!pafile.eof()) {
		string tmp;
		getline(pafile, tmp);
		if (tmp.size() <= 1)
			break;
		vector<string> ts;
		split(ts, tmp, '\t');
		string id = ts[0];
		string ntitle = ts[1];
		if (title2id.find(ntitle) != title2id.end()) {
			magid2id.insert(map<string, int>::value_type(ts[0], title2id.find(ntitle)->second));
		}
	}
	pafile.close();


	pafile = ifstream(paaPath);
	count = 0;
	while (!pafile.eof()) {
		string tmp;
		getline(pafile, tmp);
		if (tmp.size() <= 1)
			break;
		vector<string> ts;
		split(ts, tmp, '\t');
		string magid = ts[0];
		if (magid2id.find(magid) == magid2id.end())
			continue;
		int id = magid2id.find(magid)->second;
		string author = ts[1];
		string aff = ts[2];
		Paper * p1 = id2paper.find(id)->second;
		p1->author2aff.insert(map<string, string>::value_type(author, aff));
		count += 1;
	}
	cout << "MAG Affiliation \t" << count << endl;
	pafile.close();

	//fpath = "D:/Windows_User/Desktop/MAG/PaperKeywords.txt";
	//pafile = ifstream(fpath);
	//count=0;
	//while(!pafile.eof()){
	//	string tmp;
	//	getline(pafile,tmp);
	//	if(tmp.size()<=1)
	//		break;
	//	vector<string> ts;
	//	split(ts,tmp,'\t');
	//	string magid = ts[0];
	//	if(magid2id.find(magid)==magid2id.end())
	//		continue;
	//	int id = magid2id.find(magid)->second;
	//	string keyword = ts[1];
	//	Paper * pa = id2paper.find(id)->second;
	//	int kid = km.getID(ts[1]);
	//	if(kid!=-1)
	//		pa->addKeyword(kid);
	//	count+=1;
	//}
	//cout<<"MAG keyword \t"<<count<<endl;
	//pafile.close();



}

