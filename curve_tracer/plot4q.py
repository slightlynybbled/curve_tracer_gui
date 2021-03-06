import tkinter as tk


class Plot4Q(tk.Frame):
    DEFAULT_BG_COLOR = 'grey'
    DEFAULT_LINE_COLOR = '#39FF14'

    def __init__(self, master, x_pixels=200, y_pixels=200, xrange=1.0, yrange=1.0, grid=False, x_axis_label_str=None, y_axis_label_str=None):
        self.parent = master
        super().__init__(self.parent)

        self.width_px = x_pixels
        self.height_px = y_pixels

        self.x_per_pixel = xrange/x_pixels
        self.y_per_pixel = yrange/y_pixels
        self.grid = grid

        self.plot = tk.Canvas(self.parent, width=x_pixels, height=y_pixels, background=self.DEFAULT_BG_COLOR)
        self.plot.grid()

        self.draw_axes()
        self.draw_grid()

        self.label_x_axis(x_axis_label_str)
        self.label_y_axis(y_axis_label_str)

        self.plot_series_number = 0

        self.current_points = {}

    def remove_points(self):
        self.plot.delete('data_point')

    def remove_lines(self):
        self.plot.delete('data_line')

    def draw_axes(self):
        # draw the primary axes
        x0, y0 = self.to_screen_coords(-self.width_px / 2, 0)
        x1, y1 = self.to_screen_coords(self.width_px / 2, 0)
        x_axis = self.plot.create_line(x0, y0, x1, y1, tag='x_axis')

        x0, y0 = self.to_screen_coords(0, self.height_px / 2)
        x1, y1 = self.to_screen_coords(0, -self.height_px / 2)
        y_axis = self.plot.create_line(x0, y0, x1, y1, tag='y-axis')

    def draw_grid(self):
        if self.grid:
            # create the grid
            x_grid_interval_px = self.width_px / 10
            y_grid_interval_px = self.height_px / 10
            dash_tuple = (1, 1)

            for i in range(4):
                # top to bottom lines, right quadrants
                grid_x = (i + 1) * x_grid_interval_px
                grid_y = self.height_px / 2
                x1, y1 = self.to_screen_coords(grid_x, grid_y)
                x1, y2 = self.to_screen_coords(grid_x, -grid_y)
                self.plot.create_line(x1, y1, x1, y2, dash=dash_tuple, tag='grid')

                # top to bottom lines, left quadrants
                grid_x = -(i + 1) * x_grid_interval_px
                grid_y = self.height_px / 2
                x1, y1 = self.to_screen_coords(grid_x, grid_y)
                x1, y2 = self.to_screen_coords(grid_x, -grid_y)
                self.plot.create_line(x1, y1, x1, y2, dash=dash_tuple, tag='grid')

                # left-to-right lines, upper quadrants
                grid_x = self.width_px / 2
                grid_y = (i + 1) * y_grid_interval_px
                x1, y1 = self.to_screen_coords(grid_x, grid_y)
                x2, y1 = self.to_screen_coords(-grid_x, grid_y)
                self.plot.create_line(x1, y1, x2, y1, dash=dash_tuple, tag='grid')

                # left-to-right lines, lower quadrants
                grid_x = self.width_px / 2
                grid_y = -(i + 1) * y_grid_interval_px
                x1, y1 = self.to_screen_coords(grid_x, grid_y)
                x2, y1 = self.to_screen_coords(-grid_x, grid_y)
                self.plot.create_line(x1, y1, x2, y1, dash=dash_tuple, tag='grid')

    def label_x_axis(self, label):
        if label:
            self.plot.create_text((self.width_px - 5, (self.height_px/2)+5), text=label, anchor=tk.NE, tag='x-axis-label')
        else:
            pass

    def label_y_axis(self, label):
        if label:
            self.plot.create_text((self.width_px/2 + 5, 5), text=label, anchor=tk.NW, tag='y-axis-label')
        else:
            pass

    def plot_line(self, first_point, second_point, point_format=None, fill=None, tag='data_line'):
        if not fill:
            fill = self.DEFAULT_LINE_COLOR

        x0, y0 = first_point
        x1, y1 = second_point

        if point_format != 'px':
            x0 /= self.x_per_pixel
            x1 /= self.x_per_pixel
            y0 /= self.y_per_pixel
            y1 /= self.y_per_pixel

        x0_screen, y0_screen = self.to_screen_coords(x0, y0)
        x1_screen, y1_screen = self.to_screen_coords(x1, y1)

        self.plot.create_line(x0_screen,
                              y0_screen,
                              x1_screen,
                              y1_screen,
                              fill=fill,
                              width=3.0,
                              tag=tag)

    def plot_point(self, point, fill='green', tag='data_point'):
        if not fill:
            fill = self.DEFAULT_LINE_COLOR

        # find the location of the point on the canvas
        x, y = point

        x /= self.x_per_pixel
        y /= self.y_per_pixel

        x_screen, y_screen = self.to_screen_coords(x, y)

        point_radius = 2

        x0 = x_screen - point_radius
        y0 = y_screen - point_radius
        x1 = x_screen + point_radius
        y1 = y_screen + point_radius

        # if the tag exists, then move the point, else create the point
        point_ids = self.plot.find_withtag(tag)

        if point_ids != ():
            point_id = point_ids[0]

            location = self.plot.coords(point_id)
            current_x = (location[0] + location[2])/2
            current_y = (location[1] + location[3])/2

            move_x = x_screen - current_x
            move_y = y_screen - current_y

            self.plot.move(point_id, move_x, move_y)

        else:
            point = self.plot.create_oval(x0,
                                          y0,
                                          x1,
                                          y1,
                                          outline=fill,
                                          fill=fill,
                                          tag=tag)

    def scatter(self, list_of_points=[], color='#0000ff', tag='current'):
        """ create the new points then delete the old points """
        num_of_points = len(list_of_points)

        for i, point in enumerate(list_of_points):
            new_tag = tag + str(i)
            self.plot_point(point, fill=color, tag=new_tag)

        self.current_points[tag] = list_of_points

    def remove_scatter(self, tag='current'):
        # if erase is True, then we will delete all tags containing the
        # prefix contained in 'tag'.  For instance, if tag == 'current',
        # then we will delete 'current0', 'current1', ...
        del_list = []
        for e in self.plot.find_all():
            for item_tag in self.plot.gettags(e):
                if tag in item_tag and item_tag not in del_list:
                    del_list.append(item_tag)

        for item_tag in del_list:
            self.plot.delete(item_tag)

    def to_screen_coords(self, x, y):
        new_x = x + self.width_px/2
        new_y = self.height_px/2 - y

        return new_x, new_y

if __name__ == "__main__":
    # initialize tk items for display
    root = tk.Tk()
    root.title("for(embed) - Curve Tracer Viewer")
    app = Plot4Q(root)

    points = [(0,0), (10,10), (20,20)]
    app.scatter(points)
    app.scatter()

    root.mainloop()
