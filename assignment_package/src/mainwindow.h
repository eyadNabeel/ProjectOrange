#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidget>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();

    void on_actionLoad_OBJ_triggered();

    void on_actionLoad_Skeleton_triggered();

    void on_actionCamera_Controls_triggered();

    // loads all the mesh data into the QListWidgets
    void loadListWidget();

    // sets the selected value in ui->mygl to the currently
    // selected component and causes it to render
    void selectVertex(QListWidgetItem *selected);
    void selectHalfEdge(QListWidgetItem *selected);
    void selectFace(QListWidgetItem *selected);

    // changes the selected vertex's x, y, or z position
    // respectively, then refreshes the mesh
    void setVertXPos(double x);
    void setVertYPos(double y);
    void setVertZPos(double z);

    // changes the selected face's r, g, or b value
    // respectively, then refreshes the mesh
    void setFaceRed(double r);
    void setFaceGreen(double g);
    void setFaceBlue(double b);

    // causes ui->mygl to add a vertex at the midpoint
    // betweeen the two endpoints of the selected edge
    void addVertex();

    // causes ui->mygl to triangulate the selected face
    // given it has more than 3 vertices
    void triangulate();

    // Calls the Catmull-Clark subdivision function on the mesh
    void subdivide();

    // Selects the joint causing its display color to change
    void selectJoint(QTreeWidgetItem *current);

    // Functions to rotate the selected joint about the mentioned axes
    void rotateRight();
    void rotateLeft();
    void rotateUp();
    void rotateDown();
    void rotateCW();
    void rotateCCW();

    // Set the influence of a joint on the selected vertex to val
    void setInf0(double val);
    void setInf1(double val);

    // Bind/Unbind the mesh
    void bindMesh();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
