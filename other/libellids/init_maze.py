# -*- coding: utf-8 -*-

filename = input('File di output: ')
h = int(input('Altezza: '))
w = int(input('Larghezza: '))

with open(filename, 'wt') as f:
    f.write('+---' * w + '+\n')
    for i in range(h):
        f.write('|   ' + '    ' * (w - 1) + '|\n')
        if i < h - 1:
            f.write('+   ' * w + '+\n')
    f.write('+---' * w + '+\n')
    

