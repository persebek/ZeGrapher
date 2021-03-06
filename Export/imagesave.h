/****************************************************************************
**  Copyright (c) 2016, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeGrapher's source code.
**
**  ZeGrapher is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU General Public License as published by the
**  Free Software Foundation, either version 3 of the License, or (at your
**  option) any later version.
**
**  This file is distributed in the hope that it will be useful, but
**  WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/




#ifndef IMAGESAVE_H
#define IMAGESAVE_H

#include <QWidget>
#include "structures.h"
#include "GraphDraw/imagepreview.h"

namespace Ui {
    class ImageSave;
}

class ImageSave : public QWidget
{
    Q_OBJECT

public:
    explicit ImageSave(Information *info);
    ~ImageSave();   

public slots:
    void setSize(int W, int H);
    void setWindow(ZeGraphView win);
    void setPrecision(short prec);   
    void setW(int W);
    void setH(int H);
    void save();    

private:
    Ui::ImageSave *ui;
    QWidget *previewWindow;    

    int largeur, hauteur;
    ZeGraphView window;
    short precision;
    ImagePreview *scene;   
    QFileDialog fileDialog;
};

#endif // IMAGESAVE_H
