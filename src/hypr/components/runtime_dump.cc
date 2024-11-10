#include <hypr/components/runtime_dump.h>

#include <hyprfile/runtime_dump_file.h>
#include <hyprutils/file.h>

namespace hypr
{
    RuntimeDump::RuntimeDump(Loader* loader) : LoaderComponent(loader, "RuntimeDump")
    {
    }

    std::shared_ptr<const RuntimeDump::ModuleRecord> RuntimeDump::FindModuleRecord(segaddr_t address)
    {
        std::shared_ptr<ModuleRecord> module = modules_search_.Find(address);
        return module;
    }

    std::shared_ptr<const RuntimeDump::ProcRecord> RuntimeDump::FindProcRecord(segaddr_t address)
    {
        hyprutils::LogManager& logman = GetLogManager();
        std::shared_ptr<ProcRecord> proc = procs_search_.Find(address);

        if (!proc)
            return {};

        if (proc->new_address == 0)
        {
            std::shared_ptr<ModuleRecord> module = proc->module.lock();
            proc->new_address = reinterpret_cast<uintptr_t>(GetProcAddress(LoadLibraryA(module->name.c_str()), proc->name.c_str()));
            if (proc->new_address == 0)
            {
                logman.Error("failed to load proc {}.{}", module->name, proc->name);
                return {};
            }
        }
        return proc;
    }

    bool RuntimeDump::LoadRuntimeDumpFileFromMemory(const void* data, size_t size)
    {
        hyprutils::LogManager& logman = GetLogManager();

        hyprfile::RuntimeDumpFile runtime_dump_file{};
        if (!runtime_dump_file.LoadFromMemory(data, size))
        {
            logman.Error("failed to parse runtime dump file");
            return false;
        }

        auto load_module = [&](const hyprfile::RuntimeDumpFile::ModuleRecord& module)
            {
                std::shared_ptr<ModuleRecord> mod = std::make_shared<ModuleRecord>();
                mod->name = module.name;
                mod->imagebase = uintptr_t(module.imagebase);
                mod->imagesize = module.imagesize;

                std::vector<hyprfile::RuntimeDumpFile::ProcRecord> procs;
                runtime_dump_file.GetProcRecords(module, procs);
                
                for (auto& hproc : procs)
                {
                    std::shared_ptr<ProcRecord> proc = std::make_shared<ProcRecord>(
                        mod,
                        hproc.ordinal,
                        0,
                        hproc.name);

                    procs_.push_back(proc);
                    procs_search_.AddElement(uintptr_t(hproc.address), proc);
                    mod->procs.push_back(proc);
                }
                modules_.push_back(mod);
                modules_search_.AddElement({ uintptr_t(module.imagebase), module.imagesize }, mod);
            };

        std::vector<hyprfile::RuntimeDumpFile::ModuleRecord> modules;
        runtime_dump_file.GetModuleRecords(modules);

        for (auto& module : modules)
        {
            load_module(module);
        }
        
        logman.Log("loaded {} modules", static_cast<int>(modules.size()));

        return true;
    }
   
    bool RuntimeDump::LoadRuntimeDumpFileFromFile(const std::string& path)
    {
        hyprutils::LogManager& logman = GetLogManager();

        size_t size = 0;
        std::shared_ptr<uint8_t[]> buffer = hyprutils::ReadFileToMemory(path, &size);

        if (!buffer || size == 0)
        {
            logman.Error("failed to read runtime dump file \"{}\"", path);
            return false;
        }

        return LoadRuntimeDumpFileFromMemory(buffer.get(), size);
    }

}

