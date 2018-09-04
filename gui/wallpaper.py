import os
import ctypes

class Wallpaper:
    def setWallpaper(path):
        if(os.name == "nt"):
            ctypes.windll.user32.SystemParametersInfoA(20, 0, path, 0)
