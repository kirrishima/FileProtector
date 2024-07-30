import os
import chardet
import fnmatch

def should_ignore(path, ignore_patterns):
    for pattern in ignore_patterns:
        if fnmatch.fnmatch(path, pattern):
            return True
    return False

def convert_file_encoding(file_path, target_encoding):
    with open(file_path, 'rb') as f:
        raw_data = f.read()

    detected_encoding = chardet.detect(raw_data)['encoding']
    if detected_encoding is None:
        print(f"Could not detect encoding for {file_path}")
        return

    if detected_encoding.lower() == target_encoding.lower():
        print(f"File {file_path} is already in {target_encoding} encoding")
        return

    try:
        text = raw_data.decode(detected_encoding)
        normalized_text = text.replace('\r\n', '\n').replace('\r', '\n')

        with open(file_path, 'w', encoding=target_encoding, newline='\n') as f:
            f.write(normalized_text)
        
        print(f"Converted {file_path} from {detected_encoding} to {target_encoding}")
    except Exception as e:
        print(f"Error converting {file_path}: {e}")

def convert_directory_encoding(directory, target_encoding, ignore_patterns):
    for root, dirs, files in os.walk(directory):
        # Игнорирование директорий
        dirs[:] = [d for d in dirs if not should_ignore(os.path.join(root, d), ignore_patterns)]
        for file in files:
            file_path = os.path.join(root, file)
            if not should_ignore(file_path, ignore_patterns) and (file.endswith('.cpp') or file.endswith('.h')):
                convert_file_encoding(file_path, target_encoding)

if __name__ == "__main__":
    target_directory = os.path.dirname(__file__)[:os.path.dirname(__file__).rfind("\\" )]
    target_encoding = input("Enter the number of target encoding (utf-8 - 1 or windows-1251 - 2): ")
    encodings = [' UTF-8-SIG', 'windows-1251']

    ignore_patterns = [
        '*/opencv/*',       # Игнорировать любую папку opencv в любом пути
        '*/OpenSSL/*',      # Игнорировать любую папку OpenSSL в любом пути
        '*.pyc',            # Игнорировать все файлы с расширением .pyc
        '__pycache__'       # Игнорировать папку __pycache__
    ]

    if target_encoding not in ['1', '2']:
        print("Invalid Invalid target encoding. Please enter '1' for 'utf-8' or '2' for 'windows-1251'.")
    else:
        convert_directory_encoding(target_directory, encodings[int(target_encoding) - 1], ignore_patterns)
        input("\nProgram finished. . .")
