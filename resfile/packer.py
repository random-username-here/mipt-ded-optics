"""
Resource packer

Usage: packer.py DIRECTORY DEST-FILE
"""

import os, sys, struct

def blob_replacer(data : dict, dir : str):
    file = os.path.join(dir, data['file'])
    with open(file, 'rb') as f:
        data['blob'] = f.read()
    del data['file']

REPLACERS = {
    'blob': blob_replacer
}

def load_as(path: str):
    source = []
    with open(path, 'r') as f:
        source = f.read().splitlines()
    data = {}
    for line in source:
        if not line: continue
        key, value = line.split(maxsplit=1)
        try:
            value = int(value)
        except ValueError:
            try:
                value = float(value)
            except ValueError:
                pass
        data[key] = value
    return data

def main():
    if len(sys.argv) != 3:
        print('Expected two arguments: directory and target file')
        exit(-1)
    dir = sys.argv[1]
    dest = sys.argv[2]

    results = {}
    root_path = None
    for path, dirs, files in os.walk(dir):
        if root_path is None:
            root_path = path
        path = path.removeprefix(root_path).removeprefix('/')
        for name in files:
            if not name.endswith('.as'):
                continue
            print(root_path, path, name)
            dirpath = os.path.join(root_path, path)
            file = os.path.join(dirpath, name)
            print('Packing', file)
            data = load_as(file)
            if not 'type' in data:
                print('File', file, 'is missing `type`')
            if data['type'] in REPLACERS:
                REPLACERS[data['type']](data, dirpath)
            resname = (path + '/' if path else '').replace('/', '.') + name.removesuffix('.as')
            results[resname] = data

    with open(dest, 'wb') as out:

        def wu32(v : int):
            out.write(struct.pack('I', v))

        def wi32(v : int):
            out.write(struct.pack('i', v))

        def wf32(v : float):
            out.write(struct.pack('f', v))

        def wc(v : str):
            out.write(v[0].encode())

        def wb(v : bytes):
            wu32(len(v))
            out.write(v)

        def ws(v : str):
            wb(v.encode())

        wu32(len(results)) 
        for name, data in results.items():
            ws(name)
            ws(data['type'])
            wu32(len(data) - 1) # without type
            for key, value in data.items():
                if key == 'type': continue
                ws(key)
                if isinstance(value, int):
                    wc('i')
                    wi32(value)
                elif isinstance(value, float):
                    wc('f')
                    wf32(value)
                elif isinstance(value, str):
                    wc('s')
                    ws(value)
                elif isinstance(value, bytes):
                    wc('s')
                    wb(value)
                else:
                    print('Cannot encode', value)


if __name__ == '__main__':
    main()
