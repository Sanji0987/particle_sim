import mmap
import struct
import sys
import tkinter as tk

SHM_NAME = "/particle_count"
MAX_PARTICLES = 5000

KEYBINDINGS = {
    "WASD": "Pan camera",
    "Scroll Wheel": "Zoom in/out",
    "Space": "Increase repulsion force",
    "P": "Open particle count slider",
    "H": "Show keybindings",
    "Escape": "Quit",
}


def open_help():
    root = tk.Tk()
    root.title("Keybindings")
    root.geometry("350x220")
    root.resizable(False, False)

    tk.Label(root, text="Keybindings", font=("Helvetica", 14, "bold")).pack(pady=(10, 5))

    bindings_frame = tk.Frame(root)
    bindings_frame.pack(padx=20, fill=tk.X)
    for key, action in KEYBINDINGS.items():
        row = tk.Frame(bindings_frame)
        row.pack(fill=tk.X, pady=2)
        tk.Label(row, text=key, font=("Helvetica", 10, "bold"), width=14, anchor=tk.W).pack(side=tk.LEFT)
        tk.Label(row, text=action, font=("Helvetica", 10), anchor=tk.W).pack(side=tk.LEFT)

    root.mainloop()


def open_slider():
    shm_fd = open(f"/dev/shm{SHM_NAME}", "r+b")
    shm_mmap = mmap.mmap(shm_fd.fileno(), 4)

    def read_count():
        shm_mmap.seek(0)
        return struct.unpack("i", shm_mmap.read(4))[0]

    def write_count(val):
        shm_mmap.seek(0)
        shm_mmap.write(struct.pack("i", int(float(val))))

    root = tk.Tk()
    root.title("Particle Count")
    root.geometry("400x100")
    root.resizable(False, False)

    slider = tk.Scale(root, from_=1, to=MAX_PARTICLES, orient=tk.HORIZONTAL,
                      length=350, command=write_count)
    slider.set(read_count())
    slider.pack(pady=20)

    def on_close():
        shm_mmap.close()
        shm_fd.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "help"
    if mode == "slider":
        open_slider()
    else:
        open_help()
