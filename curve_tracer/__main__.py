import tkinter as tk
from curve_tracer.app import CurveTracer

root = tk.Tk()
root.resizable(0, 0)
root.title("Curve Tracer - for(embed)")

try:
    root.iconbitmap('curve_tracer/images/forembed.ico')
except FileNotFoundError:
    pass

app = CurveTracer(root)
root.mainloop()
