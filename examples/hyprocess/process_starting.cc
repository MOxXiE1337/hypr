#include <hyprocess/process_starter.h>
#include <hyprtrace/tracer.h>

int main()
{
	hyprocess::ProcessStarter starter;

	// if you want to use hyprtrace to trace api calls
	//hyprtrace::Tracer& tracer = hyprtrace::Tracer::GetInstance();
	//hyprfile::RuntimeDumpFile runtime_dump_file;
	//runtime_dump_file.Load(R"()");
	//tracer.InitializeProcessStarter(starter, runtime_dump_file);
	
	starter.ReserveMemory(0x7000000000000000, 0x1000, PAGE_EXECUTE_READWRITE);
	starter.SetImagePath(R"()");
	starter.StartProcess();
	return 0;
}