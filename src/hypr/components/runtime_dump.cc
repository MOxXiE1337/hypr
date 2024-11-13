#include <hypr/components/runtime_dump.h>

#include <hyprfile/runtime_dump_file.h>
#include <hyprutils/file.h>

namespace hypr
{

    bool RuntimeDump::ProcRecord::LoadProc()
    {
        if (new_address != 0)
            return true;

        std::shared_ptr<ModuleRecord> module = this->module.lock();
        new_address = reinterpret_cast<uintptr_t>(GetProcAddress(LoadLibraryA(module->name.c_str()), name.c_str()));

        if (new_address == 0)
            return false;
        return true;
    }

    RuntimeDump::RuntimeDump(Loader* loader) : LoaderComponent(loader, "RuntimeDump")
    {
    }

    std::shared_ptr<RuntimeDump::ModuleRecord> RuntimeDump::FindModuleRecord(segaddr_t address)
    {
        return modules_address_search_.Find(address);
    }

    std::shared_ptr<RuntimeDump::ModuleRecord> RuntimeDump::FindModuleRecord(const std::string& name)
    {
        return modules_name_search_.Find(name);
    }

    std::shared_ptr<RuntimeDump::ProcRecord> RuntimeDump::FindProcRecord(segaddr_t address)
    {
        hyprutils::LogManager& logman = GetLogManager();
        std::shared_ptr<ProcRecord> proc = procs_search_.Find(address);

        if (!proc)
            return {};

        if (!proc->LoadProc())
        {
            logman.Error("failed to load proc {}!{}", proc->module.lock()->name, proc->name);
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
                        hproc.address,
                        0,
                        hproc.name);

                    procs_.push_back(proc);
                    procs_search_.AddElement(uintptr_t(hproc.address), proc);
                    mod->procs.push_back(proc);
                    mod->procs_name_search.AddElement(hproc.name, proc);
                }
                modules_.push_back(mod);
                modules_address_search_.AddElement({ uintptr_t(module.imagebase), module.imagesize }, mod);
                modules_name_search_.AddElement(module.name, mod);
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

