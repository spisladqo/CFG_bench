#!/usr/bin/env python3
"""
File Processor: Replace " -> " with tab characters
"""

import os
import sys
import argparse
from pathlib import Path

def process_file(input_file, output_file=None, in_place=False, backup=False):
    """
    Process a file by replacing " -> " with tab characters.
    
    Args:
        input_file: Path to input file
        output_file: Path to output file (if None, prints to console)
        in_place: If True, modify the file in place
        backup: If True and in_place=True, create a backup
    """
    try:
        # Read the input file
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Replace " -> " with a tab character
        modified_content = content.replace(" -> ", "\t")
        
        if in_place:
            # Create backup if requested
            if backup:
                backup_file = f"{input_file}.bak"
                with open(backup_file, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Backup created: {backup_file}")
            
            # Write modified content back to the same file
            with open(input_file, 'w', encoding='utf-8') as f:
                f.write(modified_content)
            print(f"File processed successfully: {input_file}")
            
        elif output_file:
            # Write to specified output file
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(modified_content)
            print(f"File processed successfully: {output_file}")
        else:
            # Print to console (for testing)
            print(modified_content)
            
        return True
        
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        return False
    except PermissionError:
        print(f"Error: Permission denied for '{input_file}'.")
        return False
    except Exception as e:
        print(f"Error processing file: {e}")
        return False

def main():
    """Main function with command-line interface."""
    parser = argparse.ArgumentParser(
        description='Replace " -> " with tabs in text files',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s input.txt                # Show output in console
  %(prog)s input.txt -o output.txt  # Write to new file
  %(prog)s input.txt -i             # Modify file in place
  %(prog)s input.txt -i -b          # Modify in place with backup
  %(prog)s *.txt -i                 # Process all .txt files in current directory
        """
    )
    
    parser.add_argument('files', nargs='+', help='Input file(s) to process')
    parser.add_argument('-o', '--output', help='Output file (only works with single input)')
    parser.add_argument('-i', '--in-place', action='store_true', 
                       help='Modify file(s) in place')
    parser.add_argument('-b', '--backup', action='store_true',
                       help='Create backup before in-place modification')
    
    args = parser.parse_args()
    
    # Check for invalid combinations
    if args.output and len(args.files) > 1:
        print("Error: Cannot use -o with multiple input files")
        return 1
    
    if args.backup and not args.in_place:
        print("Warning: -b ignored without -i")
    
    # Process files
    success_count = 0
    for file_path in args.files:
        # Handle glob patterns
        import glob
        for actual_file in glob.glob(file_path):
            result = process_file(
                actual_file,
                output_file=args.output if len(args.files) == 1 and args.output else None,
                in_place=args.in_place,
                backup=args.backup
            )
            if result:
                success_count += 1
    
    print(f"\nProcessed {success_count} file(s) successfully.")
    
    return 0 if success_count > 0 else 1

if __name__ == "__main__":
    sys.exit(main())