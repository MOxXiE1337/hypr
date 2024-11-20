import os
import sys
import pefile
import struct

from minidump.minidumpfile import MinidumpFile
from minidump.minidumpfile import MinidumpMemorySegment

def read_mem(reader, start, size):
    segments = reader.get_memory()
    
    data = bytes()
    
    prev_seg = MinidumpMemorySegment()
    prev_seg.start_virtual_address = 0
    prev_seg.size = 0
    
    for seg in segments:
        if seg.start_virtual_address >= start and seg.start_virtual_address + seg.size < start + size:
            if seg.start_virtual_address > prev_seg.start_virtual_address + prev_seg.size and prev_seg.start_virtual_address != 0:
                padding = seg.start_virtual_address - prev_seg.start_virtual_address - prev_seg.size
                print("padding!")
                data += bytes(padding)
            data += reader.read(seg.start_virtual_address, seg.size)    
            prev_seg = seg
            
    return data
            
def unmap_pe(data):
    try:
        pe = pefile.PE(data=data, fast_load = True)
    except:
        return bytes()
        
    sizeof_headers = pe.OPTIONAL_HEADER.SizeOfHeaders
    new_data = data[:sizeof_headers]
    
    for sect in pe.sections:
        new_data += data[sect.VirtualAddress:sect.VirtualAddress+sect.SizeOfRawData]
        
    return new_data

def get_modules(dmp):
    modules = []
    for module in dmp.modules.modules:
        module_name = os.path.basename(module.name)
        if module_name.find('.exe') != -1: # skip exe
            continue
        modules.append({'imagebase': module.baseaddress, 'imagesize': module.size, 'name': module_name, 'path': module.name})
    return modules

def main():
    if len(sys.argv) < 3:
        print("Usage: python hdmpgen.py <path_to_dmp_file> <output_file_name> [extract_dll0] [extract_dll1] ...")
        return

    dmp_file_path = sys.argv[1]
    dll_filters = sys.argv[3:]
    
    print("filter:", dll_filters)
    
    if not os.path.isfile(dmp_file_path):
        print(f"file {dmp_file_path} does not exist.")
        return

    dmp = MinidumpFile.parse(dmp_file_path)
    dmp_reader = dmp.get_reader()
    
    dmp_modules = get_modules(dmp)
    
    modules = []
    module_name_table = bytes()
    proc_table = bytes()
    proc_name_table = bytes()
    
    for module in dmp_modules:
        pe_bin = unmap_pe(read_mem(dmp_reader, module['imagebase'], module['imagesize']))

        if len(pe_bin) == 0:
            print(f'skipping {module["name"]}')
            continue
            
        if len(dll_filters) != 0:
            if not module["name"] in dll_filters:
                continue
            
        pe = pefile.PE(data=pe_bin, fast_load = True)
        pe.parse_data_directories(0)
            
        procs = [] 
        
        if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
            print(f'skipping {module["name"]}')
            continue
            
        if len(pe.DIRECTORY_ENTRY_EXPORT.symbols) == 0:
            print(f'skipping {module["name"]}')
            continue
            
        print(f'dumping {module["name"]}')
          
        tmpmodule = {
            'name': len(module_name_table),
            'imagebase': module['imagebase'],
            'imagesize': module['imagesize'],
            'procs': len(proc_table)
        }   
        
        proc_num = 0
        
        for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            if exp.name == None:
                proc_table += struct.pack('IIQ', exp.ordinal, 0, pe.OPTIONAL_HEADER.ImageBase + exp.address)
            else:
                proc_table += struct.pack('IIQ', exp.ordinal, len(proc_name_table), pe.OPTIONAL_HEADER.ImageBase + exp.address)
                proc_name_table += exp.name + b'\x00'
            
            proc_num = proc_num + 1
            #procs.append({'ordinal': exp.ordinal, 'address': pe.OPTIONAL_HEADER.ImageBase + exp.address, 'name': len(proc_name_table)})
            
            #procs.append({'address': pe.OPTIONAL_HEADER.ImageBase + exp.address, 'name': exp.name, 'ordinal': exp.ordinal})
        
        tmpmodule['proc_num'] = proc_num
        modules.append(tmpmodule)
        
        module_name_table += bytes(module['name'].encode()) + b'\x00'
    
    with open(sys.argv[2], 'wb') as f:
        f.write(b'HDMP') # magic number
        f.write(struct.pack('I', len(modules))) # module num
        f.write(struct.pack('III', 0, 0, 0)) # module name table offset, proc name table offset, proc table offset
        
        for module in modules:
            f.write(struct.pack('I', module['name'])) # name
            f.write(struct.pack('Q', module['imagebase'])) # imagebase
            f.write(struct.pack('I', module['imagesize'])) # imagesize
            f.write(struct.pack('I', module['proc_num'])) # proc num
            f.write(struct.pack('I', module['procs'])) # proc
            
        
        module_name_table_offset = f.tell()
        f.seek(8)
        f.write(struct.pack('I', module_name_table_offset))
        f.seek(0,2)
        f.write(module_name_table)
        
        proc_name_table_offset = f.tell()
        f.seek(12)
        f.write(struct.pack('I', proc_name_table_offset))
        f.seek(0,2)
        f.write(proc_name_table)
        
        proc_table_offset = f.tell()
        f.seek(16)
        f.write(struct.pack('I', proc_table_offset))
        f.seek(0,2)
        f.write(proc_table)
        
        
    print(f'hdmp file has been written to {sys.argv[2]}')
    return
    
if __name__ == '__main__':
    main()