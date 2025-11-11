import sys

def convert_file(input_file, output_file):
    with open(input_file, 'r') as f:
        content = f.read().strip()
    
    # Handle different separators
    numbers = content.replace(',', ' ').split()
    
    with open(output_file, 'w') as f:
        for number in numbers:
            f.write(number + '\n')

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python convert.py input.txt output.txt")
        sys.exit(1)
    
    convert_file(sys.argv[1], sys.argv[2])
    print(f"Converted {sys.argv[1]} to {sys.argv[2]}")