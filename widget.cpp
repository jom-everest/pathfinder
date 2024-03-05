#include "widget.h"
#include "ui_widget.h"
#include "QDebug"
#include "qevent.h"
#include <QGraphicsSceneMouseEvent>
#include <QTransform>
#include <QGraphicsItem>
#include <QToolTip>

#include "smatrix.h"
#include "pathsmap_matrix_wa.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();
    ui->view->setScene(scene);
    scene->installEventFilter(this);

    resizeWidget();
}

void Widget::resizeEvent(QResizeEvent *event)
{
    resizeWidget();
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != scene)
        return QWidget::eventFilter(watched, event);

    if (sceneMode == SMODE::GENERATE && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent* ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);

        size_t x = ev->scenePos().x()/cellLength;
        size_t y = ev->scenePos().y()/cellLength;

        if (x >= sizeX || y >= sizeY) return false;

        if (pathsMap.data(x, y).totalPathCost == 0) {
            ui->view->setToolTip("Unreacheble");
        }
        else {
            ui->view->setToolTip(QString().sprintf("Path Value: %.1f", pathsMap.data(x, y).totalPathCost));
            scene->removeItem(pathsGraphicsLayer);

            QList<QGraphicsItem*> graphicsItems;
            SMatrix<bool> visitedNodes(sizeX, sizeY, false);
            std::vector<Coord> pathToStart;

            pathToStart.push_back({x,y});
            visitedNodes.data({x,y}) = true;

            for (size_t i = 0; i < pathToStart.size(); ++i) {
                Coord coord = pathToStart[i];
                for (const Coord& nextCoord: pathsMap.data(coord).pathsFrom) {
                    graphicsItems.push_back(scene->addLine(nextCoord.x*cellLength+cellLength/2,
                                   nextCoord.y*cellLength+cellLength/2,
                                   coord.x*cellLength+cellLength/2,
                                   coord.y*cellLength+cellLength/2,
                                   QPen(Qt::magenta)));
                    bool& visitedNode = visitedNodes.data(nextCoord);
                    if (visitedNode) {
                        continue;
                    }
                    visitedNode = true;
                    pathToStart.push_back(nextCoord);
                }
            }
            pathsGraphicsLayer = scene->createItemGroup(graphicsItems);
        }

        return false;
    }
    else if (sceneMode == SMODE::ERASER && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent* ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        int x = ev->scenePos().x()/cellLength;
        int y = ev->scenePos().y()/cellLength;

        sceneErase({x, y});

        return false;
    }
    else if (sceneMode == SMODE::OBST && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent* ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        int x = ev->scenePos().x()/cellLength;
        int y = ev->scenePos().y()/cellLength;

        sceneAddObstacle({x, y});

        return false;
    }
    else if (sceneMode == SMODE::GENERATE && event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent* ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        int x = ev->scenePos().x()/cellLength;
        int y = ev->scenePos().y()/cellLength;

        setStartPoint({x, y});
        setMode(SMODE::GENERATE);

        return false;
    }

    return QWidget::eventFilter(watched, event);
}

void Widget::sceneAddObstacle(Coord c)
{
    if (!validatePoint(c)) return;
    if (startPoint == c) return;

    rectItemMap.data(c)->setBrush(QBrush(Qt::black));
}

void Widget::sceneErase(Coord c)
{
    if (!validatePoint(c)) return;
    if (startPoint == c) return;

    rectItemMap.data(c)->setBrush(QBrush(Qt::white));
}

bool Widget::validatePoint(Coord c)
{
    return (c.x != 0 && c.x < sizeX-1 && c.y != 0 && c.y < sizeY-1);
}

void Widget::setStartPoint(Coord c)
{
    if (validatePoint(c)) {
        if (validatePoint(startPoint)) {
            rectItemMap.data(startPoint)->setBrush(QBrush(Qt::white));
        }
        startPoint = c;
        rectItemMap.data(startPoint)->setBrush(QBrush(Qt::red));
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::resizeWidget()
{
    show();

    sizeX = ui->view->viewport()->width()/cellLength;
    sizeY = ui->view->viewport()->height()/cellLength;

    if (sizeX < 3 || sizeY < 3) return;

    scene->clear();
    pathsGraphicsLayer = nullptr;
    scene->setSceneRect(0, 0, sizeX*cellLength, sizeY*cellLength);
    rectItemMap = SMatrix<QGraphicsRectItem*>(sizeX, sizeY);
    for (size_t x = 0; x < sizeX; ++x) {
        for (size_t y = 0; y < sizeY; ++y) {
            auto rect = rectItemMap.data(x, y) = scene->addRect(x*cellLength, y*cellLength, cellLength, cellLength);
            if (!x || !y || x == sizeX-1 || y == sizeY-1) {
                rect->setBrush(QBrush(Qt::black));
            }
        }
    }
    setStartPoint({1,1});
    setMode(SMODE::GENERATE);
}

void Widget::createPathsMap()
{
    // create a TerrainMap where value greater than 1000 is obstacle and all values
    // in (1, 1000) are terrain patency factor

    // create the TerrainMap based on GraphicsScene objects

    SMatrix<uint32_t> obstaclesMap(sizeX, sizeY);

    for (size_t x = 0; x < sizeX; ++x) {
        for (size_t y = 0; y < sizeY; ++y) {
            auto item = rectItemMap.data(x ,y);
            if (item->brush().style() != Qt::NoBrush && item->brush().color() == Qt::black) {
                obstaclesMap.data(x,y) = 1000;
            }
            else {
                obstaclesMap.data(x, y) = 1;
            }
        }
    }

    pathsMap = createPathsMap_v1(startPoint, obstaclesMap, diagonalMode);
}

void Widget::setMode(SMODE mode)
{
    sceneMode = mode;
    ui->view->setMouseTracking(false);

    if (mode == SMODE::GENERATE) {
        createPathsMap();
        ui->view->setMouseTracking(true);
    }
}

void Widget::on_radioButton_2_clicked()
{
    setMode(SMODE::ERASER);
}

void Widget::on_radioButton_clicked()
{
    setMode(SMODE::OBST);
}

void Widget::on_radioButton_3_clicked()
{
    setMode(SMODE::GENERATE);
}

void Widget::on_radioButton_4_clicked()
{
    diagonalMode = true;
    setMode(SMODE::GENERATE);
}

void Widget::on_radioButton_5_clicked()
{
    diagonalMode = false;
    setMode(SMODE::GENERATE);
}
