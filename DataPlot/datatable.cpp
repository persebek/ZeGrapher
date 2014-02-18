/****************************************************************************
**  Copyright (c) 2013, Adel Kara Slimane, the ZeGrapher project <contact@zegrapher.com>
**
**  This file is part of the ZeGrapher project, version 2.1.
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

#include "DataPlot/datatable.h"

DataTable::DataTable(Informations *info, int rowCount, int columnCount, int rowHeight, int columnWidth)
{
    informations = info;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    setMinimumSize(0,0);
    calculator = new ExprCalculator(false, informations->getFuncsList());

    QColor color;
    color.setNamedColor(VALID_COLOR);
    validPalette.setColor(QPalette::Base, color);

    color.setNamedColor(INVALID_COLOR);
    invalidPalette.setColor(QPalette::Base, color);

    tableWidget = new QTableWidget(rowCount,columnCount);

    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    tableWidget->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    tableWidget->horizontalHeader()->setMovable(true);
    connect(tableWidget->horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(columnMoved(int,int,int)));

    tableWidget->horizontalHeader()->setFixedHeight(25);    



    resizeColumns(columnWidth);
    resizeRows(rowHeight);

    disableChecking = true;

    for(int col = 0 ; col < columnCount ; col++)
    {
        values << QList<double>();
        for(int row = 0 ; row < rowCount ; row++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(" ");
            tableWidget->setItem(row, col, item);
            values[col] << NAN ;
        }
    }

    disableChecking = false;

    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(checkCell(QTableWidgetItem*)));
    connect(tableWidget->verticalHeader(), SIGNAL(geometriesChanged()), this, SIGNAL(newPosCorrections()));
    connect(tableWidget->horizontalHeader(), SIGNAL(geometriesChanged()), this, SIGNAL(newPosCorrections()));
    connect(tableWidget->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(renameColumn(int)));

    mainLayout->addWidget(tableWidget);
    mainLayout->addStretch();
    setLayout(mainLayout);

    for(int i = 0 ; i < columnCount ; i++) { columnNames << tr("Renommez moi!") ; }
    tableWidget->setHorizontalHeaderLabels(columnNames);

    nameValidator.setPattern("^([a-z]|[A-Z])([a-z]|[A-Z]|_)+([a-z]|[A-Z])$");

}

void DataTable::fillColumnFromRange(int col, Range range)
{
    double val = range.start;
    int row = 0;
    QTableWidgetItem *item;
    while(val <= range.end)
    {
        values[col][row] = val;

        item = tableWidget->item(row, col);
        item->setText(QString::number(val, 'g', MAX_DOUBLE_PREC));
        item->setBackgroundColor(VALID_COLOR);

        val += range.step;
        row++;

        if(tableWidget->rowCount() == row+1)
            addRow();
    }
}

void DataTable::addRow()
{
    insertRow(tableWidget->rowCount());
}

void DataTable::addColumn()
{
    insertColumn(tableWidget->columnCount());
}

void DataTable::columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    Q_UNUSED(logicalIndex);
    values.move(oldVisualIndex, newVisualIndex);
    columnNames.move(oldVisualIndex, newVisualIndex);
}

void DataTable::checkCell(QTableWidgetItem *item)
{
    if(disableChecking)
        return;

    if(item->column()+1 == tableWidget->columnCount())
        addColumn();
    if(item->row()+1 == tableWidget->rowCount())
        addRow();

    QString expr = item->text();

    if(expr.isEmpty())
    {
        item->setBackgroundColor(Qt::white);
        return;
    }

    bool ok = false;
    double val = calculator->calculateExpression(expr, ok);

    if(ok)
    {
        item->setBackgroundColor(VALID_COLOR);
        values[item->column()][item->row()] = val;
        item->setText(QString::number(val, 'g', MAX_DOUBLE_PREC));
    }
    else
    {
        item->setBackgroundColor(INVALID_COLOR);
        values[item->column()][item->row()] = NAN;
    }
}

void DataTable::insertRow(int index)
{
    tableWidget->insertRow(index);
    tableWidget->setRowHeight(index, cellHeight);

    disableChecking = true;

    for(int col = 0 ; col < values.size() ; col++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(" ");
        tableWidget->setItem(index, col, item);
        values[col].insert(index, NAN);
    }

    disableChecking = false;

    emit newRowCount(tableWidget->rowCount());
}

void DataTable::insertColumn(int index)
{
    tableWidget->insertColumn(index);
    columnNames.insert(index, tr("Renommez moi!"));
    tableWidget->setHorizontalHeaderLabels(columnNames);

    tableWidget->setColumnWidth(index, cellWidth);
    int rowCount = values[0].size();
    values.insert(index, QList<double>());

    disableChecking = true;

    for(int row = 0 ; row < rowCount ; row++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(" ");
        tableWidget->setItem(row, index, item);
        values[index] << NAN ;
    }

    disableChecking = false;

    tableWidget->setFixedWidth(tableWidget->columnCount() * cellWidth + tableWidget->verticalHeader()->width() + 10);

    emit newColumnNames(columnNames);
    emit newColumnCount(tableWidget->columnCount());
}

void DataTable::removeRow(int index)
{
    tableWidget->removeRow(index);

    for(int col = 0 ; col < values.size() ; col++) { values[col].removeAt(index); }

    emit newRowCount(tableWidget->rowCount());
}

void DataTable::removeColumn(int index)
{
    tableWidget->removeColumn(index);
    columnNames.removeAt(index);

    values.removeAt(index);

    tableWidget->setFixedWidth(tableWidget->columnCount() * cellWidth + tableWidget->verticalHeader()->width() + 10);

    emit newColumnNames(columnNames);
    emit newRowCount(tableWidget->rowCount());

}

void DataTable::renameColumn(int index)
{
    bool ok = true;
    QString name = QInputDialog::getText(this, tr("Nouveau nom de colonne"), tr("Veuillez entrer un nom pour cette colonne :"), QLineEdit::Normal, "", &ok);

    if(!ok)
        return;
    if(!nameValidator.exactMatch(name))
    {
        QMessageBox::information(this, tr("Erreur"), tr("Les noms ne peuvent contenir que des lettres et \"_\" "));
        return;
    }

    columnNames[index] = name;
    tableWidget->horizontalHeaderItem(index)->setText(name);

    emit newColumnNames(columnNames);

}

void DataTable::resizeColumns(int columnWidth)
{
    cellWidth = columnWidth;

    for(int i = 0 ; i < tableWidget->columnCount(); i++)
    {
        tableWidget->setColumnWidth(i, columnWidth);
    }

    tableWidget->setFixedWidth(tableWidget->columnCount() * cellWidth + tableWidget->verticalHeader()->width() + 10);
}

void DataTable::resizeRows(int rowHeight)
{
    cellHeight = rowHeight;

    for(int i = 0 ; i < tableWidget->rowCount(); i++)
    {
        tableWidget->setRowHeight(i, rowHeight);
    }    
}

QSize DataTable::getVerticalHeaderSize()
{
    return tableWidget->verticalHeader()->size();
}

QSize DataTable::getHorizontalHeaderSize()
{
    return tableWidget->horizontalHeader()->size();
}

int DataTable::getColumnCount()
{
    return tableWidget->columnCount();
}

int DataTable::getRowCount()
{
    return tableWidget->rowCount();
}