import os
import chardet
import fnmatch
from colorama import Fore,  Style, init
from ftfy import guess_bytes

def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        raw_data = f.read()

    # Пробуем угадать, как должен выглядеть текст
    result, encoding = guess_bytes(raw_data)
    return encoding
# Инициализация colorama
init(autoreset=True)

def should_ignore(path, ignore_patterns):
    for pattern in ignore_patterns:
        if fnmatch.fnmatch(path, pattern):
            return True
    return False

def convert_file_encoding(file_path, target_encoding, current_encoding):
    with open(file_path, 'rb') as f:
        raw_data = f.read()

    # detected_encoding = chardet.detect(raw_data)['encoding']
    detected_encoding = detect_encoding(file_path)

    # if detected_encoding.lower() == target_encoding.lower():
    #     print(Fore.YELLOW + "File " + Fore.CYAN + f"{file_path}" + Fore.YELLOW + " is already in " + Fore.BLUE + f"{target_encoding}" + Fore.YELLOW + " encoding")
    #     return

    print(Fore.CYAN + Style.BRIGHT + f"Detected encoding: {detected_encoding}")
    try:
        detected_encoding = current_encoding
        print(Fore.CYAN + Style.BRIGHT + f"Parsing file as {detected_encoding}")
        text = raw_data.decode(detected_encoding)
        # print(text)
        normalized_text = text.replace('\r\n', '\n').replace('\r', '\n')
        # print(normalized_text)
        with open(file_path, 'w', encoding=target_encoding, newline='\n') as f:
            # print("ABOBA")
            f.write(normalized_text)
            # print("BOBA")
        print(Fore.GREEN +  Style.BRIGHT +"Converted " + Fore.CYAN + Style.BRIGHT +f"{file_path}" + Fore.GREEN + Style.BRIGHT +" from " + Fore.BLUE +Style.BRIGHT + detected_encoding + Fore.GREEN + Style.BRIGHT +" to " + Fore.BLUE + Style.BRIGHT +f"{target_encoding}")

    except Exception as e:
        print(Fore.RED + f"Error converting {file_path}: {e}")

def convert_directory_encoding(directory, target_encoding, current_encoding, ignore_patterns):
    for root, dirs, files in os.walk(directory):
        # Игнорирование директорий
        dirs[:] = [d for d in dirs if not should_ignore(os.path.join(root, d), ignore_patterns)]
        for file in files:
            file_path = os.path.join(root, file)
            if not should_ignore(file_path, ignore_patterns) and (file.endswith('.cpp') or file.endswith('.h')):
                convert_file_encoding(file_path, target_encoding, current_encoding)

if __name__ == "__main__":
    target_directory = os.path.dirname(__file__)[:os.path.dirname(__file__).rfind("\\" )]
    target_encoding = input("Enter the number of target encoding: 1) utf-8 or 2) windows-1251: ")
    encodings = ['UTF-8-SIG', 'windows-1251']

    ignore_patterns = [
        '*/opencv/*',       # Игнорировать любую папку opencv в любом пути
        '*/OpenSSL/*',      # Игнорировать любую папку OpenSSL в любом пути
        '*.pyc',            # Игнорировать все файлы с расширением .pyc
        '__pycache__'       # Игнорировать папку __pycache__
    ]

    if target_encoding not in ['1', '2']:
        print(Fore.RED + "Invalid Invalid target encoding. Please enter '1' for 'utf-8' or '2' for 'windows-1251'.")
    else:
        convert_directory_encoding(target_directory, encodings[int(target_encoding) - 1], encodings[abs(int(target_encoding) - 2)], ignore_patterns)
        input("\nProgram finished. . .")
