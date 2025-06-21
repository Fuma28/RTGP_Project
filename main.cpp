// Std. Includes
#include <string>
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// dimensions of application's window
GLuint screenWidth = 1920, screenHeight = 1080;

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();
// if one of the arrow keys is pressed, we move the active light in the corresponding direction
void apply_light_movements(int activeLight);
void addLight();
void removeLight();

// setup of Shader Programs for the 5 shaders used in the application
void SetupShaders();
// delete Shader Programs whan application ends
void DeleteShaders();
// load image from disk and create an OpenGL texture
GLint LoadTexture(const char* path);

// we initialize an array of booleans for each keyboard key
bool keys[1024];
// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;
// define if the mouse movement  will cause camer movement or not. It allows to have camera movement and GUI interaction together. 
bool moveMode = false;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat elapsedTime = 0.0f;
unsigned int frameCounter = 0;

// rotation angle on Y axis
GLfloat orientationY = 1.0f;
// rotation speed on Y axis
GLfloat spin_speed = 0.5f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;
// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{ PLAIN, BUMP, NORMAL, PARALLAX, DISPLACEMENT, LIGHT };
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "PLAIN", "BUMP", "NORMAL", "PARALLAX", "DISPLACEMENT", "LIGHT"};

// index of the current shader (= 0 in the beginning)
GLint current_program = 0;
// a vector for all the Shader Programs used and swapped in the application
vector<Shader> shaders;

Camera camera(glm::vec3(0.0f, 1.8f, 5.0f), GL_TRUE);

// Uniforms to be passed to shaders
// pointlights positions
vector<glm::vec3> lightPositions = {glm::vec3(0.0f, 0.0f, 0.0f)};
GLuint nLights = 1;
GLint activeLight = 0;
const char * LightsNames[] = { "Light 1", "Light 2", "Light 3", "Light 4", "Light 5"};

// specular and ambient components
GLfloat specularColor[3] = {1.0,1.0,1.0};
GLfloat ambientColor[3] = {0.1,0.1,0.1};
// weights for the diffusive, specular and ambient components
GLfloat Kd = 0.65f;
GLfloat Ks = 0.1f;
GLfloat Ka = 0.05f;
// shininess coefficient for Blinn-Phong shader
GLfloat shininess = 32.0f;

GLfloat height_scale = 1.5f;
// UV repetitions
GLint repeat = 1;

const char * available_textures[] = { "cobble", "brick wall", "sofa"};
GLint current_texture = 0;

// vector for the textures IDs
vector<GLint> textureID;

bool tessellation = false;

/////////////////// MAIN function ///////////////////////
int main()
{
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    // It is possible to raise the values, in order to use functionalities of more recent OpenGL specs.
    // If not supported by your graphics HW, the context will not be created and the application will close
    // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
    // in relation also to the values indicated in these GLFW commands
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Bump Mapping", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create the Shader Programs used in the application
    SetupShaders();

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model planeModel("../../models/plane.obj");
    Model sphereModel("../../models/sphere.obj");
    Model potModel("../../models/pot.obj");

    // we load the images and store them in a vector
    //cobble
    textureID.push_back(LoadTexture("../../textures/cobble/diffuse.png"));
    textureID.push_back(LoadTexture("../../textures/cobble/normal.png"));
    textureID.push_back(LoadTexture("../../textures/cobble/height.png"));
    //brick wall
    textureID.push_back(LoadTexture("../../textures/bw/diffuse.png"));
    textureID.push_back(LoadTexture("../../textures/bw/normal.png"));
    textureID.push_back(LoadTexture("../../textures/bw/height.png"));
    //sofa
    textureID.push_back(LoadTexture("../../textures/sofa/diffuse.jpg"));
    textureID.push_back(LoadTexture("../../textures/sofa/normal.jpg"));
    textureID.push_back(LoadTexture("../../textures/sofa/height.jpg"));

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix: the camera moves, so we just set to indentity now
    glm::mat4 view = glm::mat4(1.0f);

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 potModelMatrix = glm::mat4(1.0f);
    glm::mat3 potNormalMatrix = glm::mat3(1.0f);

    //set that we work with triangular texels
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    //set up IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    glfwSwapInterval(0);

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameCounter++;
        elapsedTime+=deltaTime;

        //every second we update the fps
        if(elapsedTime>=1){
            std::string FPS = std::to_string(frameCounter/elapsedTime);
            frameCounter = 0;
            elapsedTime = 0.0f;
            std::string newTitle = "Bump Mapping - " + FPS + " fps";
            glfwSetWindowTitle(window, newTitle.c_str());
        }

        // Check is an I/O event is happening
        glfwPollEvents();
        apply_camera_movements();
        apply_light_movements(activeLight);

        view = camera.GetViewMatrix();

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, then we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        // We "install" the selected Shader Program as part of the current rendering process
        shaders[current_program].Use();

        GLint diffuseMapLocation = glGetUniformLocation(shaders[current_program].Program, "diffuseMap");
        GLint normalMapLocation = glGetUniformLocation(shaders[current_program].Program, "normalMap");
        GLint heightMapLocation = glGetUniformLocation(shaders[current_program].Program, "heightMap");
        GLint repeatLocation = glGetUniformLocation(shaders[current_program].Program, "repeat");
        GLint matAmbientLocation = glGetUniformLocation(shaders[current_program].Program, "ambientColor");
        GLint matSpecularLocation = glGetUniformLocation(shaders[current_program].Program, "specularColor");
        GLint kaLocation = glGetUniformLocation(shaders[current_program].Program, "Ka");
        GLint kdLocation = glGetUniformLocation(shaders[current_program].Program, "Kd");
        GLint ksLocation = glGetUniformLocation(shaders[current_program].Program, "Ks");
        GLint shineLocation = glGetUniformLocation(shaders[current_program].Program, "shininess");
        GLint numFacesLocation = glGetUniformLocation(shaders[current_program].Program, "numFaces");
        
        // we assign the value to the uniform variables
        glUniform3fv(matAmbientLocation, 1, ambientColor);
        glUniform3fv(matSpecularLocation, 1, specularColor);
        glUniform1f(shineLocation, shininess);
        glUniform1f(kaLocation, Ka);
        glUniform1f(kdLocation, Kd);
        glUniform1f(ksLocation, Ks);
        glUniform1i(diffuseMapLocation, 0);
        glUniform1i(normalMapLocation, 1);
        glUniform1i(heightMapLocation, 2);
        glUniform1i(repeatLocation, repeat);

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        // we pass light position to the shader
        for (GLuint i = 0; i < nLights; i++)
        {
            string number = to_string(i);
            glUniform3fv(glGetUniformLocation(shaders[current_program].Program, ("pointLightPosition[" + number + "]").c_str()), 1, glm::value_ptr(lightPositions[i]));
        }
        glUniform1i(glGetUniformLocation(shaders[current_program].Program, "nLights"), nLights);
        glUniform3fv(glGetUniformLocation(shaders[current_program].Program,"viewPosition"), 1, glm::value_ptr(camera.Position));

        //diffuseMap
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID[3*current_texture]);
        //normalMap
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureID[3*current_texture+1]);
        //heightMap
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureID[3*current_texture+2]);        

        //if we are using the displacement shader we pass the height scale variable and activate tessellation
        if(current_program == 4){
            GLint heightScaleLocation = glGetUniformLocation(shaders[current_program].Program, "height_scale");
            glUniform1f(heightScaleLocation, height_scale);
            tessellation = true;
        }else{
            tessellation = false;
        }

        //PLANE
        glUniform1i(numFacesLocation, planeModel.numFaces());
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, 0.0f, -10.0f));
        planeModelMatrix = glm::rotate(planeModelMatrix, orientationY, glm::vec3(0.0f, 1.0f, 0.0f));
        planeModelMatrix = glm::rotate(planeModelMatrix,  glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        planeModel.Draw(tessellation);

        //POT
        glUniform1i(numFacesLocation, potModel.numFaces());
        potModelMatrix = glm::mat4(1.0f);
        potNormalMatrix = glm::mat3(1.0f);
        potModelMatrix = glm::translate(potModelMatrix, glm::vec3(10.0f, 0.0f, -10.0f));
        potModelMatrix = glm::rotate(potModelMatrix, orientationY, glm::vec3(0.0f, 1.0f, 0.0f));
        potModelMatrix = glm::scale(potModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
        potNormalMatrix = glm::inverseTranspose(glm::mat3(potModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(potModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(potNormalMatrix));
        potModel.Draw(tessellation);

        //SPHERE
        glUniform1i(numFacesLocation, sphereModel.numFaces());
        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);
        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-10.0f, 0.0f, -10.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, orientationY, glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(sphereModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));
        sphereModel.Draw(tessellation);
        
        //LIGHTS
        shaders[LIGHT].Use();
        glUniformMatrix4fv(glGetUniformLocation(shaders[LIGHT].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[LIGHT].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        for(int i=0; i<nLights; i++){
            sphereModelMatrix = glm::mat4(1.0f);
            sphereNormalMatrix = glm::mat3(1.0f);
            sphereModelMatrix = glm::translate(sphereModelMatrix, lightPositions[i]);
            sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.2f));
            sphereNormalMatrix = glm::inverseTranspose(glm::mat3(sphereModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(shaders[LIGHT].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
            glUniformMatrix3fv(glGetUniformLocation(shaders[LIGHT].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));
            sphereModel.Draw(false);
        }
        

        //IMGUI Control panels definition
        ImGui::Begin("Blinn-Phong parameters");
        ImGui::SliderFloat("Kd", &Kd, 0, 1);
        ImGui::SliderFloat("Ks", &Ks, 0, 1);
        ImGui::SliderFloat("Ka", &Ka, 0, 1);
        ImGui::SliderFloat("Shininess", &shininess, 0, 128);
        ImGui::ColorEdit3("Specular color", specularColor);
        ImGui::ColorEdit3("Ambient  color", ambientColor);
        ImGui::End();

        ImGui::Begin("Objects appearance");
        ImGui::SliderInt("Repeat", &repeat, 1, 5);
        ImGui::Combo("Shader", &current_program, print_available_ShaderPrograms, IM_ARRAYSIZE(print_available_ShaderPrograms));
        if(current_program==4){
            ImGui::SliderFloat("Height scale", &height_scale, 0, 3);
        }
        ImGui::Combo("Texture", &current_texture, available_textures, IM_ARRAYSIZE(available_textures));
        ImGui::SliderFloat("Spin speed", &spin_speed, 0, 10);
        ImGui::End();

        ImGui::Begin("Light panel");
        if (ImGui::Button("addLight"))
            addLight();
        if (ImGui::Button("removeLight"))
            removeLight();
        for(int i=0; i<nLights; i++){
            ImGui::PushID(i);
            ImGui::RadioButton(LightsNames[i], &activeLight, i); ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    //IMGUI cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShaders();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// we create and compile shaders (code of Shader class is in include/utils/shader.h), and we add them to the list of available shaders
void SetupShaders()
{
    Shader shader1("shaders/basic.vert", "shaders/basic.frag");
    shaders.push_back(shader1);
    Shader shader2("shaders/blinn_bump.vert", "shaders/blinn_bump.frag");
    shaders.push_back(shader2);
    Shader shader3("shaders/tangent.vert", "shaders/normal.frag");
    shaders.push_back(shader3);
    Shader shader4("shaders/tangent.vert", "shaders/parallax.frag");
    shaders.push_back(shader4);
    Shader shader5("shaders/displacement.vert", "shaders/displacement.frag", "shaders/displacement.tcs", "shaders/displacement.tes");
    shaders.push_back(shader5);
    Shader shader6("shaders/light.vert", "shaders/light.frag");
    shaders.push_back(shader6);
}

//////////////////////////////////////////
// we delete all the Shaders Programs
void DeleteShaders()
{
    for(GLuint i = 0; i < shaders.size(); i++)
        shaders[i].Delete();
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{   
    GLuint new_program;
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
       moveMode = true;
    if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
       moveMode = false;

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    stbi_set_flip_vertically_on_load(1);
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);
    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;
    if(channels == 4){
        stbi_image_free(image);
        image = stbi_load(path, &w, &h, &channels, STBI_rgb_alpha);
    }
    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if (channels==4){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;

}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    
    // we move the camera view following the mouse cursor
    // we calculate the offset of the mouse cursor from the position in the last frame
    // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // offset of mouse cursor position
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    // the new position will be the previous one for the next frame
    lastX = xpos;
    lastY = ypos;
    
    if(moveMode){
        // we pass the offset to the Camera class instance in order to update the rendering
        camera.ProcessMouseMovement(xoffset, yoffset);
    }

}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
    // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
    // then the camera will apply a compensation factor to the velocities applied in the single directions, 
    // in order to have the full v applied in the diagonal direction  
    // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.  
    GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
    camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);  
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void apply_light_movements(int activeLight)
{
    GLboolean diagonal_movement = (keys[GLFW_KEY_UP] ^ keys[GLFW_KEY_DOWN]) && (keys[GLFW_KEY_LEFT] ^ keys[GLFW_KEY_RIGHT]) && (keys[GLFW_KEY_PAGE_UP] ^ keys[GLFW_KEY_PAGE_DOWN]); 
    GLfloat movementCompensation = (diagonal_movement ? DIAGONAL_COMPENSATION : 1.0f);
    GLfloat movementSpeed = 5.0f;
    GLfloat velocity = movementSpeed * deltaTime * movementCompensation;
    if(keys[GLFW_KEY_UP])
        lightPositions[activeLight] += camera.Front * velocity;
    if(keys[GLFW_KEY_DOWN])
        lightPositions[activeLight] -= camera.Front * velocity;
    if(keys[GLFW_KEY_RIGHT])
        lightPositions[activeLight] += camera.Right * velocity;
    if(keys[GLFW_KEY_LEFT])
        lightPositions[activeLight] -= camera.Right * velocity;
    if(keys[GLFW_KEY_PAGE_UP])
        lightPositions[activeLight] += camera.Up * velocity;
    if(keys[GLFW_KEY_PAGE_DOWN])
        lightPositions[activeLight] -= camera.Up * velocity;
        
}

void addLight(){
    if (nLights<5){
        lightPositions.push_back(glm::vec3(0.0, 0.0, 0.0));
        nLights++;
        activeLight = nLights - 1;
    }else{
        std::cout << "Max number of lights reached" << std::endl;
    }
}

void removeLight(){
    if(nLights>0){
        lightPositions.pop_back();
        nLights--;
    }
    if(activeLight>=nLights){
        activeLight = nLights - 1;
    }
}
    