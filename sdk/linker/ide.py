import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext, simpledialog
import os
import subprocess

# Funciones del menú Files
def new_file():
    editor.delete(1.0, tk.END)
    root.title("New Project.bat")

def open_file():
    file_path = filedialog.askopenfilename(filetypes=[("Batch Files", "*.bat")])
    if file_path:
        with open(file_path, 'r') as file:
            editor.delete(1.0, tk.END)
            editor.insert(tk.END, file.read())
        root.title(file_path)
        highlight_syntax()  # Resalta sintaxis al cargar archivo

def save_file():
    file_path = filedialog.asksaveasfilename(defaultextension=".bat", filetypes=[("Batch Files", "*.bat")])
    if file_path:
        with open(file_path, 'w') as file:
            file.write(editor.get(1.0, tk.END))
        root.title(file_path)

def dos_shell():
    os.system('cmd /k shell.bat')  # Lanza el shell DOS específico

def quit_ide():
    root.quit()

# Funciones del menú Edit
def undo_action():
    editor.edit_undo()

def redo_action():
    editor.edit_redo()

def cut_action():
    editor.event_generate('<<Cut>>')

def copy_action():
    editor.event_generate('<<Copy>>')

def paste_action():
    editor.event_generate('<<Paste>>')

def clear_action():
    editor.delete("sel.first", "sel.last")

# Funciones del menú Search
def find_text():
    target = simpledialog.askstring("Find", "Enter text to find:")
    content = editor.get(1.0, tk.END)
    start_idx = content.find(target)
    
    if start_idx != -1:
        end_idx = start_idx + len(target)
        editor.tag_add('highlight', f'1.0+{start_idx}c', f'1.0+{end_idx}c')
        editor.tag_config('highlight', background='yellow')
    else:
        messagebox.showinfo("Not found", f"'{target}' not found.")

def replace_text():
    find_str = simpledialog.askstring("Find", "Enter text to find:")
    replace_str = simpledialog.askstring("Replace", f"Replace '{find_str}' with:")
    content = editor.get(1.0, tk.END)
    new_content = content.replace(find_str, replace_str)
    editor.delete(1.0, tk.END)
    editor.insert(tk.END, new_content)

def go_to_line():
    line = simpledialog.askinteger("Go to line", "Enter line number:")
    editor.mark_set("insert", f"{line}.0")
    editor.see(f"{line}.0")

# Funciones del menú Run
def run_bat():
    file_path = filedialog.askopenfilename(filetypes=[("Batch Files", "*.bat")])
    if file_path:
        os.system(f'cmd /k {file_path}')  # Ejecuta el archivo .bat en un terminal de Windows

def winezip_pre_compiler():
    os.system('cmd /k winezip.bat')  # Ejecuta el pre-compilador winezip.bat

def run_progman():
    os.system('cmd /k progman.bat dock.zip')  # Ejecuta progman.bat

# Wizard para compilar APK
def compile_apk():
    def compile_wizard():
        app_name = app_name_entry.get()
        zip_file = zip_file_entry.get()
        icon_file = icon_file_entry.get()

        if not app_name or not zip_file:
            messagebox.showerror("Error", "App name and ZIP file are required.")
            return

        # Resumen de la compilación
        summary = f"App Name: {app_name}\nZIP File: {zip_file}\nIcon File: {icon_file or 'Default'}"
        messagebox.showinfo("Compilation Summary", summary)

        # Generar el archivo BAT para la compilación
        bat_code = f"""@echo off
REM Compiling {app_name} to APK
move "{zip_file}" "C:/path/to/exe2apk"
"""

        if icon_file:
            bat_code += f'copy "{icon_file}" "C:/path/to/exe2apk/icon.png"\n'

        bat_code += f'winecpp "{zip_file}" "{app_name}" "{icon_file or "default_icon.png"}"\n'
        editor.delete(1.0, tk.END)
        editor.insert(tk.END, bat_code)

    wizard = tk.Toplevel(root)
    wizard.title("APK Compilation Wizard")

    # Campos del wizard
    tk.Label(wizard, text="Application Name:").grid(row=0, column=0, padx=10, pady=5)
    app_name_entry = tk.Entry(wizard)
    app_name_entry.grid(row=0, column=1, padx=10, pady=5)

    tk.Label(wizard, text="ZIP File:").grid(row=1, column=0, padx=10, pady=5)
    zip_file_entry = tk.Entry(wizard)
    zip_file_entry.grid(row=1, column=1, padx=10, pady=5)
    tk.Button(wizard, text="Find...", command=lambda: zip_file_entry.insert(0, filedialog.askopenfilename(filetypes=[("ZIP Files", "*.zip")]))).grid(row=1, column=2, padx=10, pady=5)

    tk.Label(wizard, text="Icon (optional):").grid(row=2, column=0, padx=10, pady=5)
    icon_file_entry = tk.Entry(wizard)
    icon_file_entry.grid(row=2, column=1, padx=10, pady=5)
    tk.Button(wizard, text="Browse...", command=lambda: icon_file_entry.insert(0, filedialog.askopenfilename(filetypes=[("PNG Images", "*.png")]))).grid(row=2, column=2, padx=10, pady=5)

    tk.Button(wizard, text="Compile", command=compile_wizard).grid(row=3, column=0, columnspan=3, pady=10)

# Funciones del menú Proyect
def new_project():
    project_bat_code = """@echo off
REM Batch project template
REM move ZIP file to EXE2APK directory
move archivo.zip direccion_de_exe2pak_completa

REM Copy icon to EXE2APK (if exists)
copy icono.png direccion_de_exe2pak_completa

REM Compile the APK using winecpp
winecpp archivo.zip "nombre_del_programa" icono.png
"""
    editor.delete(1.0, tk.END)
    editor.insert(tk.END, project_bat_code)
    root.title("New Project.bat")

def run_winebox():
    messagebox.showinfo("WineBOX", "Running WineBOX with ZIP precompilation...")

# Funciones del menú Help
def about():
    messagebox.showinfo("About EXE2APK IDE", "EXE2APK IDE Version 1.12\n")

# Función para resaltar sintaxis de comandos batch
def highlight_syntax(event=None):
    editor.tag_remove("highlight_keyword", "1.0", tk.END)
    editor.tag_remove("highlight_comment", "1.0", tk.END)

    # Palabras clave del lenguaje batch
    keywords = ['@echo', 'move', 'copy', 'winecpp', 'if', 'else', 'set', 'goto', 'call', 'exit', 'pause', 'rem', '@', 'wine', 'winecfg','progman','dock','winefile']
    
    content = editor.get("1.0", tk.END)
    
    # Resaltar comentarios (líneas que empiezan con REM)
    start_pos = "1.0"
    while True:
        start_pos = editor.search(r"rem", start_pos, stopindex=tk.END, regexp=True)
        if not start_pos:
            break
        end_pos = f"{start_pos} lineend"
        editor.tag_add("highlight_comment", start_pos, end_pos)
        editor.tag_config("highlight_comment", foreground="green")  # Comentarios en verde
        start_pos = end_pos
    
    # Resaltar palabras clave
    for keyword in keywords:
        start_pos = "1.0"
        while True:
            start_pos = editor.search(rf"\b{keyword}\b", start_pos, stopindex=tk.END, regexp=True)
            if not start_pos:
                break
            end_pos = f"{start_pos}+{len(keyword)}c"
            editor.tag_add("highlight_keyword", start_pos, end_pos)
            editor.tag_config("highlight_keyword", foreground="blue")  # Comandos en azul
            start_pos = end_pos

# Configuración principal del IDE
root = tk.Tk()
root.title("EXE2APK IDE - Untitled")
root.geometry("640x480")

# Menú principal
menu_bar = tk.Menu(root)

# Menú Files
file_menu = tk.Menu(menu_bar, tearoff=0)
file_menu.add_command(label="New", command=new_file)
file_menu.add_command(label="Open...", command=open_file)
file_menu.add_command(label="Save", command=save_file)
file_menu.add_separator()
file_menu.add_command(label="DOS Shell", command=dos_shell)
file_menu.add_command(label="Quit", command=quit_ide)
menu_bar.add_cascade(label="Files", menu=file_menu)

# Menú Edit
edit_menu = tk.Menu(menu_bar, tearoff=0)
edit_menu.add_command(label="Undo", command=undo_action)
edit_menu.add_command(label="Redo", command=redo_action)
edit_menu.add_separator()
edit_menu.add_command(label="Cut", command=cut_action)
edit_menu.add_command(label="Copy", command=copy_action)
edit_menu.add_command(label="Paste", command=paste_action)
edit_menu.add_command(label="Clear", command=clear_action)
menu_bar.add_cascade(label="Edit", menu=edit_menu)

# Menú Search
search_menu = tk.Menu(menu_bar, tearoff=0)
search_menu.add_command(label="Find", command=find_text)
search_menu.add_command(label="Replace...", command=replace_text)
search_menu.add_command(label="Go to line number...", command=go_to_line)
menu_bar.add_cascade(label="Search", menu=search_menu)

# Menú Run
run_menu = tk.Menu(menu_bar, tearoff=0)
run_menu.add_command(label="Run...", command=run_bat)
run_menu.add_command(label="WineZIP Pre-Compiler", command=winezip_pre_compiler)
run_menu.add_command(label="Progman", command=run_progman)
menu_bar.add_cascade(label="Run", menu=run_menu)

# Menú Compile
compile_menu = tk.Menu(menu_bar, tearoff=0)
compile_menu.add_command(label="Compile", command=compile_apk)
menu_bar.add_cascade(label="Compile", menu=compile_menu)

# Menú Proyect
project_menu = tk.Menu(menu_bar, tearoff=0)
project_menu.add_command(label="New Project", command=new_project)
project_menu.add_command(label="Run WineBOX", command=run_winebox)
menu_bar.add_cascade(label="Proyect", menu=project_menu)

# Menú Help
help_menu = tk.Menu(menu_bar, tearoff=0)
help_menu.add_command(label="About", command=about)
menu_bar.add_cascade(label="Help", menu=help_menu)

root.config(menu=menu_bar)

# Editor de texto con desplazamiento y resaltado de sintaxis
editor = scrolledtext.ScrolledText(root, wrap=tk.WORD, undo=True)
editor.pack(fill=tk.BOTH, expand=1)
editor.bind('<KeyRelease>', highlight_syntax)  # Activar resaltado de sintaxis al escribir

root.mainloop()
