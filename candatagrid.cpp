#include "candatagrid.h"
#include "ui_candatagrid.h"

#include <QPainter>
#include <QDebug>

CANDataGrid::CANDataGrid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CANDataGrid)
{
    ui->setupUi(this);

    memset(data, 0, 8);
    memset(refData, 0, 8);
}

CANDataGrid::~CANDataGrid()
{
    delete ui;
}

void CANDataGrid::paintEvent(QPaintEvent *event)
{
    int x, y;
    unsigned char prevByte, thisByte;
    bool thisBit, prevBit;
    QBrush blackBrush, whiteBrush, redBrush, greenBrush;
    QPainter painter(this);

    QRect viewport = painter.viewport();

    int xSpan = viewport.right() - viewport.left();
    int ySpan = viewport.bottom() - viewport.top();

    int xSector = xSpan / 10;
    int ySector = ySpan / 10;

    blackBrush = QBrush(Qt::black);
    whiteBrush = QBrush(Qt::white);
    redBrush = QBrush(Qt::red);
    greenBrush = QBrush(Qt::green);

    //the whole thing is broken up into 100 chunks which are allocated as such:
    //The entirety of the upper row is taken up by "BITS"
    //The next row down skips two columns and then has 7 6 5 4 3 2 1 0
    //The bottom right 64 entries are the grid
    //The left most column has "BYTES" written vertically down it
    //the next left most column has "0 1 2 3 4 5 6 7" written down along the grid

    painter.setPen(QPen(Qt::black));
    QFont theFont;
    theFont.setPixelSize(qMin(xSector, ySector) - 10);
    painter.setFont(theFont);

    painter.drawText(QRect(viewport.left(), viewport.top(), xSpan, ySector), Qt::AlignCenter, tr("BITS"));

    for (x = 0; x < 8; x++)
    {
        painter.drawText(QRect(viewport.left() + (x+2) * xSector, viewport.top() + ySector, xSector, ySector), Qt::AlignCenter, QString::number(7-x));
    }
    for (y = 0; y < 8; y++)
    {
        painter.drawText(QRect(viewport.left() + xSector, viewport.top() + ySector * (y + 2), xSector, ySector), Qt::AlignCenter, QString::number(y));
    }

    painter.drawText(viewport.left() + 10, viewport.top() + ySector * 4, "B");
    painter.drawText(viewport.left() + 10, viewport.top() + ySector * 5, "Y");
    painter.drawText(viewport.left() + 10, viewport.top() + ySector * 6, "T");
    painter.drawText(viewport.left() + 10, viewport.top() + ySector * 7, "E");
    painter.drawText(viewport.left() + 10, viewport.top() + ySector * 8, "S");

    //now, color the bitfield by seeing if a given bit is freshly set/unset in the new data
    //compared to the old. Bits that are not set in either are white, bits set in both are black
    //bits that used to be set but now are unset are red, bits that used to be unset but now are set
    //are green

    for (y = 0; y < 8; y++)
    {
        thisByte = data[y];
        prevByte = refData[y];
        for (x = 0; x < 8; x++)
        {
            thisBit = false;
            prevBit = false;
            if ((thisByte & (1 << x)) == (1 << x)) thisBit = true;
            if ((prevByte & (1 << x)) == (1 << x)) prevBit = true;

            if (thisBit)
            {
                if (prevBit)
                {
                    painter.setBrush(blackBrush);
                }
                else
                {
                    painter.setBrush(greenBrush);
                }
            }
            else
            {
                if (prevBit)
                {
                    painter.setBrush(redBrush);
                }
                else
                {
                    painter.setBrush(whiteBrush);
                }
            }

            //painter.fillRect(viewport.left() + (x+2) * xSector, viewport.top() + (y+2) * ySector, xSector, ySector, redBrush);
            painter.drawRect(viewport.left() + (x+2) * xSector, viewport.top() + (y+2) * ySector, xSector, ySector);
        }
    }             
}

void CANDataGrid::setReference(unsigned char *newRef, bool bUpdate = true)
{
    memcpy(refData, newRef, 8);
    if (bUpdate) this->update();
}

void CANDataGrid::updateData(unsigned char *newData, bool bUpdate = true)
{
    memcpy(data, newData, 8); //on a 64 bit processor this is probably optimized to a single instruction
    if (bUpdate) this->update();
}
