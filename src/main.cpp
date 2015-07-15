#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "control.h"
#include "sdk.h"

class Controller {
private:
	std::string ConfigFile;
	std::vector <Control *> ControlList;
	std::vector <std::string> command_name;
	std::vector <int> command_times;

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

	void ExplainRun(FILE *fin) {
		std::string algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			algo = algo + ch;
			if (algo == "(") break;
		}
		algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			if (ch == ',') break;
			algo = algo + ch;
		}
		command_name.push_back(algo);

		algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			if (ch == ')') break;
			algo = algo + ch;
		}
		command_times.push_back(atoi(algo.c_str()));
	}

	void ExplainDo(FILE *fin) {
		std::string algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			algo = algo + ch;
			if (algo == "(") break;
		}
		algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			algo = algo + ch;
			if (ch == ')') break;
		}
		command_name.push_back("do");
		command_times.push_back(atoi(algo.c_str()));
	}

	int Run(int con) {
		if (command_name[con] == "loop") return con;
		if (command_name[con] == "do") {
			int tmp;
			for (int i = 1; i <= command_times[con]; i++) 
				tmp = Run(con+1);
			return Run(tmp+1);
		}
		Control *contr = GetControlByName(command_name[con]);
		for (int i = 1; i <= command_times[con]; i++)
			contr->Run();
		return Run(con+1);
	}

public:
	
	Controller() {
		ControlList.clear();
		ConfigFile = "config";
		Control *con = new sdk(this);
		ControlList.push_back(con);
	}

	Controller(int argc, char **argv) {
		ControlList.clear();
		int i;
		if ((i = ArgPos((char *)"-config", argc, argv)) > 0) ConfigFile = (std::string)(argv[i + 1]);
		Control *con = new sdk(this);
		ControlList.push_back(con);
	}

	void Explain() {
		FILE *fin = fopen(ConfigFile.c_str(),"r");

		std::string algo = "";
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			algo = algo + ch;
			if (algo == "#define") break;
		}

		algo = "";
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
			if (algo == "#end") break;
		}

		algo = "";
		command_name.push_back("do");
		command_times.push_back(1);
		while (!feof(fin)) {
			char ch = fgetc(fin);
			if (ch == '\n' || ch =='\r' || ch=='\t' || ch==' ') continue;
			algo = algo + ch;
			if (algo == "run") {
				ExplainRun(fin);
				algo = "";
			}
			if (algo == "do") {
				ExplainDo(fin);
				algo = "";
			}
			if (algo == "loop") {
				command_name.push_back("loop");
				command_times.push_back(0);
				algo = "";
			}
		}
		command_name.push_back("loop");
		command_times.push_back(0);
		Run(0);
		fclose(fin);
	}

	Control *GetControlByName(char *con) {
		return GetControlByName(std::string(con));
	}

	Control *GetControlByName(std::string name) {
		for (int i = 0; i < ControlList.size(); i++) 
			if (ControlList[i]->GetName() == name) return ControlList[i];
		return NULL;
	}

	~Controller() {
		for (int i = 0; i < ControlList.size(); i++)
			if (ControlList[i]!=NULL)
				delete ControlList[i];
	}
};

int main(int argc, char **argv) {
	Controller corl;
	corl.Explain();
	return 0;
}
