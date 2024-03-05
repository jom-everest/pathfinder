#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsScene>

#include "pathsmap_matrix_wa.h"

namespace Ui {
class Widget;
}

enum class SMODE {OBST, ERASER, GENERATE};

class Widget : public QWidget
{
    Q_OBJECT


    const int cellLength = 15;

public:
    bool eventFilter(QObject *watched, QEvent *event) override;
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void resizeWidget();
    void on_radioButton_2_clicked();
    void on_radioButton_clicked();
    void on_radioButton_3_clicked();
    void on_radioButton_4_clicked();
    void on_radioButton_5_clicked();

private:
    Ui::Widget *ui;
    QGraphicsScene *scene;
    Coord startPoint {1, 1};
    size_t sizeX, sizeY;
    SMatrix<QGraphicsRectItem*> rectItemMap;
    QGraphicsItemGroup* pathsGraphicsLayer {nullptr};
    SMatrix<Node> pathsMap;
    SMODE sceneMode {SMODE::GENERATE};
    bool diagonalMode {0};

    bool validatePoint(Coord c);
    void setStartPoint(Coord c);
    void sceneErase(Coord c);
    void sceneAddObstacle(Coord c);
    void setMode(SMODE mode);
    void createPathsMap();

    void resizeEvent(QResizeEvent *event);

};

#endif // WIDGET_H
