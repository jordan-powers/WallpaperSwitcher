import os
import sqlite3
import time
import ctypes
import argparse
import random

wallpapers_folder = "C:\\users\\jordan\\Documents\\static wallpapers"
sleep_time = 120
acceptable_extensions = [".jpg",".JPG",".png",".PNG"]

parser = argparse.ArgumentParser(description="Cycle through all wallpapers in a folder")
parser.add_argument('-s', '--shuffle', action='store_true', dest='shuffle', help="Shuffle photo order")
parser.add_argument('-t', '--time', default=sleep_time, dest='time', help="How many seconds to wait between switching wallpapers")

args = parser.parse_args()

script_dir = os.path.dirname(os.path.realpath(__file__))
print("Opening database at %s" % os.path.join(script_dir,"wallpapers.db"))
conn = sqlite3.connect(os.path.join(script_dir,"wallpapers.db"))

c = conn.cursor()
c.execute("CREATE TABLE IF NOT EXISTS wallpapers (FILE TEXT PRIMARY KEY, LASTTS REAL);")

def updateDB():
    c.execute("SELECT FILE FROM wallpapers;")
    files = c.fetchall()

    wfencoded = os.fsencode(wallpapers_folder)
    for file in os.listdir(wfencoded):
        filename = os.fsdecode(file)
        _, extension = os.path.splitext(filename)
        if extension in acceptable_extensions:
            if (filename,) in files:
                files.remove((filename,))
            c.execute("SELECT * FROM wallpapers WHERE FILE=?;",(filename,))
            res = c.fetchall()
            if len(res) == 0:
                print("Adding %s to db" % filename)
                c.execute("INSERT INTO wallpapers VALUES ( ? , ? );",(filename,time.time()))

    for file in files:
        print("Removing %s from db" % file)
        c.execute("DELETE FROM wallpapers WHERE FILE=?;",file)

    conn.commit();

def shuffle():
    print("Shuffling...")
    c.execute("SELECT FILE FROM wallpapers;")
    files = c.fetchall()
    for file in files:
        randomts = random.randint(0,int(time.time()))
        c.execute("UPDATE wallpapers SET LASTTS=? WHERE FILE=?",(randomts,file[0]))
    conn.commit();

updateDB()

if args.shuffle:
    shuffle()

while True:

    c.execute("SELECT * FROM wallpapers ORDER BY LASTTS ASC LIMIT 1;")
    filename, ts = c.fetchone()
    c.execute("UPDATE wallpapers SET LASTTS=? WHERE FILE=?;",(time.time(),filename));

    conn.commit();
    print("Setting %s as new wallpaper..." % filename)
    ctypes.windll.user32.SystemParametersInfoA(20, 0, os.fsencode(os.path.join(wallpapers_folder,filename)), 0)

    time.sleep(int(args.time))

    updateDB()
