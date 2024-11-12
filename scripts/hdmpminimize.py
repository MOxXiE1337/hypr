import os
import re
import sys
import struct 

def main():
    if len(sys.argv) != 2:
        print("Usage: python hdmpminimize.py <hdmp_path>")
        return
    
    path = sys.argv[1]
    
    try:
        f = open(path, 'rb') 
        
        magic = f.read(4)
        
        if magic != b'HDMP':
            print(f'{path} is not a hdmp file')
            return
            
        module_num = struct.unpack('I', f.read(4))[0]
        
        print(f'module num: {module_num}')
        
        f.seek(0)
        
        data = f.read(20 + 24 * module_num)
        
        new_path = ''
        
        if re.search(r'.hdmp', path) is None:
            new_path = path + '.min.hdmp'
        else:
            new_path = re.sub(r'.hdmp', '.min.hdmp', path)
            
        with open(new_path, 'wb') as nf:
            nf.write(data)
     
        print(f'minimal hdmp file has been written to {new_path}')
        
      
    except Exception as e:
        print(e)
        
    return
        
        
    
if __name__ == '__main__':
    main()