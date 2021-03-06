import tkinter as tk
from PIL import Image, ImageTk


class ShortcutBar:
    HEIGHT = 30
    WIDTH = 30

    def __init__(self, parent):
        self.parent = parent

        self.btn_images = []
        self.buttons = []

    def add_btn(self, text=None, image_path=None, command=None, tag=None):
        if image_path:
            # make an image from the item and resize as appropriate
            img = Image.open(image_path)
            img = img.resize((self.HEIGHT, self.WIDTH), Image.ANTIALIAS)
            self.btn_images.append(ImageTk.PhotoImage(img))

            self.buttons.append(tk.Button(self.parent, text=text, image=self.btn_images[-1], command=command, tag=tag))

        else:
            self.buttons.append(tk.Button(self.parent, text=text, command=command, tag=tag))

        self.buttons[-1].grid(row=0, column=len(self.buttons)-1)

if __name__ == '__main__':
    root = tk.Tk()
    root.title("for(embed) - Curve Tracer")
    root.iconbitmap('images/forembed.ico')
    app = ShortcutBar(root)
    root.mainloop()
