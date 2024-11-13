#include <hyprtrace/exec_tracer.h>

namespace hyprtrace
{
	hyprutils::LogManager ExecutionTracer::logman_{ "ExecutionTracer" };
	bool ExecutionTracer::inited_ = false;

	std::unordered_map<uint32_t, ExecutionTracer::ExecutionTrace> ExecutionTracer::traced_threads_;
	std::unordered_map<uintptr_t, ExecutionTracer::ExecutionBreakPoint> ExecutionTracer::execution_breakpoints_;
	std::vector<ExecutionTracer::ShellCodePage> ExecutionTracer::shellcode_pages_;

	ExecutionTracer::ShellCodePage* ExecutionTracer::CreateShellCodePage()
	{
		hyprutils::LogManager& logman = GetLogManager();

		void* address = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (address == nullptr)
		{
			logman.Error("failed to create code page");
			return nullptr;
		}

		memset(address, 0xCC, 0x1000);

		shellcode_pages_.push_back({ reinterpret_cast<uintptr_t>(address), 0x1000, 0 });

		return &shellcode_pages_.back();
	}

	LONG NTAPI ExecutionTracer::ExceptionHandler(struct _EXCEPTION_POINTERS* exception)
	{
		hyprutils::LogManager& logman = GetLogManager();
		uint32_t code = exception->ExceptionRecord->ExceptionCode;
		uintptr_t address = reinterpret_cast<uintptr_t>(exception->ExceptionRecord->ExceptionAddress);

		if (code == EXCEPTION_PRIV_INSTRUCTION)
		{
			bool prev_exec = true;
			uintptr_t bp_address = *reinterpret_cast<uintptr_t*>(address + 1);

			auto it = execution_breakpoints_.find(address);
			if (it == execution_breakpoints_.end())
			{
				it = execution_breakpoints_.find(bp_address);
				prev_exec = false;
			}

			if (it == execution_breakpoints_.end())
				return EXCEPTION_CONTINUE_SEARCH;

			ExecutionBreakPoint& bp = it->second;
			// prev execution?
			if (prev_exec)
			{
#ifdef _WIN64
				uintptr_t rip = exception->ContextRecord->Rip;
#else
				uintptr_t eip = exception->ContextRecord->Eip;
#endif
				// handler
				if (bp.prev_execution_handler)
					bp.prev_execution_handler(&logman, exception->ContextRecord);

#ifdef _WIN64
				// not modified by handler
				if (rip == exception->ContextRecord->Rip)
					exception->ContextRecord->Rip = bp.original_insn_address;
#else
				if (eip == exception->ContextRecord->Eip)
					exception->ContextRecord->Eip = bp.original_insn_address;
#endif
			}
			else // after execution?
			{
#ifdef _WIN64
				exception->ContextRecord->Rip = bp_address + bp.length;
#else
				exception->ContextRecord->Eip = bp_address + bp.length;
#endif

				// handler
				if (bp.after_execution_handler)
					bp.after_execution_handler(&logman, exception->ContextRecord);
			}
			return EXCEPTION_CONTINUE_EXECUTION; // continue execution
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	hyprutils::LogManager& ExecutionTracer::GetLogManager()
	{
		return logman_;
	}

	bool ExecutionTracer::Initialize()
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (inited_)
		{
			logman.Error("execution tracer is already intiialized");
			return false;
		}

		if (CreateShellCodePage() == nullptr)
			return false;

		AddVectoredExceptionHandler(TRUE, ExceptionHandler);

		inited_ = true;

		return true;
	}

	uint32_t ExecutionTracer::StartTracingAt(uintptr_t address, ExecutionTraceHandler handler, void* parameter)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (!inited_)
		{
			logman.Error("execution tracer is not intiialized");
			return false;
		}

		DWORD thread_id = -1;
		HANDLE thread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(address), parameter, CREATE_SUSPENDED, &thread_id);

		if (thread == NULL)
		{
			logman.Error("failed to create thread at {:X}, parameter: {:X}", address, reinterpret_cast<uintptr_t>(parameter));
			return -1;
		}

		CONTEXT context{ 0 };
		context.ContextFlags = CONTEXT_CONTROL;
		if (!GetThreadContext(thread, &context))
		{
			logman.Error("failed to get thread context");
			return -1;
		}

		context.EFlags |= 0x100; // TF

		if (!SetThreadContext(thread, &context))
		{
			logman.Error("failed to set thread context");
			return -1;
		}

		if (ResumeThread(thread) == -1)
		{
			logman.Error("failed to resume thread");
			return -1;
		}

		traced_threads_.insert({ thread_id,{ thread_id, handler } });

		return thread_id;
	}

	bool ExecutionTracer::StopTracing(uint32_t thread_id)
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (!inited_)
		{
			logman.Error("execution tracer is not intiialized");
			return false;
		}

		auto it = traced_threads_.find(thread_id);
		if (it == traced_threads_.end())
		{
			logman.Error("thread {:X} is not traced");
			return false;
		}

		traced_threads_.erase(it);

		return true;
	}

	bool ExecutionTracer::AddExecutionBreakPoint(uintptr_t address, size_t insn_len, ExecutionBreakPointHandler prev_exec_handler, ExecutionBreakPointHandler after_exec_handler)
	{
		hyprutils::LogManager& logman = GetLogManager();
		if (!inited_)
		{
			logman.Error("execution tracer is not intiialized");
			return false;
		}

		auto it = execution_breakpoints_.find(address);

		if (it != execution_breakpoints_.end())
		{
			logman_.Warn("execution breakpoint {:X} is already set", address);
			return false;
		}

		if (IsBadCodePtr(reinterpret_cast<FARPROC>(address)))
		{
			logman_.Error("{:X} is not executable for breakpoint", address);
			return false;
		}

		// get last page
		ShellCodePage* page = &shellcode_pages_.back();

		if (page->size - page->pos < insn_len + 1)
		{
			// create a new page
			page = CreateShellCodePage();
			if (!page)
				return false;
		}

		DWORD old_protect = PAGE_EXECUTE_READ;
		if (!VirtualProtect(reinterpret_cast<void*>(address), insn_len, PAGE_EXECUTE_READWRITE, &old_protect))
		{
			logman_.Error("failed to modify page protection at {:X}", address);
			return false;
		}

		memcpy(reinterpret_cast<void*>(page->address + page->pos), reinterpret_cast<void*>(address), insn_len);
		memset(reinterpret_cast<void*>(page->address + page->pos + insn_len), 0xF4, 1); // hlt
		*reinterpret_cast<uintptr_t*>(page->address + page->pos + insn_len + 1) = address; // to find the breakpoint info

		if (!VirtualProtect(reinterpret_cast<void*>(address), insn_len, PAGE_EXECUTE_READWRITE, &old_protect))
		{
			logman_.Error("failed to modify page protection at {:X}", address);
			return false;
		}

		memset(reinterpret_cast<void*>(address), 0x90, insn_len);
		memset(reinterpret_cast<void*>(address), 0xF4, 1); // hlt for triggering breakpoint

		if (!VirtualProtect(reinterpret_cast<void*>(address), insn_len, old_protect, &old_protect))
		{
			logman_.Error("failed to restore page protection at {:X}", address);
			return false;
		}

		logman_.Log("added execution breakpoint at {:X}, size {:X}, exec: {:X}", address, insn_len, page->address + page->pos);

		execution_breakpoints_.insert(
			{
				address,
				{
					page->address + page->pos,
					insn_len,
					prev_exec_handler,
					after_exec_handler
				}
			}
		);

		page->pos += insn_len + sizeof(uintptr_t) + 2;

		return true;
	}

	bool ExecutionTracer::RemoveExecutionBreakPoint(uintptr_t address)
	{
		hyprutils::LogManager& logman = GetLogManager();
		if (!inited_)
		{
			logman.Error("execution tracer is not intiialized");
			return false;
		}

		auto it = execution_breakpoints_.find(address);
		if (it == execution_breakpoints_.end())
		{
			logman_.Warn("execution breakpoint {:X} does not exist", address);
			return false;
		}

		ExecutionBreakPoint& breakpoint = it->second;

		DWORD old_protect = PAGE_EXECUTE_READ;
		if (!VirtualProtect(reinterpret_cast<void*>(address), breakpoint.length, PAGE_EXECUTE_READWRITE, &old_protect))
		{
			logman_.Error("failed to modify page protection at {:X}", address);
			return false;
		}

		// restore bytes
		memcpy(reinterpret_cast<void*>(address), reinterpret_cast<void*>(breakpoint.original_insn_address), breakpoint.original_insn_address);

		if (!VirtualProtect(reinterpret_cast<void*>(address), breakpoint.length, old_protect, &old_protect))
		{
			logman_.Error("failed to restore page protection at {:X}", address);
			return false;
		}

		logman_.Log("removed exection breakpoint {:X}", address);
		execution_breakpoints_.erase(it);

		return true;
	}
}