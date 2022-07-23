#!/usr/bin/env python3

import sys
from PySide import QtGui
import gfxprim.core as core
import gfxprim.loaders as loaders

def getpixmap(path):
    img = loaders.load(path)
    if img.pixel_type != core.C.PIXEL_BGR888:
        img = img.convert(core.C.PIXEL_BGR888)
    qt_img = QtGui.QImage(img.to_byte_array(), img.w, img.h,
                          img.bytes_per_row, QtGui.QImage.Format_RGB888)
    pix = QtGui.QPixmap.fromImage(qt_img)
    return pix

class ImageLabel(QtGui.QLabel):
    def __init__(self, path, parent=None):
        QtGui.QLabel.__init__(self, parent)

        self.setWindowTitle(path)

        self.pix = getpixmap(path)
        self.setPixmap(self.pix)
        size = self.pix.size()
        self.setGeometry(100, 100, size.width(), size.height())

def main():
    app = QtGui.QApplication(sys.argv)
    w = ImageLabel(sys.argv[1])
    w.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
