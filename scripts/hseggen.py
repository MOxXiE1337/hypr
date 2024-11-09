import os
import sys
import re
import struct

def main():
    if len(sys.argv) < 3:
        print("Usage: python hseggen.py <segs_folder_path> <output_file_name> [base_address]")
        print("Make sure the segment file is named like {address}_{size}.bin ! (40000000_1E000.bin)")
        return
        
    folder = os.path.abspath(sys.argv[1])
    output_name = sys.argv[2]
    base_address = 0
    if len(sys.argv) == 4:
        base_address = int(sys.argv[3], 16)
        print(f'base address: {hex(base_address)}')
    
    if not os.path.isdir(folder):
        print(f'{folder} is not a directory!')
        return
        
    files = os.listdir(folder)
    
    ordinal = 0
    segments = []
    raw_data = bytes()
    
    for name in files:
        fullpath = folder + '\\' + name
        
        if not re.match('[0-9A-Fa-f]+_[0-9A-Fa-f]+\\.bin', name):
            print(f'skipping {name}')
            continue
            
        result = re.findall('[0-9A-Fa-f]+', name)
        
        address = int(result[0], 16)
        vsize = int(result[1], 16)
        
        if vsize > 0xffffffff:
            print(f'skipping {name}')
            continue
        
        rsize = os.path.getsize(fullpath)
        
        segments.append({'ordinal': ordinal, 'address': address, 'vsize': vsize, 'rsize': rsize, 'data': len(raw_data)})
        
        with open(fullpath, 'rb') as raw:
            raw_data += raw.read(rsize)
        
        ordinal = ordinal + 1
        
    try:
        f = open(output_name, 'wb')
    except Exception as e:
        print(e)
        
    # header
    f.write(b'HSEG')
    f.write(struct.pack('Q', base_address))
    f.write(struct.pack('I', len(segments)))
    
    # segments 
    for segment in segments:
        f.write(struct.pack('I', segment['ordinal']))
        f.write(struct.pack('Q', segment['address']))
        f.write(struct.pack('I', segment['vsize']))
        f.write(struct.pack('I', segment['rsize']))
        f.write(struct.pack('I', segment['data']))
        #f.write(struct.pack('IQIII', segment['ordinal'], segment['address'], segment['vsize'], segment['rsize'], segment['data']))
            
    # raw data
    f.write(raw_data)
    print(f'hseg file has been written to {output_name}')
    
    return 
    

if __name__ == '__main__':
    main()