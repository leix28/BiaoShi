#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "TrainTransE.h"
#include "word2vec.h"
#include "header.h"

const int BUFFER_SIZE = 100;

struct Vocab {
	int f;
	std::string word;
};

class Control {
protected:
	std::string name;

	int CalcHash(const char *word, unsigned long long p = 257, unsigned long long hashSize = 10000007) {
		unsigned long long hash = 0;
		for (int i = 0; i < strlen(word); i++)
			hash = hash * p + word[i];
		hash = hash % hashSize;
		return hash;
	}

	void InitialHash(std::vector<int> &hash, unsigned long long hashSize = 10000007) {
		hash.clear();
		for (int i = 0; i < hashSize; i++)
			hash.push_back(-1);
	}

	void addWordHash(int num, const char *word, std::vector<int> &hash, unsigned long long p = 257, unsigned long long hashSize = 10000007) {
		int hashNum = CalcHash(word, p, hashSize);
		while (hash[hashNum] != -1)
			hashNum = (hashNum + 1) % hashSize;
		hash[hashNum] = num;
	}

	int SearchWord(const  std::vector<Vocab> &vocabList, const char *word, const std::vector<int> &hash, unsigned long long p = 257, unsigned long long hashSize = 10000007) {
		int hashNum = CalcHash(word, p, hashSize);
		while (1) {
			if (hash[hashNum] == -1) return -1;
			if (word == vocabList[hash[hashNum]].word) return hash[hashNum];
			hashNum = (hashNum + 1) % hashSize;
		}
		return -1;
	}

	int addWord( std::vector<Vocab> &vocabList, const char *word, std::vector<int> &hash, unsigned long long p = 257, unsigned long long hashSize = 10000007) {
		int hashNum = SearchWord(vocabList, word, hash, p, hashSize);
		if (hashNum != -1) {
			vocabList[hashNum].f+=1;
			return hashNum;
		}
		struct Vocab work;
		work.f = 1;
		work.word = (std::string)(word);
		vocabList.push_back(work);
		addWordHash(vocabList.size()-1, word, hash, p ,hashSize);
		return vocabList.size()-1;
	}

public:
	virtual void AddConfig(std::string, std::string) {};
	virtual void Init() {};
	virtual void Run() {};

	bool Explain(FILE *fin) {
		char ch;
		name = "";
		while (!feof(fin)) {
			ch = fgetc(fin);
			if (ch == '{') break;
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			name = name + ch;
		}
		std::string content, title;
		std::string buffer = "";
		if (ch != '{') return 1;
		while (!feof(fin)) {
			ch = fgetc(fin);
			if (ch == '}') break;
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			if (ch == ':') title = buffer, buffer = ""; else
			if (ch == ';') content = buffer, buffer = "", AddConfig(title, content); else
			buffer = buffer + ch;
		}
		if (ch != '}') return 1;
		return 0;
	}

	~Control() {
	}

};

class Word2VecControl:public Control {

private:
	real rate, sample;
	int cbow, dimension, binary, window, hs, negative, iter, MinimalCount, thread;
	std::string trainFile, outputFile;
	word2vec *work;

public:
	void AddConfig(std::string title, std::string content) {
		if (title == "rate") rate = atof(content.c_str()); else
		if (title == "train") trainFile = content; else
		if (title == "method") {
			if (content == "cbow") cbow = 1; else cbow =0;
		} else
		if (title == "thread") thread = atoi(content.c_str()); else
		if (title == "dimension") dimension = atoi(content.c_str()); else
		if (title == "binary") binary = atoi(content.c_str());
		if (cbow) rate = 0.05;
		if (title == "rate") rate = atof(content.c_str()); else
    	if (title == "output") outputFile = content; else
    	if (title == "window") window = atoi(content.c_str()); else
    	if (title == "sample") sample = atof(content.c_str()); else
    	if (title == "hs") hs = atoi(content.c_str()); else
    	if (title == "negative") negative = atoi(content.c_str()); else
    	if (title == "iter") iter = atoi(content.c_str()); else
    	if (title == "min-count") MinimalCount = atoi(content.c_str());
	}

	void Init() {
		work = new word2vec();
		work->SetDimension(dimension);
		work->SetTrain(trainFile);
		work->SetBinary(binary);
		work->SetMethod(cbow);
		work->SetRate(rate);
		work->SetOutput(outputFile);
		work->SetWindow(window);
		work->SetHs(hs);
		work->SetSample(sample);
		work->SetNegative(negative);
		work->SetThread(thread);
		work->SetIter(iter);
		work->SetMinCount(MinimalCount);
		work->SetIntial();
		work->Run();

	}

	void Run() {
		work->Run();
	}

	~Word2VecControl() {
		delete work;
	}
};

class TransEControl:public Control {

private:
	struct arr {
		int e1, e2, r;
		arr(int a, int b, int c) {
			e1 = a;
			e2 = b;
			r = c;
		}
	};

	struct cmp {
		bool operator()(const arr &a, const arr &b) {
			return (a.e1 < b.e1) || (a.e1 == b.e1 && a.e2 < b.e2) || (a.e1 == b.e1 && a.e2 == b.e2 && a.r < b.r);
		}
	};

	const static int MaxStringLength = 1000;
	const static int hashSize = 10000007;
	const static int hashP =  257;
	int method, npoch, thread, dimension;
	int entitySum, relationSum, fb_num;
	int *fb_h, *fb_l, *fb_r;
	real margin, rate;
	std::string trainFile;
	std::vector<int> hashEntity, hashRelaton;
	std::vector<Vocab> vocabRelation, vocabEntity;
	std::vector<arr> trainQueue;
	real *relationVec, *entityVec;
	TrainTransE *work;


public:
	void AddConfig(std::string title, std::string content) {
		if (title == "margin") margin = atof(content.c_str()); else
		if (title == "rate") rate = atof(content.c_str()); else
		if (title == "train") trainFile = content; else
		if (title == "npoch") npoch = atoi(content.c_str()); else
		if (title == "method") {
			if (content == "bern") method = 1; else method =0;
		} else
		if (title == "thread") thread = atoi(content.c_str());
		if (title == "dimension") dimension = atoi(content.c_str());
	}

	void Init() {
		FILE *fin = fopen(trainFile.c_str(),"r");
		InitialHash(hashEntity, hashSize);
		InitialHash(hashRelaton, hashSize);
		vocabEntity.clear();
		vocabRelation.clear();
		char buffer1[MaxStringLength], buffer2[MaxStringLength], buffer3[MaxStringLength];
		int hx=0;
		while (!feof(fin)) {
			fscanf(fin,"%s", buffer1);
			if (feof(fin)) break;
			fscanf(fin,"%s",buffer2);
			if (feof(fin)) break;
			fscanf(fin,"%s",buffer3);
			if (feof(fin)) break;
			int tmp1 = addWord(vocabEntity, buffer1, hashEntity, hashP, hashSize);
			int tmp2 = addWord(vocabEntity, buffer2, hashEntity, hashP, hashSize);
			int tmp3 = addWord(vocabRelation, buffer3, hashRelaton, hashP, hashSize);
			trainQueue.push_back(arr(tmp1, tmp2, tmp3));
		}
		fclose(fin);
		printf("Size of Relations : %d   Size of Entities : %d\n",vocabRelation.size(), vocabEntity.size());
		sort(trainQueue.begin(), trainQueue.end(), cmp());
		fb_num = trainQueue.size();
		fb_h = (int *)calloc(fb_num, sizeof(int));
		fb_l =   (int *)calloc(fb_num, sizeof(int));
		fb_r = (int *)calloc(fb_num, sizeof(int));
		for (int i = 0; i < fb_num; i++) {
			fb_h[i] = trainQueue[i].e1;
			fb_l[i] = trainQueue[i].e2;
			fb_r[i] = trainQueue[i].r;
		}
		relationVec = (real *)calloc(vocabRelation.size() * dimension , sizeof(real));
		entityVec = (real *)calloc(vocabEntity.size() * dimension, sizeof(real));

		work = new TrainTransE();
		work->SetN(dimension);
		work->SetRate(rate);
		work->SetMargin(margin);
		work->SetNepoch(npoch);
		work->SetMethod(method);
		work->SetThread(thread);
		work->SetRelationNum(vocabRelation.size());
		work->SetEntityNum(vocabEntity.size());
		work->SetTrainFile(fb_h, fb_l, fb_r, fb_num);
	}

	void Run() {
		work->Run(entityVec, relationVec);
	}

	~TransEControl() {
		free(fb_h);
		free(fb_l);
		free(fb_r);
		delete work;
	}
};

class Controller {
private:
	std::string ConfigFile;
	std::vector<Control *> ControlList;

	int ArgPos(char *str, int argc, char **argv) {
		for (int a = 1; a < argc; a++)
			if (!strcmp(str, argv[a])) {
				if (a == argc - 1) return -1;
		 		return a;
		}
		return -1;
	}

	void ExplainTransE(FILE *fin) {
		Control *work = new TransEControl();
		work -> Explain(fin);
		work -> Init();
		ControlList.push_back(work);
	}

	void ExplainWord2Vec(FILE *fin) {
		Control *work = new Word2VecControl();
		work -> Explain(fin);
		work -> Init();
		ControlList.push_back(work);
	 }

public:
	Controller() {
		ControlList.clear();
		ConfigFile = "config";
	}
	Controller(int argc, char **argv) {
		ControlList.clear();
		int i;
		if ((i = ArgPos((char *)"-config", argc, argv)) > 0) ConfigFile = (std::string)(argv[i + 1]);
	}
	void explain() {
		FILE *fin = fopen(ConfigFile.c_str(),"r");
		std::string algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			if (ch == '=') {
				if (algo=="word2vec") {
					ExplainWord2Vec(fin); 
					algo = "";
				} else 
				if (algo=="transE") {
					ExplainTransE(fin); 
					algo = "";
				} else {
					printf("Wrong in config file!\n");
					return;
				}
			} else
			algo = algo + ch;
		}
		fclose(fin);
	}
};

int main(int argc, char **argv) {
	Controller corl;
	corl.explain();
	return 0;
}
