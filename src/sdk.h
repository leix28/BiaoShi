#ifndef SDK_H
#define SDK_H
#include "control.h"

class Controller;

class sdk:public Control {

private:
	Controller *command;

protected:


public:

	void Run() {
		printf("3\n");
	}

	sdk() {
		name = "sdk";
		command = NULL;
	}

	sdk(Controller *con) {
		name = "sdk";
		command = con;
	}

	~sdk() {

	}
};

#endif