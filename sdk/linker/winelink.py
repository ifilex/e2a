import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess


class APKWizardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("APK Creator Wizard")
        self.geometry("800x600")
        self.configure(bg="#2E3440")
        self.step = 1
        self.zip_file = None
        self.app_name = None
        self.include_icon = False
        self.icon_path = None

        self.create_widgets()

    def create_widgets(self):
        self.clear_frame()
        self.create_header()
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

    def create_header(self):
        header = tk.Label(
            self,
            text="Bienvenido al Asistente de Creación de APK",
            font=("Segoe UI", 24, "bold"),
            bg="#88C0D0",
            fg="white",
            padx=10,
            pady=10,
        )
        header.pack(fill="x")

        description = tk.Label(
            self,
            text="Este asistente te ayudará a convertir un archivo ZIP en un APK para Android.\n"
                 "Sigue los pasos a continuación para configurar tu aplicación.",
            font=("Segoe UI", 18),
            bg="#2E3440",
            fg="#D8DEE9",
        )
        description.pack(pady=10)

    def step_one(self):
        tk.Label(
            self,
            text="Paso 1: Seleccionar el archivo ZIP",
            font=("Segoe UI", 20, "bold"),
            bg="#2E3440",
            fg="#A3BE8C",
        ).pack(pady=20)
        tk.Label(
            self,
            text="Elige el archivo ZIP que contiene los recursos de tu aplicación.",
            font=("Segoe UI", 16),
            bg="#2E3440",
            fg="#D8DEE9",
        ).pack(pady=5)

        tk.Button(
            self,
            text="Seleccionar ZIP",
            command=self.select_zip,
            font=("Segoe UI", 16),
            bg="#5E81AC",
            fg="white",
            relief="flat",
            padx=20,
            pady=10,
        ).pack(pady=10)

        if self.zip_file:
            tk.Label(
                self,
                text=f"Archivo seleccionado: {self.zip_file}",
                font=("Segoe UI", 14),
                bg="#2E3440",
                fg="#EBCB8B",
                wraplength=700,
            ).pack(pady=10)

        self.create_footer_buttons("Siguiente >", self.go_to_next_step, None)

    def step_two(self):
        tk.Label(
            self,
            text="Paso 2: Ingresar el nombre de la aplicación",
            font=("Segoe UI", 20, "bold"),
            bg="#2E3440",
            fg="#A3BE8C",
        ).pack(pady=20)
        tk.Label(
            self,
            text="Escribe el nombre de la aplicación que deseas crear.",
            font=("Segoe UI", 16),
            bg="#2E3440",
            fg="#D8DEE9",
        ).pack(pady=5)

        self.app_name_entry = tk.Entry(self, font=("Segoe UI", 16), width=40)
        self.app_name_entry.pack(pady=10)

        self.create_footer_buttons("Siguiente >", self.go_to_next_step, self.go_to_previous_step)

    def step_three(self):
        tk.Label(
            self,
            text="Paso 3: Seleccionar un icono",
            font=("Segoe UI", 20, "bold"),
            bg="#2E3440",
            fg="#A3BE8C",
        ).pack(pady=20)
        tk.Label(
            self,
            text="Puedes elegir un icono para personalizar tu aplicación o usar uno genérico.",
            font=("Segoe UI", 16),
            bg="#2E3440",
            fg="#D8DEE9",
        ).pack(pady=5)

        tk.Button(
            self,
            text="Seleccionar icono",
            command=self.select_icon,
            font=("Segoe UI", 16),
            bg="#5E81AC",
            fg="white",
            relief="flat",
            padx=20,
            pady=10,
        ).pack(pady=10)

        tk.Button(
            self,
            text="Usar icono genérico",
            command=self.skip_icon,
            font=("Segoe UI", 16),
            bg="#4C566A",
            fg="white",
            relief="flat",
            padx=20,
            pady=10,
        ).pack(pady=10)

        if self.icon_path or not self.include_icon:
            icon_text = f"Icono seleccionado: {self.icon_path}" if self.include_icon else "Usando icono genérico."
            tk.Label(
                self,
                text=icon_text,
                font=("Segoe UI", 14),
                bg="#2E3440",
                fg="#EBCB8B",
            ).pack(pady=10)

        self.create_footer_buttons("Siguiente >", self.go_to_next_step, self.go_to_previous_step)

    def step_four(self):
        tk.Label(
            self,
            text="Paso 4: Confirmar la información",
            font=("Segoe UI", 20, "bold"),
            bg="#2E3440",
            fg="#A3BE8C",
        ).pack(pady=20)
        tk.Label(
            self,
            text=f"Archivo ZIP: {self.zip_file}\n"
                 f"Nombre de la aplicación: {self.app_name}\n"
                 f"Icono: {self.icon_path if self.include_icon else 'Icono genérico'}",
            font=("Segoe UI", 16),
            bg="#2E3440",
            fg="#D8DEE9",
        ).pack(pady=10)

        self.create_footer_buttons("Siguiente >", self.go_to_next_step, self.go_to_previous_step)

    def run_winecpp(self):
        self.clear_frame()
        tk.Label(
            self,
            text="Creando APK...",
            font=("Segoe UI", 20, "bold"),
            bg="#2E3440",
            fg="#A3BE8C",
        ).pack(pady=20)

        progress = tk.Text(self, width=80, height=15, state="disabled", bg="#3B4252", fg="#D8DEE9", font=("Consolas", 14))
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
                tk.Button(self, text="Cerrar", command=self.destroy, bg="#5E81AC", fg="white", font=("Segoe UI", 16)).pack(pady=10)

        self.after(100, execute_command)

    def select_zip(self):
        file_path = filedialog.askopenfilename(filetypes=[("Archivos ZIP", "*.zip")])
        if file_path:
            self.zip_file = file_path
            self.create_widgets()

    def select_icon(self):
        file_path = filedialog.askopenfilename(filetypes=[("Archivos PNG", "*.png")])
        if file_path:
            self.include_icon = True
            self.icon_path = file_path
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

    def go_to_previous_step(self):
        if self.step > 1:
            self.step -= 1
            self.create_widgets()

    def create_footer_buttons(self, continue_text, continue_command, previous_command):
        button_frame = tk.Frame(self, bg="#2E3440")
        button_frame.pack(side="bottom", fill="x", pady=10)

        if previous_command:
            tk.Button(
                button_frame,
                text="< Anterior",
                command=previous_command,
                font=("Segoe UI", 16),
                bg="#BF616A",
                fg="white",
                relief="flat",
                padx=20,
                pady=10,
            ).pack(side="left", padx=10)

        tk.Button(
            button_frame,
            text=continue_text,
            command=continue_command,
            font=("Segoe UI", 16),
            bg="#A3BE8C",
            fg="white",
            relief="flat",
            padx=20,
            pady=10,
        ).pack(side="right", padx=10)


if __name__ == "__main__":
    app = APKWizardApp()
    app.mainloop()
