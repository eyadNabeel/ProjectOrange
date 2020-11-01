#include "joint.h"
#include <glm/gtc/matrix_transform.hpp>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <iostream>

int Joint::counter = 0;

Joint::Joint(OpenGLContext *context)
    :Drawable(context),
     iD(counter), name("Joint #" + std::to_string(counter)),
     parent(nullptr), pos(0.f, 0.f, 0.f),
     selected(false), rotation(glm::quat()),
     bind(glm::mat4(1.f))
{
    QTreeWidgetItem::setText(0, QString::fromStdString(name));
    counter++;
}

Joint::Joint(OpenGLContext *context, std::string iName, Joint *iParent, glm::vec3 iPos)
    :Drawable(context),
     iD(counter), name(iName),
     parent(iParent), pos(iPos),
     selected(false), rotation(glm::quat()),
     bind(glm::mat4(1.f))
{
    QTreeWidgetItem::setText(0, QString::fromStdString(name));
    counter++;
}

// The transformtaion of the joint independent of parents
glm::mat4 Joint::getLocalTransformation()
{
    return glm::translate(glm::mat4(), pos) * glm::toMat4(rotation);
}

// Gets the transformation for the joint from (0, 0, 0)
glm::mat4 Joint::getOverallTransformation()
{
    if (!parent){
        return getLocalTransformation();
    }
    return parent->getOverallTransformation() * getLocalTransformation();
}

// Adds a child, setting its parent pointer to this
Joint *Joint::addChild(glm::vec3 newPos)
{
    children.push_back(mkU<Joint>(mp_context));
    children[children.size() - 1].get()->parent = this;
    children[children.size() - 1].get()->pos = newPos;
    QTreeWidgetItem::addChild(children[children.size() - 1].get());
    return children[children.size() - 1].get();
}

// Sets the bind matrix to the inverse of the current transformation
void Joint::bindJoint()
{
    bind = glm::inverse(getOverallTransformation());
}


// Rotates the joint by multiplying the quaternion
void Joint::rotate(float angle, glm::vec3 axis)
{
    glm::vec3 normalizedAxis = glm::normalize(axis);
    rotation *= glm::quat(glm::cos(angle/2.f),
                          glm::sin(angle/2.f) * normalizedAxis[0],
                          glm::sin(angle/2.f) * normalizedAxis[1],
                          glm::sin(angle/2.f) * normalizedAxis[2]);
}


// This function runs recursively on the QJsonObject derived from
// the JSON file in loadJSON(), and creates joints and sets their
// member variables
void Joint::createJoint(QJsonObject jnt)
{
    QString jName = jnt["name"].toString();
    QJsonArray jPos = jnt["pos"].toArray();
    QJsonArray jRot = jnt["rot"].toArray();
    name = jName.toStdString();
    pos = glm::vec3(jPos[0].toDouble(),
                    jPos[1].toDouble(),
                    jPos[2].toDouble());
    rotate(jRot[0].toDouble(),
            glm::vec3(jRot[1].toDouble(),
                      jRot[2].toDouble(),
                      jRot[3].toDouble()));
    QJsonArray jChildren = jnt["children"].toArray();
    QTreeWidgetItem::setText(0, jName);
    for (int i = 0; i < jChildren.size(); i++){
        QJsonObject child = jChildren[i].toObject();
        addChild(glm::vec3(0, 0, 0));
        children[children.size() - 1]->createJoint(child);
    }
}

void Joint::loadJSON(std::string fileName)
{
    children.clear();
    iD = 0;
    Joint::counter = 1;
    QString filename = QString::fromStdString(fileName);
    int i = filename.length() - 1;
    while(QString::compare(filename.at(i), QChar('/')) != 0)
    {
        i--;
    }
    QStringRef local_path = filename.leftRef(i+1);

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning("Could not open the JSON file.");
        return;
    }
    QByteArray file_data = file.readAll();

    QJsonDocument jdoc(QJsonDocument::fromJson(file_data));
    QJsonObject root = jdoc.object()["root"].toObject();
    createJoint(root);
}

void Joint::create()
{
    std::vector<glm::vec4> circleUp;
    std::vector<glm::vec4> circleRight;
    std::vector<glm::vec4> circleForward;

    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> col;
    std::vector<int> idx;


    int numSides = 30;
    glm::vec4 pR(0.f, 0.f, 0.5f, 1.f);
    glm::vec4 pU(0.5f, 0.f, 0.f, 1.f);
    glm::vec4 pF(0.5f, 0.f, 0.f, 1.f);

    float deg = glm::radians(360.f / numSides);
    for (int i = 0; i < numSides; i++)
    {
        glm::mat4 mU = glm::rotate(glm::mat4(), i * deg, glm::vec3(0, 1, 0));
        glm::mat4 mR = glm::rotate(glm::mat4(), i * deg, glm::vec3(1, 0, 0));
        glm::mat4 mF = glm::rotate(glm::mat4(), i * deg, glm::vec3(0, 0, 1));
        circleUp.push_back(mU * pU);
        circleRight.push_back(mR * pR);
        circleForward.push_back(mF * pF);
    }

    for (glm::vec4 &vert : circleUp){
        vert = getOverallTransformation() * vert;
        positions.push_back(vert);
        if (selected){
            col.push_back(glm::vec4(1, 1, 1, 1));
        } else {
            col.push_back(glm::vec4(0, 1, 0, 1));
        }
    }
    for (glm::vec4 &vert : circleRight){
        vert = getOverallTransformation() * vert;
        positions.push_back(vert);
        if (selected){
            col.push_back(glm::vec4(1, 1, 1, 1));
        } else {
            col.push_back(glm::vec4(1, 0, 0, 1));
        }
    }
    for (glm::vec4 &vert : circleForward){
        vert = getOverallTransformation() * vert;
        positions.push_back(vert);
        if (selected){
            col.push_back(glm::vec4(1, 1, 1, 1));
        } else {
            col.push_back(glm::vec4(0, 0, 1, 1));
        }
    }
    for (int i = 0; i < circleUp.size(); i++){
        idx.push_back(i);
        idx.push_back((i + 1) % (circleUp.size() - 1));
    }
    int loopLengthA = circleUp.size() + circleRight.size();
    for (int i = circleUp.size(); i < loopLengthA; i++){
        idx.push_back(i);
        if (i == loopLengthA - 1){
            idx.push_back(circleUp.size());
        } else {
            idx.push_back((i + 1));
        }
    }
    int loopLengthB = circleForward.size() + loopLengthA;
    for (int i = loopLengthA; i < loopLengthB; i++){
        idx.push_back(i);
        if (i == loopLengthB - 1){
            idx.push_back(loopLengthA);
        } else {
            idx.push_back((i + 1));
        }
    }

    if (parent){
        int len = positions.size();
        positions.push_back(getOverallTransformation() * glm::vec4(0, 0.5, 0, 1));
        positions.push_back(getOverallTransformation() * glm::vec4(0, -0.5, 0, 1));
        positions.push_back(getOverallTransformation() * glm::vec4(0, 0, 0.5, 1));
        positions.push_back(getOverallTransformation() * glm::vec4(0, 0, -0.5, 1));
        positions.push_back(getOverallTransformation() * glm::vec4(0.5, 0, 0, 1));
        positions.push_back(getOverallTransformation() * glm::vec4(-0.5, 0, 0, 1));
        positions.push_back(getOverallTransformation() * glm::inverse(glm::toMat4(rotation)) * glm::vec4(-pos, 1));
        glm::vec4 color;
        if (selected){
            color = glm::vec4(1, 1, 1, 1);
        } else {
            color = glm::vec4(1, 1, 0, 1);
        }
        col.push_back(color);
        col.push_back(color);
        col.push_back(color);
        col.push_back(color);
        col.push_back(color);
        col.push_back(color);
        if (selected){
            col.push_back(glm::vec4(1, 1, 1, 1));
        } else {
            col.push_back(glm::vec4(1, 0, 1, 1));
        }
        idx.push_back(len);
        idx.push_back(len + 6);
        idx.push_back(len + 1);
        idx.push_back(len + 6);
        idx.push_back(len + 2);
        idx.push_back(len + 6);
        idx.push_back(len + 3);
        idx.push_back(len + 6);
        idx.push_back(len + 4);
        idx.push_back(len + 6);
        idx.push_back(len + 5);
        idx.push_back(len + 6);

    }

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec4), positions.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

    for (uPtr<Joint> &child : children){
        child.get()->create();
    }
}


void retrieveJoints (Joint *j, std::vector<Joint *> &acc)
{
    acc.push_back(j);
    for (uPtr<Joint> &child : j->children){
        retrieveJoints(child.get(), acc);
    }
}

GLenum Joint::drawMode()
{
    return GL_LINES;
}
