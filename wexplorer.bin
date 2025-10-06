import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import os
import threading

class ZipFileManagerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Zip File Manager")
        self.root.geometry("600x400")

        self.debug_mode = tk.BooleanVar()

        # Frame superior para selección de carpeta y checkbox de modo debug
        top_frame = tk.Frame(self.root)
        top_frame.pack(fill=tk.X, padx=10, pady=5)

        self.folder_label = tk.Label(top_frame, text="Select Folder:")
        self.folder_label.pack(side=tk.LEFT)

        self.folder_entry = tk.Entry(top_frame, width=40)
        self.folder_entry.pack(side=tk.LEFT, padx=5)

        self.browse_button = tk.Button(top_frame, text="Browse", command=self.browse_folder)
        self.browse_button.pack(side=tk.LEFT)

        self.debug_checkbox = tk.Checkbutton(top_frame, text="Enable Debug Mode", variable=self.debug_mode)
        self.debug_checkbox.pack(side=tk.LEFT, padx=5)

        # Scrollable frame to display zip files
        self.file_list_frame = tk.Frame(self.root)
        self.file_list_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.scroll_y = tk.Scrollbar(self.file_list_frame, orient=tk.VERTICAL)
        
        self.file_listbox = tk.Listbox(self.file_list_frame, selectmode=tk.SINGLE, yscrollcommand=self.scroll_y.set)
        self.file_listbox.pack(fill=tk.BOTH, expand=True, side=tk.LEFT)

        self.scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        self.scroll_y.config(command=self.file_listbox.yview)

                
        # Debug window
        self.debug_window = scrolledtext.ScrolledText(self.root, height=10, state='disabled', wrap=tk.WORD)
        self.debug_window.pack(fill=tk.X, padx=10, pady=5)

        # Binding double click event on the file list
        self.file_listbox.bind('<Double-1>', self.on_file_double_click)

        self.populate_file_list()

    def browse_folder(self):
        folder_selected = filedialog.askdirectory()
        if folder_selected:
            self.folder_entry.delete(0, tk.END)
            self.folder_entry.insert(0, folder_selected)
            self.populate_file_list()

    def populate_file_list(self):
        folder = self.folder_entry.get()
        self.file_listbox.delete(0, tk.END)
        if os.path.isdir(folder):
            zip_files = [f for f in os.listdir(folder) if f.endswith('.zip')]
            for file in zip_files:
                self.file_listbox.insert(tk.END, file)

    def on_file_double_click(self, event):
        selected = self.file_listbox.curselection()
        if selected:
            file_name = self.file_listbox.get(selected[0])
            full_path = os.path.join(self.folder_entry.get(), file_name)

            if self.debug_mode.get():
                command = f"wine.exe --debug {full_path}"
            else:
                command = f"wine.exe {full_path}"

            self.run_command(command)

    def run_command(self, command):
        self.debug_window.config(state='normal')
        self.debug_window.delete(1.0, tk.END)
        self.debug_window.insert(tk.END, f"Executing: {command}\n")
        self.debug_window.config(state='disabled')

        # Run command in a separate thread to avoid blocking the GUI
        thread = threading.Thread(target=self.execute_command, args=(command,))
        thread.start()

    def execute_command(self, command):
        process = os.popen(command)
        output = process.read()
        process.close()

        self.debug_window.config(state='normal')
        self.debug_window.insert(tk.END, output)
        self.debug_window.config(state='disabled')


if __name__ == "__main__":
    root = tk.Tk()
    app = ZipFileManagerApp(root)
    root.mainloop()
