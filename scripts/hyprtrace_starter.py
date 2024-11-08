import os
import sys
import struct
import win32process
import ctypes

def parse_modules_from_hdmp(path):
    modules = []
    try:
        f = open(path, 'rb')      
    except:
        print(f'failed to open {path}')
        
    magic = f.read(4) 

    if magic != b'HDMP':
        print(f'{path} is not a valid hdmp file')
        return
            
    module_num = struct.unpack('I', f.read(4))[0]
    
    f.seek(20)
    
    for i in range(0, module_num):
        module = struct.unpack('<IQIII', f.read(24))
        imagebase = module[1]
        imagesize = module[2]
        modules.append({'imagebase': imagebase, 'imagesize': imagesize})
    
    f.close()
    
    print(f'parsed {len(modules)} modules')
    
    return modules

def main():
    if len(sys.argv) != 3:
        print("Usage: python hyprtrace.py <exe_path> <hdmp_path>")
        return
        
    exe_path = sys.argv[1]
    hdmp_path = sys.argv[2]
    
    if not os.path.exists(exe_path):
        print(f'executable {exe_path} does not exist')
        return
    
    print(f'executable path: {exe_path}')  # Debugging line
    
    modules = parse_modules_from_hdmp(hdmp_path)
    
    if modules is not None:
        try:
            handle = win32process.CreateProcess(
                exe_path, 
                '', 
                None, 
                None, 
                0, 
                win32process.CREATE_SUSPENDED, 
                None, 
                os.path.dirname(exe_path), 
                win32process.STARTUPINFO()
            )
            
            phandle = handle[0]
            thandle = handle[1]
            
            print(f'successfully started process: {exe_path}')
            
        except Exception as e:
            print(f'Failed to create process: {e}')
            
        kernel32 = ctypes.windll.kernel32
            
        virtual_alloc_ex = kernel32.VirtualAllocEx
        virtual_alloc_ex.argtypes = (ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32)
        virtual_alloc_ex.restype = ctypes.c_void_p 
        
        mem_num = 0
        
        for module in modules:
            addr = virtual_alloc_ex(int(phandle), module['imagebase'], module['imagesize'], 0x3000, 0x40)
            
            if addr is None:
                print('FAILED TO ALLOCATE', hex(module['imagebase']), hex(module['imagesize']) + '!')
                print('THIS MAY BE CAUSED BY ntdll(ntdll is being mapped to a fixed address), TRY TO REBOOT YOUR SYSTEM AND RETRY')
                print('terminating process...')
                win32process.TerminateProcess(phandle, 0)
                return
        
            mem_num = mem_num + 1
            
        print(f'allocated {mem_num} pages')
        print('resuming process...')
        win32process.ResumeThread(thandle)
        
            
            
        

if __name__ == '__main__':
    main()