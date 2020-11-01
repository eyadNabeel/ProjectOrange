#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <mesh.h>
#include <vertexdisplay.h>
#include <facedisplay.h>
#include <halfedgedisplay.h>
#include <joint.h>
#include <iostream>
#include <QFileDialog>
#include <fstream>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    connect(ui->mygl, SIGNAL(ctxInitialized()), this, SLOT(loadListWidget()));
    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectVertex(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectFace(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectHalfEdge(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(vertSelected(QListWidgetItem*)), this, SLOT(selectVertex(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(faceSelected(QListWidgetItem*)), this, SLOT(selectFace(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(halfEdgeSelected(QListWidgetItem*)), this, SLOT(selectHalfEdge(QListWidgetItem*)));
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setVertXPos(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setVertYPos(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setVertZPos(double)));
    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setFaceRed(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setFaceGreen(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setFaceBlue(double)));
    connect(ui->vertJntInf0, SIGNAL(valueChanged(double)), this, SLOT(setInf0(double)));
    connect(ui->vertJntInf1, SIGNAL(valueChanged(double)), this, SLOT(setInf1(double)));
    connect(ui->addVertexButton, SIGNAL(clicked()), this, SLOT(addVertex()));
    connect(ui->triangulateButton, SIGNAL(clicked()), this, SLOT(triangulate()));
    connect(ui->catmullClarkButton, SIGNAL(clicked()), this, SLOT(subdivide()));
    connect(ui->skeleton, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(selectJoint(QTreeWidgetItem*)));
    connect(ui->rotateLeft, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(ui->rotateRight, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(ui->rotateUp, SIGNAL(clicked()), this, SLOT(rotateUp()));
    connect(ui->rotateDown, SIGNAL(clicked()), this, SLOT(rotateDown()));
    connect(ui->rotateCW, SIGNAL(clicked()), this, SLOT(rotateCW()));
    connect(ui->rotateCCW, SIGNAL(clicked()), this, SLOT(rotateCCW()));
    connect(ui->bindMesh, SIGNAL(clicked()), this, SLOT(bindMesh()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

// Retrieves the obj file address and passes it on to the mesh to
// reset it and create a new one from the obj file.
void MainWindow::on_actionLoad_OBJ_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(0, QString("Load OBJ File"), QDir::currentPath().append(QString("../..")), QString("*.obj"));
    ui->mygl->getMesh()->createFromOBJ(fileName.toStdString());
    ui->mygl->resetSelection();
    if (ui->mygl->meshBound){
        ui->mygl->bindVertices();
    }
    ui->mygl->refreshMesh();
    ui->mygl->emitInit();
}

void MainWindow::on_actionLoad_Skeleton_triggered()
{
    ui->skeleton->blockSignals(true);
    QString fileName = QFileDialog::getOpenFileName(0, QString("Load JSON File"), QDir::currentPath().append(QString("../..")), QString("*.json"));
    ui->mygl->initSkeleton(fileName.toStdString());
    ui->mygl->resetSelection();
    ui->mygl->emitInit();
    ui->mygl->refreshMesh();
//    std::vector<Joint*> skeleton;
//    retrieveJoints(ui->mygl->getJoint(), skeleton);
//    for (Joint *jnt : skeleton){
//        jnt->bindJoint();
//    }
    if (ui->mygl->meshBound){
        ui->mygl->bindSkeleton();
        ui->mygl->bindVertices();
    }
    ui->skeleton->blockSignals(false);
}


void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}


void MainWindow::loadListWidget()
{
    std::vector<Face*> faces;
    for (int i = 0; i < ui->mygl->getMesh()->faces.size(); i++)
    {
        ui->facesListWidget->addItem(ui->mygl->getMesh()->facePtr(i));
        faces.push_back(ui->mygl->getMesh()->facePtr(i));
    }

    for (int i = 0; i < ui->mygl->getMesh()->vertices.size(); i++)
    {
        ui->vertsListWidget->addItem(ui->mygl->getMesh()->vertPtr(i));
    }

    for (int i = 0; i < ui->mygl->getMesh()->halfEdges.size(); i++)
    {
        ui->halfEdgesListWidget->addItem(ui->mygl->getMesh()->edgePtr(i));
    }
    Joint *skeleton = ui->mygl->getJoint();
    ui->skeleton->reset();
    ui->skeleton->insertTopLevelItem(0, ui->mygl->getJoint());
}

void MainWindow::selectVertex(QListWidgetItem *selected)
{
    // signals blocked in order to maintain unaltered
    // geometry during the operation
    ui->faceRedSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    ui->vertJntInf1->blockSignals(true);

    ui->vertsListWidget->blockSignals(true);
    ui->vertsListWidget->setCurrentItem(selected);
    ui->vertsListWidget->blockSignals(false);

    // enabling and disabling appropriate spinboxes
    ui->mygl->selectVert(selected);
    ui->faceBlueSpinBox->setValue(0.0);
    ui->faceGreenSpinBox->setValue(0.0);
    ui->faceRedSpinBox->setValue(0.0);
    ui->faceBlueSpinBox->setDisabled(true);
    ui->faceGreenSpinBox->setDisabled(true);
    ui->faceRedSpinBox->setDisabled(true);
    ui->vertPosXSpinBox->setEnabled(true);
    ui->vertPosYSpinBox->setEnabled(true);
    ui->vertPosZSpinBox->setEnabled(true);
    ui->rotateRight->setEnabled(false);
    ui->rotateLeft->setEnabled(false);
    ui->rotateUp->setEnabled(false);
    ui->rotateDown->setEnabled(false);
    ui->rotateCW->setEnabled(false);
    ui->rotateCCW->setEnabled(false);

    if (ui->mygl->meshBound){
        ui->vertJntInf0->setEnabled(true);
        ui->vertJntInf1->setEnabled(true);
    } else {
        ui->vertJntInf0->setEnabled(false);
        ui->vertJntInf1->setEnabled(false);
    }

    Vertex *v = dynamic_cast<Vertex*>(selected);

    std::vector<Joint*> joints;
    retrieveJoints(ui->mygl->getJoint(), joints);

    // changing the spinBox values to match the vertex
    ui->vertPosXSpinBox->setValue(v->pos[0]);
    ui->vertPosYSpinBox->setValue(v->pos[1]);
    ui->vertPosZSpinBox->setValue(v->pos[2]);

    if (ui->mygl->meshBound){
        ui->vertJntInf0->setValue(v->influence[0]);
        ui->vertJntInf1->setValue(v->influence[1]);
        ui->vertJnt0->setText(QString::fromStdString(v->skin[0]->name));
        ui->vertJnt1->setText(QString::fromStdString(v->skin[1]->name));
    } else {
        ui->vertJntInf0->setValue(0.0);
        ui->vertJntInf1->setValue(0.0);
        ui->vertJnt0->setText("");
        ui->vertJnt1->setText("");
    }

    // unblocking the signals for any use outside the
    // scope of this function
    ui->faceRedSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);
    ui->vertJntInf1->blockSignals(false);

    // setting focus for the ability to interact with
    // the mygl window
    ui->mygl->setFocus();
}

void MainWindow::selectHalfEdge(QListWidgetItem *selected)
{
    // signals blocked in order to maintain unaltered
    // geometry during the operation
    ui->faceRedSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    ui->vertJntInf1->blockSignals(true);

    ui->halfEdgesListWidget->blockSignals(true);
    ui->halfEdgesListWidget->setCurrentItem(selected);
    ui->halfEdgesListWidget->blockSignals(false);

    // enabling and disabling appropriate spinboxes
    ui->mygl->selectEdge(selected);
    ui->mygl->setFocus();
    ui->vertPosXSpinBox->setEnabled(false);
    ui->vertPosYSpinBox->setEnabled(false);
    ui->vertPosZSpinBox->setEnabled(false);
    ui->faceBlueSpinBox->setDisabled(true);
    ui->faceGreenSpinBox->setDisabled(true);
    ui->faceRedSpinBox->setDisabled(true);
    ui->rotateRight->setEnabled(false);
    ui->rotateLeft->setEnabled(false);
    ui->rotateUp->setEnabled(false);
    ui->rotateDown->setEnabled(false);
    ui->rotateCW->setEnabled(false);
    ui->rotateCCW->setEnabled(false);
    ui->vertJntInf0->setEnabled(false);
    ui->vertJntInf1->setEnabled(false);

    ui->faceBlueSpinBox->setValue(0.0);
    ui->faceGreenSpinBox->setValue(0.0);
    ui->faceRedSpinBox->setValue(0.0);
    ui->vertPosXSpinBox->setValue(0.0);
    ui->vertPosYSpinBox->setValue(0.0);
    ui->vertPosZSpinBox->setValue(0.0);
    ui->vertJntInf0->setValue(0.0);
    ui->vertJntInf1->setValue(0.0);

    // unblocking the signals for any use outside the
    // scope of this function
    ui->faceRedSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);
    ui->vertJntInf1->blockSignals(false);

    // setting focus for the ability to interact with
    // the mygl window
    ui->mygl->setFocus();
}

void MainWindow::selectFace(QListWidgetItem *selected)
{
    // signals blocked in order to maintain unaltered
    // geometry during the operation
    ui->faceRedSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    ui->vertJntInf1->blockSignals(true);

    ui->facesListWidget->blockSignals(true);
    ui->facesListWidget->setCurrentItem(selected);
    ui->facesListWidget->blockSignals(false);

    // enabling and disabling appropriate spinboxes
    ui->mygl->selectFace(selected);
    ui->vertPosXSpinBox->setValue(0.0);
    ui->vertPosYSpinBox->setValue(0.0);
    ui->vertPosZSpinBox->setValue(0.0);
    ui->vertJntInf0->setValue(0.0);
    ui->vertJntInf1->setValue(0.0);
    ui->faceBlueSpinBox->setDisabled(false);
    ui->faceGreenSpinBox->setDisabled(false);
    ui->faceRedSpinBox->setDisabled(false);
    ui->vertPosXSpinBox->setEnabled(false);
    ui->vertPosYSpinBox->setEnabled(false);
    ui->vertPosZSpinBox->setEnabled(false);
    ui->rotateRight->setEnabled(false);
    ui->rotateLeft->setEnabled(false);
    ui->rotateUp->setEnabled(false);
    ui->rotateDown->setEnabled(false);
    ui->rotateCW->setEnabled(false);
    ui->rotateCCW->setEnabled(false);
    ui->vertJntInf0->setEnabled(false);
    ui->vertJntInf1->setEnabled(false);



    Face *f = dynamic_cast<Face*>(selected);

    // unblocking the signals for any use outside the
    // scope of this function
    ui->faceRedSpinBox->setValue(f->colour[0]);
    ui->faceGreenSpinBox->setValue(f->colour[1]);
    ui->faceBlueSpinBox->setValue(f->colour[2]);

    // unblocking the signals for any use outside the
    // scope of this function
    ui->faceRedSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);
    ui->vertJntInf1->blockSignals(false);

    // setting focus for the ability to interact with
    // the mygl window
    ui->mygl->setFocus();

}

// Sets the x position for either the vertex or joint, depending
// which is selected
void MainWindow::setVertXPos(double x)
{
    ui->mygl->setSelectedX(x);
    ui->mygl->setFocus();
}

// Sets the y position for either the vertex or joint, depending
// which is selected
void MainWindow::setVertYPos(double y)
{
    ui->mygl->setSelectedY(y);
    ui->mygl->setFocus();

}

// Sets the z position for either the vertex or joint, depending
// which is selected
void MainWindow::setVertZPos(double z)
{
    ui->mygl->setSelectedZ(z);
    ui->mygl->setFocus();
}

void MainWindow::setFaceRed(double r)
{
    Face *f = dynamic_cast<Face*>(ui->facesListWidget->currentItem());
    f->colour[0] = r;
    ui->mygl->refreshMesh();
}

void MainWindow::setFaceGreen(double g)
{
    Face *f = dynamic_cast<Face*>(ui->facesListWidget->currentItem());
    f->colour[1] = g;
    ui->mygl->refreshMesh();
}

void MainWindow::setFaceBlue(double b)
{
    Face *f = dynamic_cast<Face*>(ui->facesListWidget->currentItem());
    f->colour[2] = b;
    ui->mygl->refreshMesh();
}

void MainWindow::addVertex()
{
    HalfEdge *e0 = dynamic_cast<HalfEdge*>(ui->halfEdgesListWidget->currentItem());
    ui->mygl->addVertex(e0);
    ui->mygl->setFocus();
}

void MainWindow::triangulate()
{
    ui->mygl->triangulate(ui->facesListWidget->currentItem());
    ui->mygl->setFocus();
}

void MainWindow::subdivide()
{
    ui->mygl->catmullClark();
    ui->mygl->setFocus();
}

void MainWindow::selectJoint(QTreeWidgetItem *current)
{
    ui->rotateRight->setEnabled(true);
    ui->rotateLeft->setEnabled(true);
    ui->rotateUp->setEnabled(true);
    ui->rotateDown->setEnabled(true);
    ui->rotateCW->setEnabled(true);
    ui->rotateCCW->setEnabled(true);

    ui->vertPosXSpinBox->setEnabled(true);
    ui->vertPosYSpinBox->setEnabled(true);
    ui->vertPosZSpinBox->setEnabled(true);

    ui->faceRedSpinBox->setEnabled(false);
    ui->faceGreenSpinBox->setEnabled(false);
    ui->faceBlueSpinBox->setEnabled(false);
    ui->vertJntInf0->setEnabled(false);
    ui->vertJntInf1->setEnabled(false);

    ui->faceRedSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    ui->vertJntInf1->blockSignals(true);

    ui->faceBlueSpinBox->setValue(0.0);
    ui->faceGreenSpinBox->setValue(0.0);
    ui->faceRedSpinBox->setValue(0.0);
    ui->vertJntInf0->setValue(0.0);
    ui->vertJntInf1->setValue(0.0);

    Joint *curr = dynamic_cast<Joint*>(current);

    if (curr){
        ui->mygl->selectJoint(current);
        curr->selected = true;
        ui->vertPosXSpinBox->setValue(curr->pos[0]);
        ui->vertPosYSpinBox->setValue(curr->pos[1]);
        ui->vertPosZSpinBox->setValue(curr->pos[2]);
    }
    ui->mygl->refreshMesh();

    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);
    ui->vertJntInf1->blockSignals(false);

}


// Rotation functions for the joints
void MainWindow::rotateRight()
{
    ui->mygl->rotateJoint(0.1, glm::vec3(0, 1, 0));
    ui->mygl->setFocus();
}

void MainWindow::rotateLeft()
{
    ui->mygl->rotateJoint(-0.1, glm::vec3(0, 1, 0));
    ui->mygl->setFocus();
}

void MainWindow::rotateUp()
{
    ui->mygl->rotateJoint(-0.1, glm::vec3(1, 0, 0));
    ui->mygl->setFocus();
}

void MainWindow::rotateDown()
{
    ui->mygl->rotateJoint(0.1, glm::vec3(1, 0, 0));
    ui->mygl->setFocus();
}

void MainWindow::rotateCW()
{
    ui->mygl->rotateJoint(-0.1, glm::vec3(0, 0, 1));
    ui->mygl->setFocus();
}

void MainWindow::rotateCCW()
{
    ui->mygl->rotateJoint(0.1, glm::vec3(0, 0, 1));
    ui->mygl->setFocus();
}

// Sets the influence for the first of the joints that the
// selected vertex is bound to
void MainWindow::setInf0(double val)
{
    ui->vertJntInf1->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    Vertex *v = dynamic_cast<Vertex*>(ui->vertsListWidget->currentItem());
    ui->vertJntInf1->setValue(1 - val);
    ui->mygl->setInfluence(v, val, 0);
    ui->mygl->setFocus();
    ui->vertJntInf1->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);
}

// Sets the influence for the second of the joints that the
// selected vertex is bound to
void MainWindow::setInf1(double val)
{
    ui->vertJntInf1->blockSignals(true);
    ui->vertJntInf0->blockSignals(true);
    Vertex *v = dynamic_cast<Vertex*>(ui->vertsListWidget->currentItem());
    ui->vertJntInf0->setValue(1 - val);
    ui->mygl->setInfluence(v, val, 1);
    ui->mygl->setFocus();
    ui->vertJntInf1->blockSignals(false);
    ui->vertJntInf0->blockSignals(false);

}

// Binds or unbinds the mesh and vertices as the pushButton is pressed
void MainWindow::bindMesh()
{
    if (!ui->mygl->meshBound){
        ui->mygl->bindSkeleton();
        ui->mygl->bindVertices();
        ui->mygl->meshBound = true;
        ui->mygl->refreshMesh();
        ui->bindMesh->setText("Unbind Mesh");
    } else {
        ui->mygl->meshBound = false;
        ui->mygl->refreshMesh();
        ui->bindMesh->setText("Bind Mesh");
    }
    if (ui->vertsListWidget->currentItem()){
        selectVertex(ui->vertsListWidget->currentItem());
    }
    ui->mygl->setFocus();
}
