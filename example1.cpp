
#include "Angel.h"
#include "string.h"
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

// robot cube
point4 points[NumVertices];
color4 colors[NumVertices];

// sphere
const float PI = 3.14;
const int SPHERE_ROW = 20;
const int SPHERE_COL = 20;
const int SPHERE_RADIUS = 1;
point4 spherePoints[SPHERE_ROW * SPHERE_COL * 2 * 3];
color4 sphereColors[SPHERE_ROW * SPHERE_COL * 2 * 3];
GLuint vaos[2]; // 0: robot cube, 1:line loop, 2:triangle fan
GLuint vbos[2]; // 0: robot cube.v.c, 1,2:line loop.v,c, 3,4:triangle fan.v.c
// sphere movement
point4 start_position = point4(0.0, 0.0, 0.0, 1.0);
point4 end_position = point4(0.0, 0.0, 0.0, 1.0);
point4 current_position = point4(0.0, 0.0, 0.0, 1.0);
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
const GLfloat BALL_RADIUS = 1;
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
/* Init the position and color of sphere*/
point4 f_u_v(float u, float v)
{
    return point4(cos(u) * sin(v),
                  cos(v), sin(u) * sin(v), 1.0);
}
void init_sphere()
{
    float start_u = 0.0;
    float start_v = 0.0;
    float end_u = PI * 2.0;
    float end_v = PI * 1.0;
    float step_u = (end_u - start_u) / SPHERE_COL;
    float step_v = (end_v - start_v) / SPHERE_ROW;
    int k = 0;
    for (int i = 0; i < SPHERE_ROW; i++)
    {
        for (int j = 0; j < SPHERE_COL; j++)
        {
            float u = i * step_u + start_u;
            float v = j * step_v + start_v;
            float un = u + step_u;
            float vn = v + step_v;
            point4 p0 = f_u_v(u, v);
            point4 p1 = f_u_v(u, vn);
            point4 p2 = f_u_v(un, v);
            point4 p3 = f_u_v(un, vn);
            spherePoints[k++] = p0;
            spherePoints[k++] = p2;
            spherePoints[k++] = p1;
            spherePoints[k++] = p3;
            spherePoints[k++] = p1;
            spherePoints[k++] = p2;
        }
    }
    for (int i = 0; i < SPHERE_COL * SPHERE_ROW; i++)
    {
        sphereColors[i] = color4(0.0, 0.0, 0.0, 1.0); // black
    }
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
    glBindVertexArray(vaos[0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
                     Scale(UPPER_ARM_WIDTH,
                           UPPER_ARM_HEIGHT,
                           UPPER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);
    glBindVertexArray(vaos[0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
                     Scale(LOWER_ARM_WIDTH,
                           LOWER_ARM_HEIGHT,
                           LOWER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);
    glBindVertexArray(vaos[0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------
void sphere()
{
    mat4 instance = (Translate(0, 0, 0) *
                     Scale(SPHERE_RADIUS,
                           SPHERE_RADIUS,
                           SPHERE_RADIUS));
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * instance);
    glBindVertexArray(vaos[1]);
    glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * SPHERE_COL * SPHERE_ROW);
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Accumulate ModelView Matrix as we traverse the tree

    model_view = camera_view[dir_selector];

    transformation = Translate(
        current_position.x,
        current_position.y,
        current_position.z);
    sphere();

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

    init_sphere();

    colorcube();

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader81.glsl", "fshader81.glsl");
    glUseProgram(program);
    GLuint vColor = glGetAttribLocation(program, "vColor");
    GLuint vPosition = glGetAttribLocation(program, "vPosition");

    // Create vertex array object
    glGenVertexArrays(2, vaos);

    // Create and initialize a buffer object for robot cube
    glBindVertexArray(vaos[0]);
    glGenBuffers(1, &vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
                 NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
    // for robot arms
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)));

    // Create and initialize a buffer object for sphere
    glBindVertexArray(vaos[1]);
    glGenBuffers(1, &vbos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(colors),
                 NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints),
                    sizeof(sphereColors), sphereColors);
    // for sphere
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(spherePoints)));

    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    glEnable(GL_DEPTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw the edge

    // Draw the sphere

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
    else if (option < NumAngles)
    {
        Axis = option;
    }
    else
    {
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
    // calculate the movement
    if (argc == 8)
    {
        // check the view mode
        if (strcmp(argv[7], "-tv") == 0)
        {
            dir_selector = Up;
        }
        else if (strcmp(argv[7], "-sv") == 0)
        {
            dir_selector = Right;
        }
        // set the ball to the start point
        start_position = point4(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1.0);
        end_position = point4(atof(argv[4]), atof(argv[6]), atof(argv[6]), 1.0);
        // test
        current_position = start_position;
    }

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
