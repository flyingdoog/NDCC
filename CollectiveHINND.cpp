#include"CollectiveHINND.h"

string dataPath = DATA_DIR;

void clear() {
	pm.clear();
	vm.clear();
	nm.clear();
	km.clear();
	elv.clear();
	am.clear();

	cout << "manager cleared OK" << endl;
}
//paramaters
double MINSIM, MAXSIM, TThres, VThres, Theta1;

double preprocess(const string & parOutpath, const string & datset) {
	string i_gtDir = dataPath + datset + "/groudtruth/";
	string resultPath = parOutpath;
	string resultsDir = dataPath + "result/HINND_" + datset + "/";
	elv.init(i_gtDir, resultPath, resultsDir);

	time_t inti, begin, end;
	time(&inti);

	//affiliation manager, using out source
	//string affStatisticPath = "D:/Windows_User/Desktop/MAG/aff_statistic.txt";
	//afm.init(affStatisticPath);

	string stopwordPath1 = dataPath + "stopword/stopword.txt";//on in 等字符
	string stopwordPath2 = dataPath + "stopword/stopword2.txt";
	string kwMapPath = dataPath + datset + "/map/id2kw.txt";
	string ksimiPath = dataPath + datset + "/ksimi.txt";//k的相似性

	//ksimiPath = dataPath + datset + "/ksimi.txt";//k的相似性
	km.init(TThres, stopwordPath1, stopwordPath2, kwMapPath, ksimiPath);

	//read vsimi
	string vpapPath = dataPath + datset + "/map/id2venue.txt";//id和venue的映射关系
	string vsimiPath = dataPath + datset + "/vsimi.txt";//venue的相似性
	vm.init(VThres, vpapPath, vsimiPath);
	//cout<<"read venue completed"<<"\r";


	//read name ambiguity
	string chienseNamepath = dataPath + datset + "/ambiguity.txt";//奇异值
	string nameAmbiguityPath = dataPath + datset + "/chinesename.txt";//中文名字
	nm.init(chienseNamepath, nameAmbiguityPath);
	cout << "read name completed" << "\r";

	time(&begin);

	cout << "MINISIMI\t" << MINSIM << endl;
	am.init(MINSIM, MAXSIM);
	cout << "am initiation completed" << endl;

	//if DEBUG ,close DEBUG Files
	nm.fileClose();
	vm.fileClose();

	cout<<"preprocess completed"<<endl;
	return difftime(end, inti);
}

double run(const string & i_filePath, const string & datset) {
	time_t inti, begin, end;

	if (datset == "MAG")
		pm.initMAG(i_filePath);
	else
		pm.init(i_filePath);

	cout << "begin to create atom"<< endl;
	time(&inti);
	am.createAtom();
	cout << "creat atom done, begin to estimateAuthorNumber" << endl;
	nm.estimateAuthorNumber(am);
	cout << "estimateAuthorNumber done, begin to calRs" << endl;

	//calcualte the relational strength for each node
	am.calRs();
	cout << "calRs done, begin to initSim" << endl;

	am.initSim();
	cout << "initSim done, begin to iterate" << endl;

	am.setThres(Theta1);
	am.iterate();

	time(&end);
	cout << "iterate done \t" << difftime(end, inti) << endl;
	am.setAlltoOld();
	return difftime(end, inti);

}

double incremental(const string &increPath) {
	time_t inti, begin, end;
	time(&begin);
	pm.init(increPath);
	time(&end); cout << "begin to create atom " << difftime(end,begin)<< endl;
	time(&inti);
	am.inc_createAtom();
	time(&end); cout << "begin to estimateAuthorNumber " << difftime(end, begin) << endl;
	nm.estimateAuthorNumber(am);
	time(&end); cout << "begin to set1stLayerNbrsUpdated " << difftime(end, begin) << endl;
	am.set1stLayerNbrsUpdated();
	time(&end); cout << "begin to calRs " << difftime(end, begin) << endl;
	time(&end); cout << "begin to iterate " << difftime(end, begin) << endl;
	am.inc_iterate();
	time(&end); cout << "done" << difftime(end, begin) << endl;
	cout << "total" << difftime(end, inti) << endl;
	return  difftime(end, inti);
}

void evaluate() {
	elv.evaluate();
}

int main(int argc, char ** argv) {
	string datset = "ACM";
	MINSIM = 1e-6;
	MAXSIM = 1e-3;
	VThres = 0.02;
	TThres = 0.75;
	Theta1 = 0.05;

	string output = datset+"Temp";
	if (argc == 7) {
		datset = string(argv[1]);
		TThres = atof((const char*)(argv[2]));
		VThres = atof((const char*)(argv[3]));
		MINSIM = atof((const char*)(argv[4]));
		MAXSIM = 1e-3;
		Theta1 = atof((const char*)(argv[5]));
		output = string(argv[6]);
	}

	cout << "TThres\t" << TThres << endl;
	cout << "VThres\t" << VThres << endl;
	cout << "MINSIM\t" << MINSIM << endl;
	cout << "Theta1\t" << Theta1 << endl;

	string basepath = DATA_DIR;
	string timePath = basepath + "/timeRecord_HINND.txt";
	ofstream tim(timePath.c_str(), ios::app);
	string filePath = basepath + datset + "/" + datset + ".xml";//DATA_DIR;
	string parOutpath = basepath + "result/" + output + ".txt";// 文件名字格式:参数名称 取值 数据集;

	cout << "FilePath\t" << filePath << endl;
	cout << "parOutpath\t" << parOutpath << endl;


	string basicPath = basepath + datset + "/Incremental/base.xml";
	string incrementalPath = basepath + datset + "/Incremental/incremental.xml";

	double preprocessTime = preprocess(parOutpath, datset);
	double runtime = run(filePath, datset);
	tim << "Running time:\t" + output + "\t" << runtime << std::endl;
	
	// string clusteringPath = basepath + datset + "/cluteringResults/clusters_base.txt";
	// am.outputAllCluster(clusteringPath);

// 	double incrRuntime =0;
// //	incrRuntime = incremental(incrementalPath);
// 	tim << "incrRuntime time:\t" + output + "\t" << incrRuntime << std::endl;

	// clusteringPath = basepath + datset + "/cluteringResults/clusters_inc.txt";
	// am.outputAllCluster(clusteringPath);

	evaluate();
	tim.close();
	return 0;
}
