#include <hyprtrace/execbp_tracer.h>

namespace hyprtrace
{
	hyprutils::LogManager ExecutionBreakPointTracer::logman_{ "ExecBpTracer" };
	bool ExecutionBreakPointTracer::inited_ = false;
	std::unordered_map<uintptr_t, ExecutionBreakPointTracer::ExecutionBreakPoint> ExecutionBreakPointTracer::execution_breakpoints_;
	std::vector<ExecutionBreakPointTracer::ShellCodePage> ExecutionBreakPointTracer::shellcode_pages_;

	ExecutionBreakPointTracer::ShellCodePage* ExecutionBreakPointTracer::CreateShellCodePage()
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

	LONG NTAPI ExecutionBreakPointTracer::ExceptionHandler(struct _EXCEPTION_POINTERS* exception)
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
				if(rip == exception->ContextRecord->Rip)
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

	hyprutils::LogManager& ExecutionBreakPointTracer::GetLogManager()
	{
		return logman_;
	}

	bool ExecutionBreakPointTracer::Initialize()
	{
		hyprutils::LogManager& logman = GetLogManager();

		if (inited_)
		{
			logman.Error("execution breakpoint tracer is already intiialized");
			return false;
		}

		if (CreateShellCodePage() == nullptr)
			return false;

		AddVectoredExceptionHandler(TRUE, ExceptionHandler);

		return true;
	}

	bool ExecutionBreakPointTracer::AddExecutionBreakPoint(uintptr_t address, size_t insn_len, ExecutionBreakPointHandler prev_exec_handler, ExecutionBreakPointHandler after_exec_handler)
	{
		hyprutils::LogManager& logman = GetLogManager();

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
	
	bool ExecutionBreakPointTracer::RemoveExecutionBreakPoint(uintptr_t address)
	{
		hyprutils::LogManager& logman = GetLogManager();

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