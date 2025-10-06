import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os


class APKWizardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Wizard para crear APK")
        self.geometry("600x400")
        self.configure(bg="#F3F3F3")
        self.step = 1
        self.zip_file = None
        self.app_name = None
        self.include_icon = False
        self.icon_path = None

        self.create_widgets()

    def create_widgets(self):
        self.clear_frame()
        if self.step == 1:
            self.step_one()
        elif self.step == 2:
            self.step_two()
        elif self.step == 3:
            self.step_three()
        elif self.step == 4:
            self.step_four()
        elif self.step == 5:
            self.run_winecpp()

    def clear_frame(self):
        for widget in self.winfo_children():
            widget.destroy()

    def step_one(self):
        tk.Label(self, text="Paso 1: Seleccione el archivo ZIP", font=("Segoe UI", 14), bg="#F3F3F3").pack(pady=20)
        tk.Button(self, text="Seleccionar ZIP", command=self.select_zip).pack(pady=10)
        if self.zip_file:
            tk.Label(self, text=f"Archivo ZIP seleccionado:\n{self.zip_file}", font=("Segoe UI", 10), bg="#F3F3F3", wraplength=500).pack(pady=10)
        self.create_footer_buttons("Continuar >", self.go_to_next_step)

    def step_two(self):
        tk.Label(self, text="Paso 2: Ingrese el nombre de la aplicación", font=("Segoe UI", 14), bg="#F3F3F3").pack(pady=20)
        self.app_name_entry = tk.Entry(self, font=("Segoe UI", 12), width=30)
        self.app_name_entry.pack(pady=10)
        self.create_footer_buttons("Continuar >", self.go_to_next_step)

    def step_three(self):
        tk.Label(self, text="Paso 3: ¿Desea incluir un icono?", font=("Segoe UI", 14), bg="#F3F3F3").pack(pady=20)
        tk.Button(self, text="Incluir icono", command=self.select_icon).pack(pady=10)
        tk.Button(self, text="Usar icono genérico", command=self.skip_icon).pack(pady=10)
        if self.icon_path or not self.include_icon:
            icon_text = self.icon_path if self.include_icon else "Icono genérico seleccionado."
            tk.Label(self, text=icon_text, font=("Segoe UI", 10), bg="#F3F3F3").pack(pady=10)
        self.create_footer_buttons("Continuar >", self.go_to_next_step)

    def step_four(self):
        tk.Label(self, text="Paso 4: Confirme la información", font=("Segoe UI", 14), bg="#F3F3F3").pack(pady=20)
        tk.Label(self, text=f"Archivo ZIP: {self.zip_file}", font=("Segoe UI", 12), bg="#F3F3F3").pack(pady=5)
        tk.Label(self, text=f"Nombre de la App: {self.app_name}", font=("Segoe UI", 12), bg="#F3F3F3").pack(pady=5)
        icon_text = self.icon_path if self.include_icon else "Icono genérico"
        tk.Label(self, text=f"Icono: {icon_text}", font=("Segoe UI", 12), bg="#F3F3F3").pack(pady=5)
        self.create_footer_buttons("Crear APK >", self.go_to_next_step)

    def select_zip(self):
        file_path = filedialog.askopenfilename(filetypes=[("Archivos ZIP", "*.zip")])
        if file_path:
            self.zip_file = self.convert_to_windows_path(file_path)
            self.create_widgets()

    def select_icon(self):
        file_path = filedialog.askopenfilename(filetypes=[("Archivos PNG", "*.png")])
        if file_path:
            self.include_icon = True
            self.icon_path = self.convert_to_windows_path(file_path)
            self.create_widgets()

    def skip_icon(self):
        self.include_icon = False
        self.icon_path = None
        self.go_to_next_step()

    def go_to_next_step(self):
        if self.step == 2:
            self.app_name = self.app_name_entry.get()
            if not self.app_name:
                messagebox.showerror("Error", "Debe ingresar un nombre para la aplicación.")
                return
        self.step += 1
        self.create_widgets()

    def run_winecpp(self):
        self.clear_frame()
        tk.Label(self, text="Creando APK...", font=("Segoe UI", 14), bg="#F3F3F3").pack(pady=20)

        progress = tk.Text(self, width=60, height=10, state="disabled", bg="#F9F9F9")
        progress.pack(pady=10)

        def execute_command():
            try:
                cmd = ["winecpp.exe", self.zip_file, self.app_name]
                if self.include_icon:
                    cmd.append(self.icon_path)

                process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                for line in process.stdout:
                    progress.config(state="normal")
                    progress.insert(tk.END, line)
                    progress.see(tk.END)
                    progress.config(state="disabled")
                    self.update_idletasks()

                process.wait()
                if process.returncode == 0:
                    messagebox.showinfo("Éxito", "El archivo APK se creó correctamente.")
                else:
                    raise Exception("Ocurrió un error durante la creación del APK.")
            except Exception as e:
                messagebox.showerror("Error", f"Error al crear el APK:\n{e}")
            finally:
                self.create_footer_buttons("Cerrar", self.destroy)

        self.after(100, execute_command)

    def convert_to_windows_path(self, path):
        return path.replace("/", "\\")

    def create_footer_buttons(self, continue_text, continue_command):
        button_frame = tk.Frame(self, bg="#F3F3F3")
        button_frame.pack(side="bottom", fill="x", pady=10)

        tk.Button(button_frame, text="Cancelar", command=self.destroy).pack(side="left", padx=10, pady=5)
        tk.Button(button_frame, text=continue_text, command=continue_command).pack(side="right", padx=10, pady=5)


if __name__ == "__main__":
    app = APKWizardApp()
    app.mainloop()
