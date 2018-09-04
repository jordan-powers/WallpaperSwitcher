import os
import sqlite3

class WallpaperDatabase:
    dbpath = os.path.join(os.path.dirname(os.path.realpath(__file__)),"wallpapers.db")
    conn = None
    c = None

    def __init__(self):
        self.connect()
        self.maketable()

    def connect(self):
        self.conn = sqlite3.connect(self.dbpath)
        self.c = self.conn.cursor()

    def maketable(self):
        self.c.execute("CREATE TABLE IF NOT EXISTS wallpapers (FILE TEXT PRIMARY KEY, IDX INTEGER NOT NULL UNIQUE)")

    def checkfiles(self, files):
        assert type(files) is list, "files must be a list of files to check"
        self.c.execute("SELECT FILE FROM wallpapers")
        results = self.c.fetchall()
        toremove = [file[0] for file in results if file[0] not in files]
        self.removefiles(toremove)
        toadd = [file for file in files if (file,) not in results]
        self.addfiles(toadd)

    def removefiles(self, files):
        assert type(files) is list, "files must be a list of files to remove"
        for file in files:
            print(file)
            self.c.execute("SELECT IDX FROM wallpapers WHERE FILE = ?", (file,))
            idx = self.c.fetchone()[0]
            print("Removing %s at %d" % (file, idx))
            self.c.execute("DELETE FROM wallpapers WHERE FILE = ?", (file,));
            self.c.execute("UPDATE wallpapers SET IDX = IDX - 1 WHERE IDX > ?", (idx,));

    def addfiles(self, files):
        assert type(files) is list, "files must be a list of files to add"
        toadd = []
        self.c.execute("SELECT IDX FROM wallpapers ORDER BY IDX DESC LIMIT 1")
        res = self.c.fetchone()
        idx = res[0]+1 if res is not None else 1
        for file in files:
            toadd.append((file,idx))
            print("Adding %s at %d" % (file,idx))
            idx += 1
        self.c.executemany("INSERT INTO wallpapers VALUES (?,?)",toadd)

    def addfile(self, file, idx):
        assert type(file) is string and type(idx) is int, "file must be string and idx must be int"
        self.c.execute("SELECT IDX FROM wallpapers ORDER BY IDX DESC LIMIT 1")
        dbidx = self.c.fetchone()[0]
        assert idx <= dbidx, "Can't insert after highest idx"
        self.c.execute("UPDATE wallpapers SET IDX = IDX + 1 WHERE IDX > ?", idx);
        self.c.execute("INSERT INTO wallpapers VALUES (?,?)",(file,idx));

    def setIdx(self, file, newidx):
        removefiles([file])
        addfile(file,idx)

    def save(self):
        self.conn.commit()

    def getFiles(self):
        self.c.execute("SELECT FILE, IDX FROM wallpapers ORDER BY IDX ASC");
        return self.c.fetchall();
