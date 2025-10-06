import os
import zipfile
from tkinter import Tk, Label, Button, filedialog, Frame, messagebox

# Datos compartidos
app_data = {"folder": None, "exe": None, "zip_path": None, "step": 0}

def show_step():
    """Muestra la sección correspondiente al paso actual."""
    for frame in steps_frames:
        frame.pack_forget()
    steps_frames[app_data["step"]].pack(side="right", fill="both", expand=True)

    # Habilitar o deshabilitar botones según el paso
    back_button.config(state="normal" if app_data["step"] > 0 else "disabled")
    next_button.config(
        state="normal" if app_data["step"] < len(steps_frames) - 1 else "disabled"
    )

def next_step():
    """Avanza al siguiente paso o crea el ZIP en el último paso."""
    if validate_step():
        # Si estamos en el último paso y presionamos "Siguiente", crear el ZIP
        if app_data["step"] == len(steps_frames) - 1:
            create_zip()
        else:
            app_data["step"] += 1
            show_step()

def previous_step():
    """Regresa al paso anterior."""
    app_data["step"] -= 1
    show_step()

def validate_step():
    """Valida que el paso actual esté completo antes de avanzar."""
    if app_data["step"] == 1 and not app_data["folder"]:
        messagebox.showwarning("Advertencia", "Selecciona una carpeta antes de continuar.")
        return False
    if app_data["step"] == 2 and not app_data["exe"]:
        messagebox.showwarning("Advertencia", "Selecciona un ejecutable antes de continuar.")
        return False
    if app_data["step"] == 3 and not app_data["zip_path"]:
        messagebox.showwarning("Advertencia", "Selecciona dónde guardar el archivo ZIP.")
        return False
    return True


def select_folder():
    folder_path = filedialog.askdirectory(title="Selecciona la carpeta del programa")
    if folder_path:
        app_data["folder"] = folder_path
        folder_label.config(text=f"📁 {folder_path}")

def select_exe():
    exe_path = filedialog.askopenfilename(
        initialdir=app_data["folder"],
        title="Selecciona el ejecutable principal",
        filetypes=[("Archivos ejecutables", "*.exe")]
    )
    if exe_path:
        app_data["exe"] = exe_path
        exe_label.config(text=f"🖥️ {exe_path}")

def select_zip_location():
    zip_folder = filedialog.askdirectory(title="Selecciona dónde guardar el ZIP")
    if zip_folder:
        # Actualizar la ruta del archivo ZIP
        app_data["zip_path"] = os.path.join(zip_folder, os.path.basename(app_data["exe"]).replace(".exe", ".zip"))
        zip_label.config(text=f"📦 Guardar en: {app_data['zip_path']}")
        next_button.config(state="normal")  # Habilitar el botón Siguiente
    else:
        zip_label.config(text="📦 Ninguna ubicación seleccionada")

def create_zip():
    """Crea el archivo ZIP en la ubicación seleccionada."""
    try:
        progress_label.config(text="📊 Creando archivo ZIP... Por favor espera.")
        root.update_idletasks()  # Actualizar la interfaz gráfica mientras trabaja

        with zipfile.ZipFile(app_data["zip_path"], 'w', zipfile.ZIP_DEFLATED) as zip_file:
            for root_folder, _, files in os.walk(app_data["folder"]):
                for file in files:
                    file_path = os.path.join(root_folder, file)
                    arcname = os.path.relpath(file_path, app_data["folder"])
                    zip_file.write(file_path, arcname)

        progress_label.config(text="✔️ ZIP creado con éxito.")
        messagebox.showinfo("Éxito", f"Archivo ZIP creado en:\n{app_data['zip_path']}")
    except Exception as e:
        progress_label.config(text="❌ Error al crear el ZIP.")
        messagebox.showerror("Error", f"Ocurrió un error durante la creación del archivo ZIP:\n{e}")

def close_app():
    """Cierra la aplicación."""
    root.quit()

# Configuración de la ventana principal
root = Tk()
root.title("Wine ZIP Wizard")
root.geometry("600x400")
root.resizable(False, False)

# Panel izquierdo gris con texto "EXE2APK"
left_panel = Frame(root, bg="lightgray", width=180)
left_panel.pack(side="left", fill="y")
Label(left_panel, text="EXE2APK", bg="lightgray", font=("Segoe UI", 16), pady=20).pack(anchor="center")

# Frames para cada paso
steps_frames = []

# Paso 1: Bienvenida
welcome_frame = Frame(root, bg="white")
Label(welcome_frame, text="Bienvenido al Asistente WineZIP", font=("Segoe UI", 14), bg="white").pack(pady=10)
Label(
    welcome_frame,
    text="Este asistente te guiará para crear un archivo ZIP\n"
         "a partir de una carpeta y luego poder exportarlo como APK.",
    font=("Segoe UI", 12),
    bg="white",
    justify="center"
).pack(pady=10)
steps_frames.append(welcome_frame)

# Paso 2: Seleccionar carpeta
step1 = Frame(root, bg="white")
Label(step1, text="Paso 1: Selecciona la carpeta del programa", font=("Segoe UI", 14), bg="white").pack(pady=10)
folder_label = Label(step1, text="📁 Ninguna carpeta seleccionada", font=("Segoe UI", 12), bg="white")
folder_label.pack(pady=5)
Button(step1, text="Seleccionar carpeta", command=select_folder).pack(pady=10)
steps_frames.append(step1)

# Paso 3: Seleccionar ejecutable
step2 = Frame(root, bg="white")
Label(step2, text="Paso 2: Selecciona el ejecutable principal", font=("Segoe UI", 14), bg="white").pack(pady=10)
exe_label = Label(step2, text="🖥️ Ningún ejecutable seleccionado", font=("Segoe UI", 12), bg="white")
exe_label.pack(pady=5)
Button(step2, text="Seleccionar ejecutable", command=select_exe).pack(pady=10)
steps_frames.append(step2)

# Paso 4: Seleccionar ubicación del ZIP
step3 = Frame(root, bg="white")
Label(step3, text="Paso 3: Selecciona dónde guardar el archivo ZIP", font=("Segoe UI", 14), bg="white").pack(pady=10)
zip_label = Label(step3, text="📦 Ninguna ubicación seleccionada", font=("Segoe UI", 12), bg="white")
zip_label.pack(pady=5)
Button(step3, text="Seleccionar ubicación", command=select_zip_location).pack(pady=10)
progress_label = Label(step3, text="", font=("Segoe UI", 10), bg="white")
progress_label.pack(pady=5)
steps_frames.append(step3)

# Botones de navegación (orden correcto)
navigation_frame = Frame(root, bg="white")
navigation_frame.pack(side="bottom", fill="x", pady=10)

cancel_button = Button(navigation_frame, text="Cancelar", command=close_app, bg="red", fg="white")
cancel_button.pack(side="right", padx=5)

next_button = Button(navigation_frame, text="Siguiente >", command=next_step)
next_button.pack(side="right", padx=5)

back_button = Button(navigation_frame, text="< Anterior", command=previous_step, state="disabled")
back_button.pack(side="right", padx=5)

# Mostrar el primer paso
show_step()

# Ejecutar la aplicación
root.mainloop()
