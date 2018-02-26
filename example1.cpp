
#include "Angel.h"

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// windowsize
int window_width = 512;
int window_height = 512;
// vertexices for a 3-D cube
point4 vertices[8] = {
    point4(-0.5, -0.5, 0.5, 1.0),
    point4(-0.5, 0.5, 0.5, 1.0),
    point4(0.5, 0.5, 0.5, 1.0),
    point4(0.5, -0.5, 0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5, 0.5, -0.5, 1.0),
    point4(0.5, 0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)};

// RGBA olors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0), // black
    color4(0.8, 0.0, 0.0, 1.0), // red
    color4(1.0, 0.7, 0.0, 1.0), // yellow
    color4(0.0, 1.0, 0.0, 1.0), // green
    color4(0.0, 0.1, 0.8, 1.0), // blue
    color4(1.0, 0.0, 1.0, 1.0), // magenta
    color4(0.0, 1.0, 1.0, 1.0), // cyan
    color4(1.0, 1.0, 1.0, 1.0)  // white

};

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT = 2.0;
const GLfloat BASE_WIDTH = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH = 0.5;

// Shader transformation matrices
mat4 model_view;
enum
{
    Front = 0,
    Up = 1,
    Right = 2,
    NumDirections = 3
};
int dir_selector = 0;
mat4 camera_view[NumDirections]; // for 3 view direction
mat4 projection[NumDirections];

mat4 transformation;
GLuint ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum
{
    Base = 0,
    LowerArm = 1,
    UpperArm = 2,
    NumAngles = 3
};
int Axis = Base;
GLfloat Theta[NumAngles] = {0.0};

// Menu option values
const int Quit = 6;

//----------------------------------------------------------------------------

int Index = 0;

void quad(int a, int b, int c, int d)
{
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[a];
    Index++;
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[b];
    Index++;
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[c];
    Index++;
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[a];
    Index++;
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[c];
    Index++;
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[d];
    Index++;
}

void colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------
/* Init the camera and projection for different direction*/
void init_camera()
{

    // projection[up] = Ortho(left, right, bottom, top, zNear, zFar);
    camera_view[Up] = LookAt(
        vec3(0.0, 0.3, 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 0.0, -1.0));
    projection[Up] = Ortho(-15, 15, -15, 15, -20, 10);

    camera_view[Front] = LookAt(
        vec3(0.0, 0.0, 0.3),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0));
    projection[Front] = Ortho(-15, 15, -10, 20, -10, 10);

    camera_view[Right] = LookAt(
        vec3(0.3, 0.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0));
    projection[Right] = Ortho(-15, 15, -10, 20, -10, 10);
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base()
{
    mat4 instance = (Translate(0.0, 0.5 * BASE_HEIGHT, 0.0) *
                     Scale(BASE_WIDTH,
                           BASE_HEIGHT,
                           BASE_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
                     Scale(UPPER_ARM_WIDTH,
                           UPPER_ARM_HEIGHT,
                           UPPER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
                     Scale(LOWER_ARM_WIDTH,
                           LOWER_ARM_HEIGHT,
                           LOWER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Accumulate ModelView Matrix as we traverse the tree
    model_view = camera_view[dir_selector];
    transformation = RotateY(Theta[Base]);
    // model_view = RotateY(Theta[Base]);
    base();

    // model_view *= (Translate(0.0, BASE_HEIGHT, 0.0) *
    //                RotateZ(Theta[LowerArm]));
    transformation *= (Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LowerArm]));
    lower_arm();

    // model_view *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
    //                RotateZ(Theta[UpperArm]));
    transformation *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0)) * RotateZ(Theta[UpperArm]);
    upper_arm();

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init(void)
{
    init_camera();

    colorcube();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
                 NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader81.glsl", "fshader81.glsl");
    glUseProgram(program);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)));

    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    glEnable(GL_DEPTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw the edge

    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Incrase the joint angle
        Theta[Axis] += 5.0;
        if (Theta[Axis] > 360.0)
        {
            Theta[Axis] -= 360.0;
        }
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // Decrase the joint angle
        Theta[Axis] -= 5.0;
        if (Theta[Axis] < 0.0)
        {
            Theta[Axis] += 360.0;
        }
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu(int option)
{
    if (option == Quit)
    {
        exit(EXIT_SUCCESS);
    }
    else if(option < NumAngles)
    {
        Axis = option;
    }
    else{
        dir_selector = option - NumAngles;
    }
}

//----------------------------------------------------------------------------

void reshape(int width, int height)
{
    glutReshapeWindow(window_width, window_height);

    // glViewport(0, 0, width, height);

    // GLfloat left = -15, right = 15.0;
    // GLfloat bottom = -15.0, top = 15.0;
    // GLfloat zNear = -20.0, zFar = 10.0;

    // GLfloat aspect = GLfloat(width) / height;

    // if (aspect > 1.0)
    // {
    //     left *= aspect;
    //     right *= aspect;
    // }
    // else
    // {
    //     bottom /= aspect;
    //     top /= aspect;
    // }

    // mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);

    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection[dir_selector]);

    model_view = mat4(1.0); // An Identity matrix
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 033: // Escape Key
    case 'q':
    case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case 'w':
        dir_selector = Up;
        break;
    case 'e':
        dir_selector = Front;
        break;
    case 'r':
        dir_selector = Right;
        break;
    case 'z':
        Axis = Base;
        break;
    case 'x':
        Axis = LowerArm;
        break;
    case 'c':
        Axis = UpperArm;
        break;
    }
}

//----------------------------------------------------------------------------
void specialkey(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
        // Incrase the joint angle
        Theta[Axis] += 5.0;
        if (Theta[Axis] > 360.0)
        {
            Theta[Axis] -= 360.0;
        }
        break;
    case GLUT_KEY_RIGHT:
        // Decrase the joint angle
        Theta[Axis] -= 5.0;
        if (Theta[Axis] < 0.0)
        {
            Theta[Axis] += 360.0;
        }
        break;
    }
}
//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("robot");
    glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialkey);
    glutMouseFunc(mouse);

    int current_menu = glutCreateMenu(menu);
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry("base", Base);
    glutAddMenuEntry("lower arm", LowerArm);
    glutAddMenuEntry("upper arm", UpperArm);
    glutAddMenuEntry("front view", Front + NumAngles);    
    glutAddMenuEntry("up view", Up + NumAngles);
    glutAddMenuEntry("side view", Right + NumAngles);
    glutAddMenuEntry("quit", Quit);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    glutSetMenu(current_menu);
    glutMainLoop();
    return 0;
}
