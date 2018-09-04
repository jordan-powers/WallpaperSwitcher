from files import Files
from database import WallpaperDatabase
from wallpaper import Wallpaper as wp

wallpapers_folder = "C:\\users\\jordan\\Documents\\static wallpapers"

files = Files(wallpapers_folder)
db = WallpaperDatabase()
db.checkfiles(files.get_files())
db.save()
