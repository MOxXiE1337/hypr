import os
import re
import sys
import struct 
def main():
    if len(sys.argv) != 2:
        print("Usage: python hsegminimize.py <hseg_path>")
        return
    
    path = sys.argv[1]
    
    try:
        f = open(path, 'rb') 
        
        magic = f.read(4)
        
        if magic != b'HSEG':
            print(f'{path} is not a hseg file')
            return
            
        base_address = f.read(8)
        segment_num = struct.unpack('I', f.read(4))[0]
        
        # write segments to file without data
        f.seek(0)
        data = f.read(16 + 24 * segment_num)
        
        new_path = ''
        
        if re.search(r'.hseg', path) is None:
            new_path = path + '.min.hseg'
        else:
            new_path = re.sub(r'.hseg', '.min.hseg', path)
            
        with open(new_path, 'wb') as nf:
            nf.write(data)
     
        print(f'minimal hseg file has been written to {new_path}')
    except Exception as e:
        print(e)
        
    return
        
        
    
if __name__ == '__main__':
    main()