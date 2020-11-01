#ifndef JOINT_H
#define JOINT_H

#include <QTreeWidget>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <drawable.h>
#include <smartpointerhelp.h>

// inherits both Drawable (to be able to be rendered),
// and QTreeWidgeItem (for display in the GUI)
class Joint : public Drawable, public QTreeWidgetItem
{
private:
    // counter to specify the iD of the joint
    static int counter;
public:
    Joint(OpenGLContext *context);

    Joint(OpenGLContext *context, std::string iName, Joint *iParent, glm::vec3 iPos);

    unsigned int iD;
    std::string name;

    // Pointer to the parent of the joint
    Joint *parent;

    // Position relative to parent
    glm::vec3 pos;

    // bool specifying if the joint is selected
    // for it to be rendered in a different color
    bool selected;

    // Vector of children
    std::vector<uPtr<Joint>> children;

    // quaternion containing rotation info
    glm::quat rotation;

    // bind matrix for the joint
    glm::mat4 bind;

    void create() override;
    GLenum drawMode() override;

    // Adds a child while setting its position to the given
    // paramenter and parent pointer to this
    Joint *addChild(glm::vec3 newPos);

    // Transformation matrices for the joint
    glm::mat4 getLocalTransformation();
    glm::mat4 getOverallTransformation();

    // Sets the bind matrix for the joint
    void bindJoint();

    // Rotates the joint based on given parameters
    void rotate(float angle, glm::vec3 axis);

    // Functions used to initialize joints from a given JSON file
    void loadJSON(std::string fileName);
    void createJoint(QJsonObject);
};

// Retrieves all the children of a joint and places pointers to them in a std::vector<Joint*>
void retrieveJoints (Joint *j, std::vector<Joint *> &acc);

#endif // JOINT_H
