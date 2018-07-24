#ifndef COROUTINE_H
#define COROUTINE_H
#include "hcoroutine.h"

#define AddTask(func) hcoroutine::GetLocalCoroutine()->CreateTask(func)
#define Yeild()		  hcoroutine::GetLocalCoroutine()->yield()
#define Run()		  hcoroutine::GetLocalCoroutine()->RunUntilNoTask()
#endif // !COROUTINE_H

