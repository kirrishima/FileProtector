import time
import pyperclip
import keyboard
import pygetwindow
import mouse


def transform(text_to_translate: str, original_clipboard: str):
    # print(text_to_translate)
    translated_text = text_to_translate[:-1]
    translated_text = translated_text.replace("std::cout << ", "")
    print(translated_text)
    translated_text = translated_text.replace("<< std::endl;", "")
    translated_text = translated_text.replace("std::cerr << ", "")
    translated_text = translated_text.replace("<< ", "+")
    translated_text = "printColoredMessage("+translated_text+", );\n"


    pyperclip.copy(translated_text)
    keyboard.press('ctrl+v')
    time.sleep(0.1)
    keyboard.release('ctrl+v')
    pyperclip.copy(original_clipboard)


def select_and_translate():
    original_clipborad = pyperclip.paste()
    pyperclip.copy("")
    mouse.click(button='left')
    time.sleep(0.05)
    mouse.click(button='left')
    time.sleep(0.05)
    mouse.click(button='left')

    keyboard.press('ctrl+c')
    time.sleep(0.01)
    keyboard.release('ctrl+c')
    
    text = pyperclip.paste()
    transform(text, original_clipborad)

x = 1
string = ""
def copy():
    global x, string
    mouse.click(button='left')

    pyperclip.copy(string)
    keyboard.press('ctrl+v')
    time.sleep(0.1)
    keyboard.release('ctrl+v')
    x+=1
    keyboard.press('ctrl+s')
    time.sleep(0.01)
    keyboard.release('ctrl+s')

def cut():
    global x, string
    mouse.click(button='left')
    time.sleep(0.05)
    mouse.click(button='left')
    keyboard.press('ctrl+x')
    time.sleep(0.01)
    keyboard.release('ctrl+x')

    string = pyperclip.paste()
    string = string.replace(";", "").removeprefix('SET_')

    keyboard.press('del')
    time.sleep(0.1)
    keyboard.release('del')

    keyboard.press('del')
    time.sleep(0.1)
    keyboard.release('del')
    
    x+=1

def s():
    mouse.click(button='left')
    time.sleep(0.05)
    mouse.click(button='left')
    keyboard.press('del')
    time.sleep(0.01)
    keyboard.release('del')

    keyboard.press('del')
    time.sleep(0.1)
    keyboard.release('del')

    keyboard.press('del')
    time.sleep(0.1)
def select_and():
    if x % 2 == 1:
        cut()
    else:
        copy()

keyboard.add_hotkey("f", select_and_translate, suppress=True) 
keyboard.add_hotkey("d", select_and, suppress=True) 
keyboard.add_hotkey("s", s, suppress=True) 
while True:
    try:
        keyboard.wait()
    except KeyboardInterrupt:
        break