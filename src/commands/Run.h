/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */


#ifndef _RUN_H
#define _RUN_H

#include "Command.h"
#include "../CompileMode.h"


class Run: public Command {
public: 
  
void Run();
  
int operator()();
private: 
  CompileMode mode_;
  
bool has_cpp();
  
void build_command();
  
vector<string> get_dependencies();
};

#endif //_RUN_H