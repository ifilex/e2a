import os
import tkinter as tk
from tkinter import messagebox, filedialog
from tkinter import ttk

class WineCFGApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Wine Config - Configure your Wine environment")
        self.root.geometry("500x250")

        # Cuadro de bienvenida
        welcome_label = tk.Label(root, text="Bienvenido a Wine Config\nEsta aplicación le permitirá configurar su entorno de Wine para emulación bajo Android.\nSeleccione la versión de Wine que desea usar.", wraplength=450, justify="center")
        welcome_label.pack(pady=20)

        # Frame para la selección de la versión de Wine
        self.version_frame = tk.Frame(root)
        self.version_frame.pack(pady=10)

        version_label = tk.Label(self.version_frame, text="Seleccione la versión de Wine:")
        version_label.grid(row=0, column=0, padx=10)

        self.version_selector = ttk.Combobox(self.version_frame, state="readonly")
        self.version_selector.grid(row=0, column=1)

        # Cargar las versiones disponibles en tools/roms
        self.load_versions()

        # Botones de Aplicar y Cancelar
        self.button_frame = tk.Frame(root)
        self.button_frame.pack(pady=20)

        apply_button = tk.Button(self.button_frame, text="Aplicar", command=self.apply_version)
        apply_button.grid(row=0, column=0, padx=10)

        cancel_button = tk.Button(self.button_frame, text="Cancelar", command=root.quit)
        cancel_button.grid(row=0, column=1, padx=10)

        # Cuadro de debug
        self.debug_window = None

    def load_versions(self):
        # Verificar el directorio tools/roms y listar los archivos zip
        roms_path = os.path.join("tools", "roms")
        if os.path.exists(roms_path):
            rom_files = [f.replace(".zip", "") for f in os.listdir(roms_path) if f.endswith(".zip")]
            self.version_selector['values'] = rom_files
        else:
            messagebox.showerror("Error", "El directorio tools/roms no existe. Asegúrese de que está configurado correctamente.")

    def apply_version(self):
        selected_version = self.version_selector.get()
        if not selected_version:
            messagebox.showwarning("Advertencia", "Por favor, seleccione una versión de Wine.")
            return

        # Mostrar la ventana de debug
        self.show_debug_window()

        # Comando para aplicar la versión de Wine
        command = f'winecfg.exe --version {selected_version}'

        # Ejecutar el comando usando os.system
        self.run_command(command)

    def show_debug_window(self):
        if self.debug_window is None or not tk.Toplevel.winfo_exists(self.debug_window):
            self.debug_window = tk.Toplevel(self.root)
            self.debug_window.title("Debug - WineCFG")
            self.debug_window.geometry("500x300")
            
            self.debug_text = tk.Text(self.debug_window, wrap="word")
            self.debug_text.pack(expand=True, fill="both")

    def run_command(self, command):
        # Ejecutar el comando y mostrar la salida en la ventana de debug
        process_output = os.popen(command).read()
        
        # Mostrar la salida del proceso en la ventana de debug
        self.debug_text.insert(tk.END, f"Ejecutando: {command}\n")
        self.debug_text.insert(tk.END, process_output)

        # Al finalizar, informar que el proceso ha completado
        self.debug_text.insert(tk.END, "\nProceso completado con éxito. Se ha cambiado a la versión de Wine seleccionada.\n")

        # Mensaje final para el usuario
        messagebox.showinfo("Éxito", f"Proceso completado con éxito. Se ha cambiado a la versión {self.version_selector.get()} de Wine.")

if __name__ == "__main__":
    root = tk.Tk()
    app = WineCFGApp(root)
    root.mainloop()

