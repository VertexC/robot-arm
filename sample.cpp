#include "Angel.h"
#include <string.h>
#include <sys/time.h>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4(-0.5, -0.5, 0.5, 1.0),
    point4(-0.5, 0.5, 0.5, 1.0),
    point4(0.5, 0.5, 0.5, 1.0),
    point4(0.5, -0.5, 0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5, 0.5, -0.5, 1.0),
    point4(0.5, 0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)};

// RGBA colors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0), // black
    color4(1.0, 0.0, 0.0, 1.0), // red
    color4(1.0, 1.0, 0.0, 1.0), // yellow
    color4(0.0, 1.0, 0.0, 1.0), // green
    color4(0.0, 0.0, 1.0, 1.0), // blue
    color4(1.0, 0.0, 1.0, 1.0), // magenta
    color4(1.0, 1.0, 1.0, 1.0), // white
    color4(0.0, 1.0, 1.0, 1.0)  // cyan
};

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT = 2.0;
const GLfloat BASE_WIDTH = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH = 0.5;
const GLfloat BALL_RADIUS = 0.25;

// Shader transformation matrices
mat4 model_view;
GLuint ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum
{
    Base = 0,
    LowerArm = 1,
    UpperArm = 2,
    NumAngles = 3
};
const int TopView = 3;
int Axis = Base;
GLfloat Theta[NumAngles] = {0.0};

// Menu option values
const int Quit = 4;

bool isTopView = false;
bool FetchMode = false;
bool withBall = false;
bool finishFetch = false;
point4 old_position;
point4 new_position;
point4 cur_position;
GLfloat oldTheta[NumAngles] = {0.0};
GLfloat newTheta[NumAngles] = {0.0};
GLfloat startTheta[NumAngles] = {0.0};
GLfloat targetTheta[NumAngles] = {0.0};
GLfloat moveTime[NumAngles] = {0.0};
timeval startTime;
GLfloat speed = 360.0 / 5000.0; // 180 per 5000ms (5s)

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

int elapsed()
{
    timeval current;
    gettimeofday(&current, NULL);
    double elapsedTime;
    elapsedTime = (current.tv_sec - startTime.tv_sec) * 1000.0;
    elapsedTime += (current.tv_usec - startTime.tv_usec) / 1000.0;
    return int(elapsedTime); // ms
}

void setMoveTime()
{
    moveTime[Base] = fabs(targetTheta[Base] - startTheta[Base]) / speed;
    moveTime[LowerArm] = fabs(targetTheta[LowerArm] - startTheta[LowerArm]) / speed;
    moveTime[UpperArm] = fabs(targetTheta[UpperArm] - startTheta[UpperArm]) / speed;
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

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
                     Scale(UPPER_ARM_WIDTH,
                           UPPER_ARM_HEIGHT,
                           UPPER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
                     Scale(LOWER_ARM_WIDTH,
                           LOWER_ARM_HEIGHT,
                           LOWER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void ball()
{
    mat4 instance = (Translate(cur_position) *
                     Scale(BALL_RADIUS * 2,
                           BALL_RADIUS * 2,
                           BALL_RADIUS * 2));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    //    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glutSolidSphere(BALL_RADIUS, 50, 50);
}

//----------------------------------------------------------------------------

void idle()
{
    glutPostRedisplay();
}

double
sign(double x)
{
    if (x < 0)
    {
        return -1;
    }
    return 1;
}

int min(double x, double y)
{
    if (x > y)
    {
        return y;
    }
    return x;
}

double
cosLaw(double a, double b, double c)
{
    return acos((pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b)) * 180 / M_PI;
}

bool move(int part)
{
    double elapsed_time = elapsed();
    if (elapsed_time <= moveTime[part])
    {
        Theta[part] = startTheta[part] + elapsed_time * speed * sign(targetTheta[part] - startTheta[part]);
        if (Theta[part] < 0.0)
        {
            Theta[part] += 360.0;
        }
        return true;
    }
    Theta[part] = targetTheta[part];
    return false;
}

void updateCurPosition()
{
    cur_position.x = 0;
    cur_position.y = LOWER_ARM_HEIGHT;
    cur_position.z = 0;
}

void setWithBall()
{
    withBall = true;
    updateCurPosition();
    startTheta[Base] = oldTheta[Base];
    startTheta[LowerArm] = oldTheta[LowerArm];
    startTheta[UpperArm] = oldTheta[UpperArm];
    targetTheta[Base] = newTheta[Base];
    targetTheta[LowerArm] = newTheta[LowerArm];
    targetTheta[UpperArm] = newTheta[UpperArm];
    gettimeofday(&startTime, NULL);
    setMoveTime();
}

void setFinishFetch()
{
    withBall = false;
    finishFetch = true;
    cur_position = new_position;
    startTheta[Base] = newTheta[Base];
    startTheta[LowerArm] = newTheta[LowerArm];
    startTheta[UpperArm] = newTheta[UpperArm];
    targetTheta[Base] = 0;
    targetTheta[LowerArm] = 0;
    targetTheta[UpperArm] = 0;
    gettimeofday(&startTime, NULL);
    setMoveTime();
}

void setRotateBack()
{
    FetchMode = false;
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Accumulate ModelView Matrix as we traverse the tree
    if (isTopView)
    {
        model_view = (Translate(0, BASE_WIDTH, 0) *
                      RotateX(90.0));
    }
    else
    {
        model_view = (RotateX(0));
    }

    if (FetchMode)
    {
        bool baseMoved = false;
        bool lowerArmMoved = false;
        bool upperArmMoved = false;
        if (!withBall)
        {
            ball();
        }

        baseMoved = move(Base);
        model_view *= RotateY(Theta[Base]);
        base();

        lowerArmMoved = move(LowerArm);
        model_view *= (Translate(0.0, BASE_HEIGHT, 0.0) *
                       RotateZ(Theta[LowerArm]));
        lower_arm();

        upperArmMoved = move(UpperArm);
        model_view *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
                       RotateZ(Theta[UpperArm]));
        upper_arm();

        if (withBall)
        {
            ball();
        }

        if (!baseMoved && !lowerArmMoved && !upperArmMoved)
        {
            if (!withBall & !finishFetch)
            {
                setWithBall();
            }
            else if (withBall)
            {
                setFinishFetch();
            }
            else if (finishFetch)
            {
                setRotateBack();
            }
        }
    }
    else
    {
        if (finishFetch)
        {
            ball();
        }
        model_view *= RotateY(Theta[Base]);
        base();
        model_view *= (Translate(0.0, BASE_HEIGHT, 0.0) *
                       RotateZ(Theta[LowerArm]));
        lower_arm();

        model_view *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
                       RotateZ(Theta[UpperArm]));
        upper_arm();
    }

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init(void)
{
    colorcube();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);

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
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
    else if (option == TopView)
    {
        isTopView = !isTopView;
        if (isTopView)
        {
            glutChangeToMenuEntry(TopView + 1, "side view", TopView);
        }
        else
        {
            glutChangeToMenuEntry(TopView + 1, "top view", TopView);
        }
    }
    else
    {
        Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    GLfloat left = -10.0, right = 10.0;
    GLfloat bottom = -5.0, top = 15.0;
    GLfloat zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width) / height;

    if (aspect > 1.0)
    {
        left *= aspect;
        right *= aspect;
    }
    else
    {
        bottom /= aspect;
        top /= aspect;
    }

    mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

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
    }
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    if (argc > 1)
    {
        old_position = point4(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1.0);
        new_position = point4(atof(argv[4]), atof(argv[5]), atof(argv[6]), 1.0);
        if (old_position.x == 0)
        {
            old_position.x = 0.000000001;
        }
        if (new_position.x == 0)
        {
            new_position.x = 0.000000001;
        }
        FetchMode = true;
        cur_position = old_position;
        if (strcmp(argv[7], "-tv") == 0)
        {
            isTopView = true;
        }

        {
            oldTheta[Base] = -atan(old_position.z / old_position.x) * 180.0 / M_PI;
            double origin_old_x = old_position.x;
            old_position.x = sqrt(pow(old_position.x, 2) + pow(old_position.z, 2));
            old_position.y -= BASE_HEIGHT;
            double oldTopTheta = cosLaw(LOWER_ARM_HEIGHT, UPPER_ARM_HEIGHT + BALL_RADIUS, length(vec2(old_position.x, old_position.y)));
            double oldBottomTheta = cosLaw(LOWER_ARM_HEIGHT, length(vec2(old_position.x, old_position.y)), UPPER_ARM_HEIGHT + BALL_RADIUS);
            double oldBallBottomTheta = atan(old_position.x / old_position.y) * 180.0 / M_PI;
            if (old_position.y < 0)
            {
                oldBallBottomTheta += 180.0;
            }
            oldTheta[LowerArm] = oldBottomTheta - oldBallBottomTheta;
            oldTheta[UpperArm] = oldTopTheta - 180.0;
            if (origin_old_x < 0)
            {
                oldTheta[LowerArm] = -oldTheta[LowerArm];
                oldTheta[UpperArm] = -oldTheta[UpperArm];
            }
            old_position.x = origin_old_x;
            old_position.y += BASE_HEIGHT;
        }

        {
            newTheta[Base] = -atan(new_position.z / new_position.x) * 180.0 / M_PI;
            double origin_new_x = new_position.x;
            new_position.x = sqrt(pow(new_position.x, 2) + pow(new_position.z, 2));
            new_position.y -= BASE_HEIGHT;
            double newTopTheta = cosLaw(LOWER_ARM_HEIGHT, UPPER_ARM_HEIGHT + BALL_RADIUS, length(vec2(new_position.x, new_position.y)));
            double newBottomTheta = cosLaw(LOWER_ARM_HEIGHT, length(vec2(new_position.x, new_position.y)), UPPER_ARM_HEIGHT + BALL_RADIUS);
            double newBallBottomTheta = atan(new_position.x / new_position.y) * 180.0 / M_PI;
            if (new_position.y < 0)
            {
                newBallBottomTheta += 180.0;
            }
            newTheta[LowerArm] = newBottomTheta - newBallBottomTheta;
            newTheta[UpperArm] = newTopTheta - 180.0;
            if (origin_new_x < 0)
            {
                newTheta[LowerArm] = -newTheta[LowerArm];
                newTheta[UpperArm] = -newTheta[UpperArm];
            }
            new_position.x = origin_new_x;
            new_position.y += BASE_HEIGHT;
        }

        targetTheta[Base] = oldTheta[Base];
        targetTheta[LowerArm] = oldTheta[LowerArm];
        targetTheta[UpperArm] = oldTheta[UpperArm];
        setMoveTime();
    }
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);    
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("robot");
    gettimeofday(&startTime, NULL);
    glewExperimental = GL_TRUE;
    glewInit();
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutCreateMenu(menu);
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry("base", Base);
    glutAddMenuEntry("lower arm", LowerArm);
    glutAddMenuEntry("upper arm", UpperArm);
    if (isTopView)
    {
        glutAddMenuEntry("side view", TopView);
    }
    else
    {
        glutAddMenuEntry("top view", TopView);
    }
    glutAddMenuEntry("quit", Quit);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);

    glutMainLoop();
    return 0;
}
