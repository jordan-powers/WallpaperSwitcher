import os
import sqlite3
import time
import ctypes
import argparse
import random
from pathlib import Path
from itertools import chain

DEFAULT_WALLPAPER_FOLDER = "C:\\Users\\jordan\\Documents\\wallpapers"
DEFAULT_SLEEP_TIME = 120
ALLOWED_EXTENSIONS = [".jpg",".png"]

parser = argparse.ArgumentParser(description="Cycle through all wallpapers in a folder")
parser.add_argument('-s', '--shuffle', action='store_true', dest='shuffle', help="Shuffle photo order")
parser.add_argument('-t', '--time', default=DEFAULT_SLEEP_TIME, dest='time', help="How many seconds to wait between switching wallpapers")
parser.add_argument('-d', '--directory', default=DEFAULT_WALLPAPER_FOLDER, dest='dir', help="The directory containing the wallpapers to display")

args = parser.parse_args()

wallpaper_dir = Path(args.dir)

wallpaper_db = wallpaper_dir / "wallpapers.db"

print("Opening database at {}".format(wallpaper_db))
conn = sqlite3.connect(wallpaper_db)

c = conn.cursor()
c.execute("CREATE TABLE IF NOT EXISTS wallpapers (FILE TEXT PRIMARY KEY, LASTTS REAL);")


def updateDB():
    """
    Synchronizes the database with the files on-disk
    Every wallpaper will be in one of four states:
      1. On-disk, but not tracked in the database (a recently added wallpaper)
      2. On-disk and tracked in the database (most wallpapers)
      3. Not on-disk, but tracked in the database (a recently deleted wallpaper)
      4. Not on-disk and not tracked in the database (a wallpaper that does not exist)
    
    This function iterates through all wallpapers in the wallpapers directory
    and updates the state of each wallpaper to be in state 2 or 4.
    If a wallpaper is in state 1, it is brought to state 2 (added to the database).
    If a wallpaper is in state 3, it is brought to state 4 (deleted from the database).
    """
    # tracked_files is the current state of the database
    c.execute("SELECT FILE FROM wallpapers;")
    tracked_files = [item[0] for item in c.fetchall()]

    for f in sorted(chain(*[wallpaper_dir.glob("**/*{}".format(ext)) for ext in ALLOWED_EXTENSIONS])):
        if f.suffix in ALLOWED_EXTENSIONS:
            if str(f) in tracked_files:
                # state 2: the file is on disk, and the file is tracked
                tracked_files.remove(str(f))
            else:
                # state 1: the file is on disk, but is not yet tracked
                print("Adding", str(f))
                c.execute("INSERT INTO wallpapers VALUES ( ? , ? );",(str(f),time.time()))
        else:
            print("Somehow globbed invalid extension: {}".format(f))

    for f in tracked_files:
        # state 3: the file is not on disk, but is tracked
        print("Removing", f)
        c.execute("DELETE FROM wallpapers WHERE FILE=?;", (str(f),))

    conn.commit()

def shuffle():
    print("Shuffling...")
    c.execute("SELECT FILE FROM wallpapers;")
    files = c.fetchall()
    for file in files:
        randomts = random.randint(0,int(time.time()))
        c.execute("UPDATE wallpapers SET LASTTS=? WHERE FILE=?",(randomts,file[0]))
    conn.commit()

if args.shuffle:
    updateDB()
    shuffle()

while True:
    updateDB()
    c.execute("SELECT * FROM wallpapers ORDER BY LASTTS ASC LIMIT 1;")
    filename, ts = c.fetchone()
    c.execute("UPDATE wallpapers SET LASTTS=? WHERE FILE=?;",(time.time(),filename))

    conn.commit()
    filepath = Path(filename)
    print("Setting {} as new wallpaper...".format(filepath.name))
    ctypes.windll.user32.SystemParametersInfoA(20, 0, bytes(filepath), 0)

    time.sleep(int(args.time))
