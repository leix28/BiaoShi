#ifndef __BiaoShi_TrainTransE_h__
#define __BiaoShi_TrainTransE_h__

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <cstring>
#include <thread>
#include "header.h"

class TrainTransE {
private:

	bool flagL1;
	unsigned long long nextRandom;

	
	int n, method;
	char version[10];
	int thread;
	real res;
	real rate, margin;
	real belta;
	int* fb_h, *fb_l, *fb_r;
	int fb_num, relation_num, entity_num;
	real *relation_vec, *entity_vec;
	real *relation_tmp, *entity_tmp;
	real *left_num, *right_num;
	int nepoch;

	void Rand();

	real RandInitialize(real min, real max);

	real NormalDouble(real x, real miu, real sigma);

	real RandN(real miu, real sigma, real min, double max);

	real Sqr(real x);

	real VecLen(real *a, int aBegin);

	void Norm(real *a, int aBegin);

	int RandMax(int x);

	real CalcSum(int e1, int e2, int rel);
	void Gradient(int e1_a, int e2_a, int rel_a, int e1_b, int e2_b, int rel_b);
	void TrainKb(int e1_a, int e2_a, int rel_a, int e1_b, int e2_b, int rel_b, real &res);
	int FindOk(int e1, int e2, int rel);
	void Bfgs();
	void PrintEmbedding();

	void Clear();
public:

	TrainTransE();

	void SetN(int inN);

	void SetRate(int inRate);

	void SetMargin(int inMargin);

	void SetMethod(int inMethod);

	void SetNepoch(int inNepoch);

	void SetThread(int inTread);

	void SetRelationNum(int inRelationNum);

	void SetEntityNum(int inEntityNum);

	~TrainTransE();

	void SetTrainFile(int *fb_h, int *fb_l, int *fb_r, int fb_num);
	bool Run(real *inEntityVec, real *inRelationVec);
};

#endif
