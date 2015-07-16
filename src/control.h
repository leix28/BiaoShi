#ifndef CONTROL_H
#define CONTROL_H
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include "word2vec.h"
#include "TrainTransE.h"
#include "header.h"

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

	int SearchWord(const std::vector<Vocab> &vocabList, const char *word, const std::vector<int> &hash, unsigned long long p = 257, unsigned long long hashSize = 10000007) {
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
	virtual int GetWordNumber(std::string) {};
	virtual int GetWordNumber(char*) {};
	virtual real *GetWordEmbedding(std::string) {};
	virtual real *GetWordEmbedding(char *) {};

	std::string GetName() {
		return name;
	}

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
	real *words;

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
		words=work->GetEmbedding();
	}

	void Run() {
		printf("Word2Vec:\n");
		work->Run();
	}

	int GetWordNumber(std::string con) {
		return GetWordNumber(con.c_str());

	}

	int GetWordNumber(char *con) {
		return work->SearchWord(con);
	}

	real * GetWordEmbedding(std::string con) {
		return GetWordEmbedding(con.c_str());
	}

	real * GetWordEmbedding(char *con) {
		int tmp = work->SearchWord(con);
		if (tmp != -1) return &words[tmp*dimension];
		return NULL;
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
		printf("Size of Relations : %d   Size of Entities : %d\n",(int)vocabRelation.size(), (int)vocabEntity.size());
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
		work->SetRandom(entityVec, relationVec);
	}

	void Run() {
		printf("TransE:\n");
		work->Run(entityVec, relationVec);
	}

	int GetWordNumber(std::string con) {
		return GetWordNumber(con.c_str());
	}

	int GetWordNumber(char *con) {
		int tmp = SearchWord(vocabEntity, con, hashEntity, hashP, hashSize);
		if (tmp != -1) return tmp;
		tmp = SearchWord(vocabRelation, con, hashRelaton, hashP, hashSize);
		if (tmp != -1) return tmp;
		return -1;
	}

	real * GetWordEmbedding(std::string con) {
		return GetWordEmbedding(con.c_str());
	}

	real * GetWordEmbedding(char *con) {
		int tmp = SearchWord(vocabEntity, con, hashEntity, hashP, hashSize);
		if (tmp != -1) return &entityVec[tmp*dimension];
		tmp = SearchWord(vocabRelation, con, hashRelaton, hashP, hashSize);
		if (tmp != -1) return &relationVec[tmp*dimension];
		return NULL;
	}

	~TransEControl() {
		free(fb_h);
		free(fb_l);
		free(fb_r);
		delete work;
	}
};

#endif
