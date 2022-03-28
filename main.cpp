#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glsl.h"
#include "objloader.h"
#include "texture.h"

using namespace std;


//--------------------------------------------------------------------------------
// Consts
//--------------------------------------------------------------------------------

const int WIDTH = 800, HEIGHT = 600;
const unsigned int objectCount = 4;

const char* fragshader_name = "fragmentshader.fsh";
const char* vertexshader_name = "vertexshader.vsh";


//Time
unsigned const int DELTA_TIME = 1;
unsigned int timeSinceStart = 0;
unsigned int oldTimeSinceStart;
unsigned int deltaTime;

vector<glm::vec3> normals[objectCount];
vector<glm::vec3> vertices[objectCount];
vector<glm::vec2> uvs[objectCount];


//Camera values
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
glm::vec3 cameraTarget;
glm::vec3 cameraDirection;

//Yaw/Pitch
float yaw = -90.0f;
float pitch = 0.0f;

//Input buffer
bool keyBuffer[128];

glm::vec3 light_position, ambient_color, diffuse_color, specular;
float power;






//--------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------

// ID's
GLuint program_id;
GLuint vao[objectCount];
GLuint texture_id[objectCount];

// Uniform ID's
GLuint uniform_mv;
GLuint uniform_proj;
GLuint uniform_light_pos;
GLuint uniform_material_ambient;
GLuint uniform_material_diffuse;
GLuint uniform_specular;
GLuint uniform_material_power;


// Matrices
glm::mat4 model[objectCount], view, projection;
glm::mat4 mv[objectCount];

struct LightSource {
    glm::vec3 position;
};

struct Material {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular;
    float power;
};

Material materials[objectCount];
LightSource light;



void CalculateCameraDirection() {
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(cameraDirection);
}


//--------------------------------------------------------------------------------
// Keyboard handling
//--------------------------------------------------------------------------------

void keyboardHandler(unsigned char key, int a, int b)
{
    keyBuffer[key] = true;

}

void keyboardUpHandler(unsigned char key, int a, int b) {
    keyBuffer[key] = false;
}

void doKeyboardInput(int key, int deltaTime) {
    float cameraSpeed = 0.05f * deltaTime;
    if (key == 27)
        glutExit();
    if (key == 119) { // w
        cameraPos += cameraSpeed * cameraFront;
    }
    if (key == 115) { //s
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (key == 97) { //a
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == 100) { //d
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == 113) {//q
        cameraPos -= cameraSpeed * cameraUp;
    }
    if (key == 101) {//e
        cameraPos += cameraSpeed * cameraUp;
    }
    if (key == 105) {  //i
        pitch += 0.1f * deltaTime;
        CalculateCameraDirection();
    }if (key == 106) { //j
        yaw -= 0.1f * deltaTime;
        CalculateCameraDirection();
    }if (key == 107) { //k
        pitch -= 0.1f * deltaTime;
        CalculateCameraDirection();
    }if (key == 108) { //l
        yaw += 0.1f * deltaTime;
        CalculateCameraDirection();
    }
}


//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------

void InitLight() {
 //For loop for multiple lightsources
    light.position = glm::vec3(4.0, 4.0, 4.0);
    for (int i = 0; i < objectCount; i++) {
       
        materials[i].ambient_color = glm::vec3(0.2, 0.2, 0.1);
        materials[i].diffuse_color = glm::vec3(0.5, 0.5, 0.3);
        materials[i].specular = glm::vec3(0.7, 0.7, 0.7);
        materials[i].power = 1024;
    }
  
}

void Render()
{
    oldTimeSinceStart = timeSinceStart;
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = timeSinceStart - oldTimeSinceStart;
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Do keyboard input
    for (int i = 0; i < (sizeof(keyBuffer) / sizeof(bool)); i++) {
        if (keyBuffer[i]) {
            doKeyboardInput(i, deltaTime);
        }
    }
    //Background color
   /* static const GLfloat blue[] = { 1.0, 0.0, 0.4, 1.0 };
    glClearBufferfv(GL_COLOR, 0, blue);*/
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Do transformation
    model[0] = glm::rotate(model[0], 0.01f, glm::vec3(1.0f, 1.0f, 0.0f));
   
    model[1] = glm::rotate(model[1], 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));
    model[2] = glm::rotate(model[2], 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
    model[4] = glm::translate(model[4], glm::vec3(0.0f, 0.0f, -0.01f));
    
    //view = glm::translate(view, glm::vec3(0.0f, 0.01f, 0.0f));
    //Camera
    // Attach to program_id
    glUseProgram(program_id);

    for (int i = 0; i < objectCount; i++) {
        mv[i] = view * model[i];

        // Send mv
        glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(mv[i]));

        //Bind texture
        glBindTexture(GL_TEXTURE_2D, texture_id[i]);

        //Fill uniform vars for material
        glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(mv[i]));
        glUniform3fv(uniform_material_ambient, 1, glm::value_ptr(materials[i].ambient_color));
        glUniform3fv(uniform_material_diffuse, 1, glm::value_ptr(materials[i].diffuse_color));
        glUniform3fv(uniform_specular, 1, glm::value_ptr(materials[i].specular));
        glUniform1f(uniform_material_power, materials[i].power);

        // Send vao
        glBindVertexArray(vao[i]);
        glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
        glBindVertexArray(0);
   }



   

    glutSwapBuffers();
}


//------------------------------------------------------------
// void Render(int n)
// Render method that is called by the timer function
//------------------------------------------------------------

void Render(int n)
{
    Render();
    glutTimerFunc(DELTA_TIME, Render, 0);
}


//------------------------------------------------------------
// void InitGlutGlew(int argc, char **argv)
// Initializes Glut and Glew
//------------------------------------------------------------

void InitGlutGlew(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Hello OpenGL");
    glutDisplayFunc(Render);
    glutKeyboardFunc(keyboardHandler);
    glutTimerFunc(DELTA_TIME, Render, 0);

    glewInit();
}



void InitLoadObjects() {
    const char* objects[objectCount] = { "teapot.obj", "cylinder18.obj", "tableTest.obj", "plane.obj"};
    const char* textures[objectCount] = { "Textures/uvtemplate.bmp", "Textures/Yellobrk.bmp","Textures/woodTexture.bmp", "Textures/grass.bmp"};
    for (int i = 0; i < objectCount; i++) {
        bool res = loadOBJ(objects[i], vertices[i], uvs[i], normals[i]);
        texture_id[i] = loadBMP(textures[i]);
    }
 
    //bool res = loadOBJ(objects[2], vertices[2], uvs[2], normals[2]);

}


//------------------------------------------------------------
// void InitShaders()
// Initializes the fragmentshader and vertexshader
//------------------------------------------------------------

void InitShaders()
{
    char* vertexshader = glsl::readFile(vertexshader_name);
    GLuint vsh_id = glsl::makeVertexShader(vertexshader);

    char* fragshader = glsl::readFile(fragshader_name);
    GLuint fsh_id = glsl::makeFragmentShader(fragshader);

    program_id = glsl::makeShaderProgram(vsh_id, fsh_id);
}


//------------------------------------------------------------
// void InitMatrices()
//------------------------------------------------------------

void InitMatrices()
{
    //mv for every object
    //model[0] = glm::mat4();
    model[0] = glm::translate(glm::mat4(), glm::vec3(3.0, 1.0, 0.0));
    model[1] = glm::mat4();
    model[2] = glm::translate(glm::mat4(), glm::vec3(-2.0, 1.0, 0.0));
    model[3] = glm::translate(glm::mat4(), glm::vec3(0.0, 1.0, -3.0));
    model[3] = glm::scale(model[3], glm::vec3(10.0, 10.0, 10.0));
    model[4] = glm::translate(glm::mat4(), glm::vec3(0.0, 4.0, 0.0));
    model[4] = glm::rotate(model[4], glm::radians(90.0f), glm::vec3(0.0,1.0,0.0));

    cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    
   
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    //cameraDirection = glm::normalize(cameraPos - cameraTarget);
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    cameraUp = glm::cross(cameraDirection, cameraRight);
    cameraFront = glm::normalize(cameraDirection);
    for (int i = 0; i < objectCount; i++) {
        
        view = glm::lookAt(
            glm::vec3(cameraPos),  // eye
            glm::vec3(cameraFront),  // center
            glm::vec3(cameraUp));  // up
        projection = glm::perspective(
            glm::radians(45.0f),
            1.0f * WIDTH / HEIGHT, 0.1f,
            1000.0f);
        mv[i] =  view * model[i];
    }
   
}


//------------------------------------------------------------
// void InitBuffers()
// Allocates and fills buffers
//------------------------------------------------------------


void InitBuffers()
{
    GLuint position_id, normal_id;
    GLuint vbo_vertices;
    GLuint vbo_normals;
    GLuint vbo_uvs;
   
    // Get vertex attributes
    position_id = glGetAttribLocation(program_id, "position");
    //color_id = glGetAttribLocation(program_id, "color");
    normal_id = glGetAttribLocation(program_id, "normal");
    GLuint uv_id = glGetAttribLocation(program_id, "uv");
    
    for (int i = 0; i < objectCount; i++) {
        // vbo for uvs
        glGenBuffers(1, &vbo_uvs);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
        glBufferData(GL_ARRAY_BUFFER, uvs[i].size() * sizeof(glm::vec2),
            &uvs[i][0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        // vbo for vertices
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER,
            vertices[i].size() * sizeof(glm::vec3), &vertices[i][0],
            GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // vbo for normals
        glGenBuffers(1, &vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        glBufferData(GL_ARRAY_BUFFER,
            normals[i].size() * sizeof(glm::vec3),
            &normals[i][0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



        // Allocate memory for vao
        glGenVertexArrays(1, &vao[i]);

        // Bind to vao
        glBindVertexArray(vao[i]);

        // Bind vertices to vao
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(position_id);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Bind normals to vao
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(normal_id);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Bind to vao
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
        glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(uv_id);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        // Stop bind to vao
        glBindVertexArray(0);
    }

    //// vbo for vertices
    //glGenBuffers(1, &vbo_vertices);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    //glBufferData(GL_ARRAY_BUFFER,
    //    vertices[2].size() * sizeof(glm::vec3), &vertices[2][0],
    //    GL_STATIC_DRAW);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    //// Allocate memory for vao
    //glGenVertexArrays(1, &vao[2]);

    //// Bind to vao
    //glBindVertexArray(vao[2]);
    //
    //// Bind vertices to vao
    //glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    //glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(position_id);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);


    glUseProgram(program_id);

  

    // Make uniform vars
    uniform_mv = glGetUniformLocation(program_id, "mv");
    uniform_proj = glGetUniformLocation(program_id, "projection");
    uniform_light_pos = glGetUniformLocation(program_id, "light_pos");
    uniform_material_ambient = glGetUniformLocation(program_id,
        "mat_ambient");
    uniform_material_diffuse = glGetUniformLocation(program_id,
        "mat_diffuse");
    uniform_specular = glGetUniformLocation(
        program_id, "mat_specular");
    uniform_material_power = glGetUniformLocation(
        program_id, "mat_power");
    
    //Fill uniform vars
    glUniform3fv(uniform_light_pos, 1, glm::value_ptr(light.position));
    glUniformMatrix4fv(uniform_proj, 1,GL_FALSE, glm::value_ptr(projection));
    
}






int main(int argc, char** argv)
{
    InitGlutGlew(argc, argv);
    InitLight();
    InitShaders();
    InitMatrices();
    InitLoadObjects();
    InitBuffers();
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutKeyboardUpFunc(keyboardUpHandler);

    // Hide console window
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    // Main loop
    glutMainLoop();

    return 0;
}
