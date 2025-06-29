import sys
import re

def parse_cc65_lbl(lbl_lines):
    symbols = []
    for line in lbl_lines:
        line = line.strip()
        if not line or line.startswith(';'):
            continue
        # Format: label = $address
        m = re.match(r'^al\s+([0-9A-Fa-f]{6})\b\s*(\S.*)$', line)
        if m:
            name = m.group(2)
            addr = int(m.group(1), 16)
            symbols.append((addr, name))
    return symbols

def write_altirra_sym(symbols, out_file):
    with open(out_file, 'w') as f:
        for addr, name in sorted(symbols):
            f.write(f"{addr:04X} {name}\n")

def main():
    if len(sys.argv) != 3:
        print("Usage: python lbl2sym.py input.lbl output.sym")
        sys.exit(1)
    with open(sys.argv[1], 'r') as f:
        lbl_lines = f.readlines()
    symbols = parse_cc65_lbl(lbl_lines)
    write_altirra_sym(symbols, sys.argv[2])

if __name__ == "__main__":
    main()