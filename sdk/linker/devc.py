import tkinter as tk
from tkinter import ttk, filedialog, messagebox, colorchooser
import subprocess
import re

class SimpleIDE(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Custom IDE")
        self.geometry("1000x600")

        # Variables de colores
        self.bg_color = "#FFFFFF"
        self.text_color = "#000000"
        self.comment_color = "#007f00"
        self.keyword_color = "#00007f"
        self.error_color = "#FF0000"

        # Variables del programa
        self.current_file = None
        self.debug_visible = True

        # Menú principal
        self.create_menu()

        # Editor de texto
        self.text_area = tk.Text(self, wrap="none", undo=True, bg=self.bg_color, fg=self.text_color, font=("Courier New", 10))
        self.text_area.pack(expand=True, fill=tk.BOTH)

        # Numeración de líneas
        self.line_numbers = tk.Text(self, width=5, bg="#f0f0f0", fg="#000", bd=0, highlightthickness=0, font=("Courier New", 10))
        self.line_numbers.pack(side=tk.LEFT, fill=tk.Y)
        
        self.scrollbar = tk.Scrollbar(self, command=self.on_scroll)
        self.scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.text_area.config(yscrollcommand=self.scrollbar.set)

        # Área de depuración
        self.debug_frame = tk.Frame(self)
        self.debug_output = tk.Text(self.debug_frame, height=10, bg="#eaeaea")
        self.debug_output.pack(expand=True, fill=tk.BOTH)
        self.debug_frame.pack(side=tk.BOTTOM, fill=tk.X)

        # Botón para ocultar/mostrar el área de depuración
        self.toggle_debug_button = tk.Button(self, text="Minimizar Debug", command=self.toggle_debug)
        self.toggle_debug_button.pack(side=tk.BOTTOM)

        # Resaltado de sintaxis
        self.text_area.bind("<KeyRelease>", self.highlight_syntax)

    def create_menu(self):
        menu_bar = tk.Menu(self)

        # Menú Archivo
        file_menu = tk.Menu(menu_bar, tearoff=0)
        file_menu.add_command(label="Nuevo Proyecto", command=self.new_project)
        file_menu.add_command(label="Abrir", command=self.open_file)
        file_menu.add_command(label="Guardar", command=self.save_file)
        file_menu.add_separator()
        file_menu.add_command(label="Salir", command=self.quit)
        menu_bar.add_cascade(label="Archivo", menu=file_menu)

        # Menú Configuración
        settings_menu = tk.Menu(menu_bar, tearoff=0)
        settings_menu.add_command(label="Configuración de Colores", command=self.open_color_settings)
        settings_menu.add_command(label="Configuración de Programas", command=self.open_program_settings)
        menu_bar.add_cascade(label="Configuración", menu=settings_menu)

        self.config(menu=menu_bar)

    def new_project(self):
        self.current_file = None
        self.text_area.delete(1.0, tk.END)
        self.line_numbers.delete(1.0, tk.END)

    def open_file(self):
        file_path = filedialog.askopenfilename()
        if file_path:
            self.current_file = file_path
            with open(file_path, 'r') as file:
                content = file.read()
                self.text_area.delete(1.0, tk.END)
                self.text_area.insert(tk.END, content)
                self.update_line_numbers()

    def save_file(self):
        if not self.current_file:
            self.current_file = filedialog.asksaveasfilename(defaultextension=".cpp")
        if self.current_file:
            with open(self.current_file, 'w') as file:
                file.write(self.text_area.get(1.0, tk.END))

    def toggle_debug(self):
        if self.debug_visible:
            self.debug_frame.pack_forget()
            self.toggle_debug_button.config(text="Mostrar Debug")
        else:
            self.debug_frame.pack(side=tk.BOTTOM, fill=tk.X)
            self.toggle_debug_button.config(text="Minimizar Debug")
        self.debug_visible = not self.debug_visible

    def open_color_settings(self):
        color_window = tk.Toplevel(self)
        color_window.title("Configuración de Colores")

        def set_color(label, attr):
            color = colorchooser.askcolor()[1]
            if color:
                label.config(bg=color)
                setattr(self, attr, color)
                self.text_area.config(bg=self.bg_color, fg=self.text_color)

        # Color de fondo
        bg_label = tk.Label(color_window, text="Color de fondo del editor:")
        bg_label.grid(row=0, column=0, padx=10, pady=10)
        bg_color_display = tk.Label(color_window, bg=self.bg_color, width=20)
        bg_color_display.grid(row=0, column=1, padx=10, pady=10)
        tk.Button(color_window, text="Cambiar", command=lambda: set_color(bg_color_display, "bg_color")).grid(row=0, column=2)

        # Color del texto
        text_label = tk.Label(color_window, text="Color del texto:")
        text_label.grid(row=1, column=0, padx=10, pady=10)
        text_color_display = tk.Label(color_window, bg=self.text_color, width=20)
        text_color_display.grid(row=1, column=1, padx=10, pady=10)
        tk.Button(color_window, text="Cambiar", command=lambda: set_color(text_color_display, "text_color")).grid(row=1, column=2)

        # Color de comentarios
        comment_label = tk.Label(color_window, text="Color de comentarios:")
        comment_label.grid(row=2, column=0, padx=10, pady=10)
        comment_color_display = tk.Label(color_window, bg=self.comment_color, width=20)
        comment_color_display.grid(row=2, column=1, padx=10, pady=10)
        tk.Button(color_window, text="Cambiar", command=lambda: set_color(comment_color_display, "comment_color")).grid(row=2, column=2)

        # Color de palabras clave
        keyword_label = tk.Label(color_window, text="Color de palabras clave:")
        keyword_label.grid(row=3, column=0, padx=10, pady=10)
        keyword_color_display = tk.Label(color_window, bg=self.keyword_color, width=20)
        keyword_color_display.grid(row=3, column=1, padx=10, pady=10)
        tk.Button(color_window, text="Cambiar", command=lambda: set_color(keyword_color_display, "keyword_color")).grid(row=3, column=2)

        tk.Button(color_window, text="Guardar", command=color_window.destroy).grid(row=4, column=1, pady=20)

    def open_program_settings(self):
        program_window = tk.Toplevel(self)
        program_window.title("Configuración de Programas")
        program_window.geometry("400x300")

        programs = ["gcc", "g++", "make", "gdb", "windres", "gprof"]
        for idx, program in enumerate(programs):
            frame = ttk.Frame(program_window)
            frame.pack(pady=5, anchor=tk.W)

            tk.Label(frame, text=f"{program}:").pack(side=tk.LEFT)
            entry = tk.Entry(frame, width=40)
            entry.pack(side=tk.LEFT, padx=5)

    def compile_code(self):
        if not self.current_file:
            messagebox.showwarning("Advertencia", "Primero guarda el archivo antes de compilar.")
            return

        self.debug_output.delete(1.0, tk.END)

        # Comando de compilación
        command = f"gcc {self.current_file} -o {self.current_file.split('.')[0]}.exe"
        try:
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, text=True)
            self.debug_output.insert(tk.END, result.stdout + result.stderr)
            self.highlight_errors(result.stderr)
        except Exception as e:
            self.debug_output.insert(tk.END, f"Error durante la compilación: {e}\n")

    def highlight_errors(self, error_output):
        lines = error_output.split('\n')
        for line in lines:
            if "error" in line:
                line_num = int(re.search(r'(\d+)', line).group(1))
                self.text_area.tag_add("error", f"{line_num}.0", f"{line_num}.end")
                self.text_area.tag_config("error", foreground=self.error_color)

    def on_scroll(self, *args):
        self.text_area.yview(*args)
        self.line_numbers.yview(*args)

    def highlight_syntax(self, event=None):
        # Implementar la lógica para resaltar la sintaxis según el lenguaje
        self.update_line_numbers()

    def update_line_numbers(self):
        line_count = int(self.text_area.index('end-1c').split('.')[0])
        self.line_numbers.delete(1.0, tk.END)
        for i in range(1, line_count + 1):
            self.line_numbers.insert(tk.END, str(i) + '\            self.line_numbers.insert(tk.END, str(i) + '\n')

    def run(self):
        self.mainloop()

if __name__ == "__main__":
    ide = SimpleIDE()
    ide.run()
