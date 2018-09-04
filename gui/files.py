import os
class Files:
    acceptable_extensions = [".jpg",".png"]
    path = None
    def __init__(self, path):
        self.path = os.fsencode(path)

    def get_files(self):
        return [os.fsdecode(file) for file in os.listdir(self.path) if os.path.splitext(os.fsdecode(file))[1].lower() in self.acceptable_extensions]

    def fileToPath(self, file):
        return os.fsencode(os.path.join(self.path,file))
