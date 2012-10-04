import re

if __name__ == '__main__':
    fin = open('stomp-strings', 'r')
    hfout = open('stomp-strings.hpp', 'w')
    cfout = open('stomp-strings.cpp', 'w')
    
    for line in fin:
        if re.match(r'^\s*[a-zA-Z0-9_]+\s*\:[a-zA-Z0-9\-]+\s*$', line):
            line = re.sub(r'\s*', '', line).split(':')
            
            length = str(len(line[1])+1)
            cfout.write("const char " + line[0] + "[" + length + "] =\n/* \"" + line[1] + "\" */\n{" + reduce(lambda a, b: a+hex(ord(b))+',', line[1], '') + "};\n\n")
            hfout.write("extern const char " + line[0] + "[" + length + "];\n")
    
    fin.close()
    hfout.close()
    cfout.close()
