import sys
import itertools as itt
from   tkinter     import *
from   tkinter.ttk import *
import numpy as np


class DigitField:
    MAX_CELL_COUNT: tuple[int,int] = (59,24)
    TILE_WIDTH:  int = 24
    TILE_HEIGHT: int = 34
    DEF_COLOR:  str = "#000"
    SEL_COLOR:  str = "#0C0"
    HINT_COLOR: str = "#C40"
    BG_DEF_COLOR:  str = "#FFF"
    BG_SEL_COLOR:  str = "#BFB"
    BG_HINT_COLOR: str = "#FC9"

    def __init__(self, _width: int, _height: int):
        if _width > DigitField.MAX_CELL_COUNT[0] or _height > DigitField.MAX_CELL_COUNT[1]:
            _width  = min(_width,  DigitField.MAX_CELL_COUNT[0])
            _height = min(_height, DigitField.MAX_CELL_COUNT[1])
            print(f"Field size exceeds the limit. Final size: ({_width}, {_height})")
        self.width = _width
        self.height = _height
        self.arr = np.array(np.random.random((_width, _height)) * 9 + 1, dtype=np.uint8)
        self.cell_to_id: dict[tuple[int, int], list[int]] = {}
        self.show_hint = False

        self.BEGIN_CELL:        tuple[int] = (-1,-1)
        self.CURRENT_CELL:      tuple[int] = (-1,-1)
        self.BEGIN_HINT_CELL:   tuple[int] = (-1,-1)
        self.CURRENT_HINT_CELL: tuple[int] = (-1,-1)

        self.root = Tk()
        self.root.title("Decimary Game")
        self.canv = Canvas(self.root, width=DigitField.TILE_WIDTH * (_width*1.2), height=DigitField.TILE_HEIGHT * (_height*1.1), offset="5,5")
        self.canv.grid()

        for cell in itt.product(range(self.arr.shape[0]), range(self.arr.shape[1])):
            bg_id = self.canv.create_rectangle( cell[0]   *DigitField.TILE_WIDTH+7,  cell[1]   *DigitField.TILE_HEIGHT+1.6,
                                               (cell[0]+1)*DigitField.TILE_WIDTH+7, (cell[1]+1)*DigitField.TILE_HEIGHT+1.6, fill= "#FFF", outline= "#000", outlineoffset="-10,-10")
            item_id = self.canv.create_text(cell[0]*DigitField.TILE_WIDTH +20,
                                            cell[1]*DigitField.TILE_HEIGHT+20, text= str(self.arr[cell]), font=("TkDefaultFont", 16), tags=cell)
            self.cell_to_id[cell] = [item_id, bg_id]
        self.canv.bind("<B1-Motion>", self)
        self.canv.bind("<ButtonPress>", self)
        self.canv.bind("<ButtonRelease>", self)
        self.root.bind("<KeyPress>", self)

    def idToCell(self, _item_id: int) -> tuple[int, int]:
        tag = self.canv.itemconfigure(_item_id)["tags"][-1]
        if len(tag) > 0 and ord('0') <= ord(tag[0]) <= ord('9'):
            return tuple( int(s)  for s in self.canv.itemconfigure(_item_id)["tags"][-1].split(' ')[:2] )
        return None

    def getRangeEndStep(self, _prev_cell: tuple[int, int], _curr_cell: tuple[int, int]) -> tuple[tuple[int, int], tuple[int, int]]:
        range_step = [ int(np.sign(_curr_cell[0] - _prev_cell[0])),
                       int(np.sign(_curr_cell[1] - _prev_cell[1])) ]
        if range_step[0] == 0: range_step[0] = 1
        if range_step[1] == 0: range_step[1] = 1

        range_end = ( _curr_cell[0] + range_step[0],
                      _curr_cell[1] + range_step[1])
        return range_end, tuple(range_step)

    def updateCellIds(self, _curr_cell: tuple[int, int]) -> bool:
        __res = False
        if _curr_cell == (-1,-1):
            if self.CURRENT_CELL == (-1,-1): return __res

            range_end, range_step = self.getRangeEndStep(self.BEGIN_CELL, self.CURRENT_CELL)

            bg_config   = { "fill" : DigitField.BG_DEF_COLOR }
            cell_config = { "fill" : DigitField.   DEF_COLOR }
            if np.sum(self.arr[self.BEGIN_CELL[0] : (range_end[0] if range_end[0] >= 0 else None) : range_step[0],
                               self.BEGIN_CELL[1] : (range_end[1] if range_end[1] >= 0 else None) : range_step[1]]) == 10:
                __res = True
                cell_config = { "text" : ' ' }
                self.arr[self.BEGIN_CELL[0] : (range_end[0] if range_end[0] >= 0 else None) : range_step[0],
                         self.BEGIN_CELL[1] : (range_end[1] if range_end[1] >= 0 else None) : range_step[1]] = 0

            for cell in itt.product(range(self.BEGIN_CELL[0], range_end[0], range_step[0]),
                                    range(self.BEGIN_CELL[1], range_end[1], range_step[1])):
                self.canv.itemconfig(self.cell_to_id[cell][0], **cell_config)
                self.canv.itemconfig(self.cell_to_id[cell][1],   **bg_config)
            self.show_hint = False
            self.BEGIN_CELL = (-1,-1)
            self.CURRENT_CELL = (-1,-1)
            self.BEGIN_HINT_CELL = (-1,-1)
            self.CURRENT_HINT_CELL = (-1,-1)
        else:
            if self.BEGIN_CELL == (-1,-1):
                self.BEGIN_CELL = _curr_cell
            self.CURRENT_CELL = _curr_cell
        return __res

    def showHint(self):
        self.show_hint = not self.show_hint

    def updateHint(self, _begin_hint_cell: tuple[int, int], _curr_hint_cell: tuple[int, int]):
        self.BEGIN_HINT_CELL = _begin_hint_cell
        self.CURRENT_HINT_CELL = _curr_hint_cell

    def updateColors(self):
        c_beg = ( min(self.BEGIN_CELL[0], self.CURRENT_CELL[0]), min(self.BEGIN_CELL[1], self.CURRENT_CELL[1]) )
        c_end = ( max(self.BEGIN_CELL[0], self.CURRENT_CELL[0]), max(self.BEGIN_CELL[1], self.CURRENT_CELL[1]) )

        for cell in itt.product(range(0, self.arr.shape[0]),
                                range(0, self.arr.shape[1])):
            if self.arr[cell] == 0: continue

            bg_color   = DigitField.BG_DEF_COLOR
            fill_color = DigitField.   DEF_COLOR
            if self.show_hint and self.BEGIN_HINT_CELL[0] <= cell[0] < self.CURRENT_HINT_CELL[0] and\
                                  self.BEGIN_HINT_CELL[1] <= cell[1] < self.CURRENT_HINT_CELL[1]:
                bg_color   = DigitField.BG_HINT_COLOR
                fill_color = DigitField.   HINT_COLOR
            if c_beg[0] <= cell[0] <= c_end[0] and\
               c_beg[1] <= cell[1] <= c_end[1]:
                bg_color   = DigitField.BG_SEL_COLOR
                fill_color = DigitField.   SEL_COLOR
            self.canv.itemconfig(self.cell_to_id[cell][0], fill= fill_color)
            self.canv.itemconfig(self.cell_to_id[cell][1], fill=   bg_color)

    def checkCombinations(self) -> bool:
        comb_exists = False
        for cell in itt.product(range(0, self.arr.shape[0]),
                                range(0, self.arr.shape[1])):
            for rad in range(1, max(self.arr.shape[0] - cell[0],
                                    self.arr.shape[1] - cell[1])):
                all_sums_ten_or_greater = True
                max_ncell = ( min(self.arr.shape[0], cell[0] + rad + 1), min(self.arr.shape[1], cell[1] + rad + 1) )
                for ncell in itt.chain(itt.product(range(cell[0], max_ncell[0]), [max_ncell[1],]),
                                       itt.product([max_ncell[0]], range(cell[1], max_ncell[1]))):
                    nsum = np.sum(self.arr[cell[0]:ncell[0], cell[1]:ncell[1]], dtype=np.uint16)
                    if nsum <  10:
                        all_sums_ten_or_greater = False
                    if nsum == 10:
                        self.updateHint(cell, ncell)
                        comb_exists = True
                        return comb_exists
                if all_sums_ten_or_greater: break
        return comb_exists

    def replaceDigits(self):
        new_arr = np.array(np.random.random((self.width, self.height)) * 9 + 1, dtype=np.uint8)
        self.arr[self.arr.nonzero()] = new_arr[self.arr.nonzero()]

        for cell in itt.product(range(0, self.arr.shape[0]),
                                range(0, self.arr.shape[1])):
            if self.arr[cell] == 0: continue
            self.canv.itemconfig(self.cell_to_id[cell][0], text=str(self.arr[cell]))

    def getScore(self):
        return self.width * self.height - np.count_nonzero(self.arr)

    def __call__(self, event: Event):
        update_colors = True
        item_id = self.canv.find_closest(event.x, event.y)

        cell = None
        if event.type is EventType.ButtonRelease:
            cell = (-1,-1)
        elif event.type is EventType.KeyPress:
            if event.keysym == 'g':
                self.replaceDigits()
            elif event.keysym == 'h':
                self.checkCombinations()
                self.showHint()
            else: update_colors = False
        elif len(item_id) > 0: # <B1-Motion> or ButtonPress
            cell = self.idToCell(item_id[0])
        else: update_colors = False

        if cell is not None:
            if self.updateCellIds(cell):
                if self.checkCombinations():
                    print("Current score:", self.getScore())
                elif (rest := np.count_nonzero(self.arr)) < 2:
                    print(("The game is over" if rest == 1 else "Congratulations") + "! Final score:", self.getScore())
                    self.root.destroy()
                    update_colors = False
                else:
                    print("No combinations, press 'g' to generate new numbers")
        if update_colors: self.updateColors()



if __name__ == "__main__":
    width  = 10
    height = 16
    if len(sys.argv) == 3:
        width  = int(sys.argv[1])
        height = int(sys.argv[2])

    field = DigitField(width, height)
    field.root.mainloop()