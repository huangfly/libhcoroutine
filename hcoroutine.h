#ifndef HCOROUTINE_H
#define HCOROUTINE_H

#include <iostream>
#include <unistd.h>
#include <ucontext.h>
#include <list>
#include <vector>
#include <future>
#include <assert.h>
#include <atomic>
typedef std::function<void()> Func;

namespace hcoroutine {
	class Task
	{
	public:
		explicit Task(Func callfunc, const size_t size = 1024 * 128) :m_func(callfunc), m_stackSize(size), m_stack(nullptr), m_isFinish(false) {}
		~Task() {
			if (m_stack != nullptr) {
				free(m_stack);
			}
		}
		static void ucontext_func(Task * tk)
		{
			tk->m_func();
			tk->m_isFinish = true;
			std::cout << "set true" << std::endl;
		}
	public:
		ucontext_t m_taskCtx;
		char * m_stack;
		Func m_func;
		size_t m_stackSize;
		bool m_isFinish;
	};

	class Coroutine
	{
	public:
		explicit Coroutine(size_t size = 1024 * 128) :m_corStackSize(size), m_current(nullptr), m_currentId(0), m_count(0) {}
		~Coroutine() {
			for (auto task : m_taskList) {
				delete task;
			}
		}

		inline void CreateTask(Func fn) {
			Task *ft = new Task(fn);
			this->m_taskList.push_back(ft);
			m_count++;
		}

		inline void yield() {
			if (!m_current) {
				return;
			}
			char* stackTop = m_current->m_stack + m_current->m_stackSize;
			char stackBottom = 0;
			assert((size_t)(stackTop - &stackBottom) <= (m_current->m_stackSize));
			swapcontext(&m_current->m_taskCtx, &this->m_ctx);
		}

		void RunUntilNoTask() {
			while (m_count) {
				//判断并且重置当前下标。
				if (m_currentId > m_taskList.size() - 1) {
					m_currentId = 0;
				}
				m_current = this->m_taskList[m_currentId++];

				//判断当前协程是否为空或者已经执行完毕
				if (!m_current || m_current->m_isFinish) {
					continue;
				}
				
				//判断当前协程是否未分配堆栈信息
				if (!m_current->m_stack) {
					getcontext(&m_current->m_taskCtx);
					m_current->m_stack = (char *)malloc(m_current->m_stackSize * sizeof(char));
					m_current->m_taskCtx.uc_stack.ss_sp = m_current->m_stack;
					m_current->m_taskCtx.uc_stack.ss_size = m_current->m_stackSize;
					m_current->m_taskCtx.uc_link = &this->m_ctx;
					makecontext(&m_current->m_taskCtx, (void(*)(void))(Task::ucontext_func), 1, m_current);//(&m_current->m_func), 0);
					swapcontext(&this->m_ctx, &m_current->m_taskCtx);
				}
				else
				{
					swapcontext(&this->m_ctx, &m_current->m_taskCtx);
				}
				if (m_current->m_isFinish) {
					m_count--;
				}

			}
		}

	private:
		Task * m_current;
		int m_currentId;
		ucontext_t m_ctx;
		std::vector<Task*> m_taskList;
		std::list<int> m_idList;
		size_t m_corStackSize;
		std::atomic<int> m_count;
	};


	inline Coroutine* GetLocalCoroutine() {
		static thread_local Coroutine* routine = NULL;
		if (!routine) {
			routine = new Coroutine();
		}
		return routine;
	}
}


#endif 
