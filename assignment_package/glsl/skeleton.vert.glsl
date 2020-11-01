#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform mat4[100] u_Bind;
uniform mat4[100] u_Trans;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader


in vec4 vs_Col;             // The array of vertex colors passed to the shader.

in ivec2 vs_Jnt;

in vec2 vs_Inf;


out vec3 fs_Pos;
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.



void main()
{
    fs_Col = vs_Col;
    // Pass the vertex colors to the fragment shader for interpolation

    mat3 invTranspose = mat3(u_ModelInvTr);


    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    fs_Pos = modelposition.xyz;

    mat4 shift0 = vs_Inf[0] * (u_Bind[vs_Jnt[0]] * u_Trans[vs_Jnt[0]]); // The shift matrix stores the interpolated
                                                                        // transformation for a vertex
    mat4 shift1 = vs_Inf[1] * (u_Bind[vs_Jnt[1]] * u_Trans[vs_Jnt[1]]);

    if (vs_Inf[1] == 0.0){ // Set the shift matrices to identity if the respective influence is equal to 0
        shift1 = mat4(1.0);
    } else if (vs_Inf[0] == 0.0){
        shift0 = mat4(1.0);
    }

    gl_Position = u_ViewProj * shift1 * shift0 * modelposition; // gl_Position is a built-in variable of OpenGL which is
                                                                // used to render the final positions of the geometry's vertices

}
