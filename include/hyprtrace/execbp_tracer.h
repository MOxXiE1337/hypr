#include "hyprtrace.h"

#include <hyprutils/logmanager.h>

namespace hyprtrace
{

	class ExecutionBreakPointTracer
	{
	public:
		using ExecutionBreakPointHandler = std::function<void(hyprutils::LogManager*, PCONTEXT)>;

	private:
		struct ExecutionBreakPoint
		{
			uintptr_t original_insn_address;
			uintptr_t length;
			ExecutionBreakPointHandler prev_execution_handler;
			ExecutionBreakPointHandler after_execution_handler;
		};

		struct ShellCodePage
		{
			uintptr_t address;
			size_t size; // default 0x1000
			ptrdiff_t pos;
		};

	private:
		static hyprutils::LogManager logman_;
		static bool inited_;
		static std::unordered_map<uintptr_t, ExecutionBreakPoint> execution_breakpoints_;
		static std::vector<ShellCodePage> shellcode_pages_;

		static ShellCodePage* CreateShellCodePage();
		static LONG NTAPI ExceptionHandler(struct _EXCEPTION_POINTERS* exception);
	public:
		static hyprutils::LogManager& GetLogManager();

		static bool Initialize();
		static bool AddExecutionBreakPoint(uintptr_t address, size_t insn_len, ExecutionBreakPointHandler prev_exec_handler, ExecutionBreakPointHandler after_exec_handler);
		static bool RemoveExecutionBreakPoint(uintptr_t address);
	};
}