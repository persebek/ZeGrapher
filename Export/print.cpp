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




#include "Export/print.h"
#include "ui_print.h"

Print::Print(Information *info) : ui(new Ui::Print)
{
    ui->setupUi(this);

    setWindowTitle(tr("Print"));

    scene = new PrintPreview(info);

    orthonormal = false;

    scene->setMinimumSize(210,297);

    ui->scrollArea->setWidget(scene);


    timer.setInterval(4000);
    timer.setSingleShot(true);   

    printersList = QPrinterInfo::availablePrinters();

    for(short i = 0; i < printersList.size(); i++)
        ui->printer->insertItem(i, printersList[i].printerName());

    connect(ui->view, SIGNAL(currentIndexChanged(int)), this, SLOT(viewChanged(int)));  
    connect(ui->height, SIGNAL(valueChanged(double)), scene, SLOT(setGraphHeight(double)));
    connect(ui->width, SIGNAL(valueChanged(double)), scene, SLOT(setGraphWidth(double)));   
    connect(scene, SIGNAL(newGraphSize(double,double)), this, SLOT(setGraphSize(double,double)));
    connect(ui->print, SIGNAL(released()), this, SLOT(print())); 
    connect(&timer, SIGNAL(timeout()), this, SLOT(enablePrintButton()));
    connect(ui->chooseLocation, SIGNAL(released()), this, SLOT(getPDFname()));
    connect(ui->inPDF, SIGNAL(toggled(bool)), this, SLOT(setPrintButtonText(bool)));

    connect(ui->legendBox, SIGNAL(toggled(bool)), scene, SLOT(setLegendState(bool)));
    connect(ui->legendSize, SIGNAL(valueChanged(int)), scene, SLOT(setlegendFontSize(int)));
    connect(ui->xLegend, SIGNAL(textChanged(QString)), scene, SLOT(setXaxisLegend(QString)));
    connect(ui->yLegend, SIGNAL(textChanged(QString)), scene, SLOT(setYaxisLegend(QString)));
    connect(ui->bold, SIGNAL(toggled(bool)), scene, SLOT(setBold(bool)));
    connect(ui->italic, SIGNAL(toggled(bool)), scene, SLOT(setItalic(bool)));
    connect(ui->underline, SIGNAL(toggled(bool)), scene, SLOT(setUnderline(bool)));
    connect(ui->numPrec, SIGNAL(valueChanged(int)), scene, SLOT(setNumPrec(int)));

}

void Print::getPDFname()
{
    PDFname = QFileDialog::getSaveFileName(this, tr("Create PDF"), QString(), "PDF (*.pdf);;PostScript (*.ps)");
    if(!PDFname.isEmpty())
    {
        if(!PDFname.endsWith(".ps") && !PDFname.endsWith(".pdf"))
            PDFname.append(".pdf");
        ui->PDFname->setText(PDFname);
        scene->setPDFname(PDFname);
    }
}

void Print::setPrintButtonText(bool inFileRadioButtonState)
{
    if(inFileRadioButtonState)
        ui->print->setText(tr("Save"));
    else ui->print->setText(tr("Print"));
}

void Print::enablePrintButton()
{
    ui->print->setEnabled(true);
}

void Print::print()
{
    if(printersList.isEmpty() && ui->inPaper->isChecked())
        return;

    if(ui->inPaper->isChecked())
    {
        int reponse = QMessageBox::question(this, tr("Warning"), tr("You are about to print, do you want to continue?\n Please check settings before printing."), QMessageBox::Yes | QMessageBox::No);

        if (reponse == QMessageBox::No)
            return;
    }

    ui->print->setEnabled(false);
    timer.start();

    if(!printersList.isEmpty())
        scene->setPrinter(printersList[ui->printer->currentIndex()]);

    bool resType;
    if(ui->screenRes->isChecked())
        resType = SCREEN_RESOLUTION;
    else resType = CUSTOM_RESOLUTION;

    bool colorType;
    if(ui->colorType->currentIndex() == 0)
        colorType = COLOR;
    else colorType = GRAYSCALE;

    bool printType;
    if(ui->inPaper->isChecked())
        printType = PRINT_SHEET;
    else
    {
        if(ui->PDFname->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please select a directory to create the PDF file in."));
            return;
        }
        printType = PRINT_FILE;
    }


    scene->print(ui->numPages->value(), colorType, printType, resType, ui->resolution->value());
}

void Print::setGraphSize(double H, double W)
{
    ui->height->setValue(H);
    ui->width->setValue(W);
}

void Print::viewChanged(int pos)
{
    if(pos == 0)
    {
        ui->width->setMaximum(20);
        ui->height->setMaximum(28.7);
        scene->setViewType(PORTRAIT);
        scene->setMinimumSize(210,297);
    }
    else
    {
        ui->width->setMaximum(28.7);
        ui->height->setMaximum(20);
        scene->setViewType(PAYSAGE);
        scene->setMinimumSize(297,210);
    }
}



Print::~Print()
{
    delete ui;   
}
