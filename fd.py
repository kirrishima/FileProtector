import os

# Конфигурационный словарь
config = {
    "show_absolute_path": False,
    "file_path": "filesystem_tree.txt",  # Показывать абсолютный путь или нет
    "skip_content_folders": [
        r"C:\Users\Force\Documents\FileProtector\FileProtector\opencv\build\install\include",
        r"HashingLib\OpenSSL\include",
        r"HashingLib\OpenSSL\lib",
        r"FileProtector\opencv\build\install\x64\vc17\staticlib",
        ".venv"
    ],  # Папки, содержимое которых нужно пропустить
    # Расширения файлов, которые нужно игнорировать
    "ignore_extensions": [".log", ".vcxproj", ".filters", ".user", ".pdb"],
    # Папки, которые нужно игнорировать полностью
    "ignore_folders": [
        ".vs",
        r"x64",
        r"FileProtector\opencv\build\install\x64\vc17\bin",
        "VIDS",
        "IMG",
        ".git",
                ".venv2"
    ],
    # Файлы, которые нужно игнорировать
    "ignore_files": [
        "skip_this_file.txt",
        r"Новый текстовый документ.txt",
        os.path.basename(__file__),
        "filesystem_tree.txt",
        ".gitignore",
        "sasa.py"
    ],
    "exceptions": ["x64", r"FileProtector\opencv\build\install\x64"],
    "bypass_exceptions": [
        r"C:\Users\Force\Documents\FileProtector\x64\Release\FileProtector.pdb",
        r"C:\Users\Force\Documents\FileProtector\x64\Release\HashingLib.pdb",
        r"C:\Users\Force\Documents\FileProtector\x64\Release\VideoEncryptorLib.pdb",
    ]
    # Исключения
}


def preprocess(directory: str):
    with open(config["file_path"], "w") as file:
        pass

    for i in range(len(config["exceptions"])):
        if not ":" in config["exceptions"][i]:
            config["exceptions"][i] = os.path.join(directory, config["exceptions"][i])


def should_skip_folder(full_path, relative_path, folder_list):
    return any(
        full_path.endswith(folder) or relative_path.startswith(folder)
        for folder in folder_list
    )


def should_ignore_folder(full_path, relative_path, folder_list):
    return any(
        full_path.endswith(folder) or relative_path.startswith(folder)
        for folder in folder_list
    )


def should_ignore_file(full_path, relative_path, file_list):
    return any(
        full_path.endswith(file) or relative_path.startswith(file) for file in file_list
    )


def is_exception(full_path, directory, exception_list):

    for i in range(len(config["exceptions"])):

        child_path = full_path
        parent_path = config["exceptions"][i]

        if full_path == exception_list[i] or (
            os.path.commonpath([child_path, parent_path]) == parent_path
            and full_path not in config["bypass_exceptions"]
        ):
            return True
    return False


def process_tree_in_file():
    IGNORE_CHARS = "└├│"
    file_path = config["file_path"]
    final_file_path = "final_" + file_path

    with open(file_path, "r", encoding="utf-8") as ifstream:
        lines = ifstream.readlines()

    # Обработка строк
    for i in range(len(lines) - 1):
        line = lines[i]
        next_line = lines[i + 1]

        # Обработка '└'
        index = line.rfind("└")
        if index != -1 and len(next_line) > index and next_line[index] == "│":
            next_line = next_line[:index] + " " + next_line[index + 1 :]
            lines[i + 1] = next_line

        # Обработка '├'
        index = line.rfind("├")
        if (
            index != -1
            and len(next_line) > index
            and next_line[index] not in IGNORE_CHARS
        ):
            line = line[:index] + "└" + line[index + 1 :]
            lines[i] = line

    # Запись измененных строк
    with open(final_file_path, "w", encoding="utf-8") as ofstream:
        ofstream.writelines(lines)

    # Дополнительная обработка последней строки
    with open(final_file_path, "r+", encoding="utf-8") as ifstream:
        lines = ifstream.readlines()

        if lines[-1][0] == "├":
            lines[-1] = "└" + lines[-1][1:]
        else:
            lines[-1] = lines[-1].replace("├", "└")
            if lines[-1].find('│') != -1:
                for i in range(len(lines) - 1, -1, -1):
                    if lines[i][0] == "├":
                        lines[i] = "└" + lines[i][1:]
                        break
                    lines[i] = " " + lines[i][1:]

        # Переместить указатель файла в начало и записать измененные строки
        ifstream.seek(0)
        ifstream.writelines(lines)
        # Обрезать файл до новой длины, если новые данные короче старых
        ifstream.truncate()


def generate_tree_recursive(directory, base_directory=None, prefix=""):

    if base_directory is None:
        base_directory = directory
        # Форматируем вывод в зависимости от конфигурации
        if config["show_absolute_path"]:
            print(directory)
        else:
            print(directory.replace("/", "\\").split("\\")[-1])
            with open(config["file_path"], "a", encoding="utf-8") as file:
                file.write(directory.replace("/", "\\").split("\\")[-1] + "\n")

    # Получаем список всех файлов и папок в текущей директории
    contents = os.listdir(directory)
    pointers = ["├── "] * (len(contents) - 1) + ["└── "]
    # print(pointers, " 1111111111 ", contents)
    for pointer, path in zip(pointers, contents):
        full_path = os.path.join(directory, path)
        relative_path = os.path.relpath(full_path, base_directory)

        # Получаем базовое имя файла или папки
        name = os.path.basename(full_path)

        # Пропускаем исключения
        if is_exception(full_path, directory, config["exceptions"]):
            pass
        # Игнорируем папки, указанные в конфиге
        elif os.path.isdir(full_path) and should_ignore_folder(
            full_path, relative_path, config["ignore_folders"]
        ):
            continue
        # Игнорируем файлы, указанные в конфиге
        elif os.path.isfile(full_path) and should_ignore_file(
            full_path, relative_path, config["ignore_files"]
        ):
            continue
        # Игнорируем файлы с указанными расширениями
        elif os.path.isfile(full_path) and any(
            name.endswith(ext if ext.startswith(".") else "." + ext)
            for ext in config["ignore_extensions"]
        ):
            continue

        with open(config["file_path"], "a", encoding="utf-8") as file:
            file.write(prefix + pointer + name + "\n")

        # Если это директория, рекурсивно вызываем функцию для этой директории
        if os.path.isdir(full_path):
            if should_skip_folder(
                full_path, relative_path, config["skip_content_folders"]
            ):
                with open(config["file_path"], "a", encoding="utf-8") as file:
                    file.write(prefix + "│   └── ◯ ◯ ◯\n")
            else:
                extension = (
                    "│   " if (pointer == "├── " and contents[-1] != path) else "    "
                )
                generate_tree_recursive(full_path, base_directory, prefix + extension)


def generate_tree(directory):
    preprocess(directory)
    generate_tree_recursive(directory)
    process_tree_in_file()


# Пример использования
base_directory = os.path.abspath(__file__).replace(
    "\\" + os.path.basename(__file__), ""
)
base_directory = base_directory[:2].capitalize() + base_directory[2:]
# print(base_directory)
generate_tree(base_directory)
input()
