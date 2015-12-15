
///////////////////////////////// includes ///////////////////////////////////
#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

// use floats and med precision operations
#define GLM_PRECISION_MEDIUMP_FLOAT
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
#include "utils.hpp"
#include <algorithm>

#include <cstdlib>
#include <iostream>
#include <array>
// use gl definitions from glbinding
using namespace gl;

/////////////////////////// variable definitions //////////////////////////////
// vertical field of view of camera
const float camera_fov = glm::radians(60.0f);
// initial window dimensions
const unsigned window_width = 800;
const unsigned window_height = 800;
// the rendering window
GLFWwindow* window;

// variables for fps computation
double last_second_time = 0;
unsigned frames_per_second = 0;

//Generate Random Numbers
int nStars = 2000;

std::vector<GLfloat> stars{};


// the main shader program
GLuint simple_program = 0;
GLuint stars_program = 0;
GLuint screen_program = 0;
// cpu representation of model
model sun_model{};
model moon_model{};
model star_model{};
model planet_model{};


// holds gpu representation of model
struct model_object {
    GLuint vertex_AO = 0;
    GLuint vertex_BO = 0;
    GLuint element_BO = 0;
};



// creating planets
model_object sun_object, moon_object, star_object, planet_object, planet_object2, planet_object3, planet_object4,planet_object5,planet_object6,planet_object7,planet_object8, screen_quad;


//putting planet_objects in array
model_object planets[8] = {planet_object,planet_object2,planet_object3,planet_object4,planet_object5,planet_object6,planet_object7,planet_object8};


//texture struct to store textures

GLuint texture_object0, texture_object1, texture_object2, texture_object3, texture_object3_1, texture_object4, texture_object5, texture_object6, texture_object7, texture_object8, texture_objectU, buffer_texture_object;
texture texture0;
texture texture1;
texture texture2;
texture texture3;
texture texture3_1;
texture texture4;
texture texture5;
texture texture6;
texture texture7;
texture texture8;
texture textureU;

// camera matrices
glm::mat4 camera_transform = glm::translate(glm::mat4{}, glm::vec3{0.0f, 0.0f, 40.0f});
glm::mat4 camera_projection{1.0f};

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;

GLint location_star_view_matrix = -1;
GLint location_star_projection_matrix = -1;

GLint location_color = -1;
GLint location_light = -1;

GLuint rb_handle;
GLuint fbo_handle;

// path to the resource folders
std::string resource_path{};

/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_projection(GLFWwindow* window, int width, int height);
void update_view();
void update_uniform_locations();
void update_uniform_star_locations();
void update_shader_programs(bool throwing = false);
void update_starshaders();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_geometry();
void renderPlanetSystem();
void renderStars();
void show_fps();
void render();
void renderBuffer();
void loadTextures();
void initializeTextures();
void createBuffer();
void initializeTextureQuad();

float RandomFloat(float a, float b);
/////////////////////////////// main function /////////////////////////////////
int main(int argc, char* argv[]) {
    
    glfwSetErrorCallback(utils::glsl_error);
    
    if(!glfwInit()) {
        std::exit(EXIT_FAILURE);
    }
    
    // set OGL version explicitly
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // create window, if unsuccessfull, quit
    window = glfwCreateWindow(window_width, window_height, "OpenGL Framework", NULL, NULL);
    if(!window) {
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    
    // use the windows context
    glfwMakeContextCurrent(window);
    // disable vsync
    glfwSwapInterval(0);
    // register key input function
    glfwSetKeyCallback(window, key_callback);
    // allow free mouse movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // register resizing function
    glfwSetFramebufferSizeCallback(window, update_projection);
    
    
    // initialize glindings in this context
    glbinding::Binding::initialize();
    
    // activate error checking after each gl function call
    utils::watch_gl_errors();
    
    //first argument is resource path
    if (argc > 1) {
        resource_path = argv[1];
    }
    // no resource path specified, use default
    else {
        std::string exe_path{argv[0]};
        resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
        resource_path += "/../../resources/";
    }
    
    
    
    // do before framebuffer_resize call as it requires the projection uniform location
    stars.resize(nStars*4);
    
    
    std::generate(stars.begin(), stars.end() , [&]{return RandomFloat(-100.0f, 100.0f);});
    //std::generate(stars.begin(), stars.end() , rand() % 100 + 1);
    for (auto elem : stars) {
        //        std::cout << elem << " ";
    }
    
    update_shader_programs(true);
    update_starshaders();
    
    // upload view uniforms to new shader
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_projection(window, width, height);
    update_view();
    
    
    
    
    
    // set up models
    star_model = model{stars, model::POSITION|model::NORMAL};
    initialize_geometry();
    initializeTextureQuad();

    
    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // rendering loop
    while(!glfwWindowShouldClose(window)) {
        // query input
        glfwPollEvents();
        // clear buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw geometry
        render();
        // swap draw buffer to front
        glfwSwapBuffers(window);
        // display fps
        show_fps();
    }
    
    quit(EXIT_SUCCESS);
}

/////////////////////////////////////////////// initialisation functions /////////////////////////////////////////////////


void initialize_geometry() {
    
    
    planet_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);
    
    
    // generate vertex array object
    glGenVertexArrays(1, &planet_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(planet_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &planet_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
    
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);
    
    // generate generic buffer
    glGenBuffers(1, &planet_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);
    
    
    
    ////////initialize Star object////////
    
    
    // generate vertex array object
    glGenVertexArrays(1, &star_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(star_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &star_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::NORMAL]);
    
    
    
    
    //initialize the texture
    initializeTextures();
    
    
    
    
}

/////////////////////////////////////////////////////// render functions /////////////////////////////////////////////////////////////
// render model
void render() {
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
    renderPlanetSystem();
    renderStars();
    
    glBindFramebuffer(GL_FRAMEBUFFER, rb_handle);
    renderBuffer();
    
    
}


void renderBuffer(){
    
    glUseProgram(screen_program);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buffer_texture_object);
    glUniform1i(glGetUniformLocation(screen_program, "texSampler"), 0);
    
    glBindVertexArray(buffer_texture_object);
    utils::validate_program(screen_program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    
}

void initializeTextureQuad(){
    
    std::vector<GLfloat> vertices {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,1.0f, -1.0f, 0.0f, 1.0f, 0.0f,-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    
    glGenVertexArrays(1, &screen_quad.vertex_AO);
    glBindVertexArray(screen_quad.vertex_AO);
    glGenBuffers(1, &screen_quad.vertex_AO);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad.vertex_BO);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(GLsizei(sizeof(float) * vertices.size())), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    int offset_a = 0 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(5 * sizeof(GLfloat)), (const GLvoid*) offset_a);
    
    glEnableVertexAttribArray(1);
    int offset_b = 3 * sizeof(GLfloat);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GLsizei(5 * sizeof(GLfloat)), (const GLvoid*) offset_b);

}
///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void update_projection(GLFWwindow* window, int width, int height) {
    

    // resize framebuffer
    glViewport(0, 0, width, height);
    createBuffer();

    float aspect = float(width) / float(height);
    float fov_y = camera_fov;
    // if width is smaller, extend vertical fov
    if(width < height) {
        fov_y = 2.0f * glm::atan(glm::tan(camera_fov * 0.5f) * (1.0f / aspect));
    }
    // projection is hor+
    camera_projection = glm::perspective(fov_y, aspect, 0.1f, 1000.0f);
    glm::mat4 inv_camera_view = glm::inverse(camera_transform);
    // upload matrix to gpu
    glUseProgram(simple_program);
    
    glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
    glUseProgram(stars_program);
    glUniformMatrix4fv(location_star_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
    
    glUseProgram(screen_program);
    glUniformMatrix4fv(glGetUniformLocation(screen_program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera_projection));
    glUniform2f(glGetUniformLocation(simple_program, "size"), GLfloat(width), GLfloat(height));
    
}

// update camera transformation
void update_view() {
    
    // vertices are transformed in camera space, so camera transform must be inverted
    glm::mat4 inv_camera_view = glm::inverse(camera_transform);
    // upload matrix to gpu
    glUseProgram(simple_program);
    glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));
    glUniform3fv(location_light, 1, glm::value_ptr(glm::mat4(0.0f)));
    glUseProgram(stars_program);
    glUniformMatrix4fv(location_star_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));
    
    
}


// load shaders and update uniform locations
void update_shader_programs(bool throwing) {
    auto update_lambda = [&](){
        // throws exception when compiling was unsuccessfull
        GLuint new_program = shader_loader::program(resource_path + "shaders/simpleBlinn.vert",
                                                    resource_path + "shaders/simpleBlinn.frag");
        
        //      GLuint new_program = [self compileShader:@ "shaders/simpleBlinn.vert" withType:GL_VERTEX_SHADER];
        //      GLuint new_program = [self compileShader:@ "shaders/simpleBlinn.frag" withType:GL_FRAGMENT_SHADER];
        
        // free old shader
        glDeleteProgram(simple_program);
        // save new shader
        simple_program = new_program;
        // bind shader
        glUseProgram(simple_program);
        // after shader is recompiled uniform locations may change
        update_uniform_locations();
        
        
        //loadTextures whenever shader is updated
        
        loadTextures();
        
    };
    if (throwing) {
        update_lambda();
    }
    else {
        try {
            update_lambda();
        }
        catch(std::exception&) {
            // dont crash, allow another try
        }
    }}

void loadTextures() {
    
    texture1 = texture_loader::file(resource_path + "textures/1.png");
    texture2 = texture_loader::file(resource_path + "textures/2.png");
    texture3 = texture_loader::file(resource_path + "textures/3.png");
    texture4 = texture_loader::file(resource_path + "textures/4.png");
    texture5 = texture_loader::file(resource_path + "textures/5.png");
    texture6 = texture_loader::file(resource_path + "textures/6.png");
    texture7 = texture_loader::file(resource_path + "textures/7.png");
    texture8 = texture_loader::file(resource_path + "textures/8.png");
    texture0 = texture_loader::file(resource_path + "textures/0.png");
    texture3_1 = texture_loader::file(resource_path + "textures/3-1.png");
    textureU = texture_loader::file(resource_path + "textures/9.png");
    
    
}

void createBuffer(){
    try {
        
        // throws exception when compiling was unsuccessfull
        GLuint new_screen_program = shader_loader::program(resource_path + "shaders/simpleBlinn.vert", resource_path + "shaders/simpleBlinn.frag");
        
        // free old shader
        glDeleteProgram(screen_program);
        // save new shader
        screen_program = new_screen_program;
        // bind shader
        glUseProgram(screen_program);
        
        // upload view uniforms to new shader
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        update_projection(window, width, height);
        update_view();
    }
    catch(std::exception&) {
        // dont crash, allow another try
    }
    
    
    
    //Renderbuffer
    glGenRenderbuffers(1, &rb_handle);
    glBindRenderbuffer(GL_RENDERBUFFER, rb_handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window_width, window_height);
    
    //texture
    glGenTextures(1, &buffer_texture_object);
    glBindTexture(GL_TEXTURE_2D, buffer_texture_object);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(GL_TEXTURE_2D, 0, GLint(GL_RGBA8), window_width, window_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    
    //Framebuffer
    glGenFramebuffers(1, &fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer_texture_object, 0);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER_EXT, rb_handle);
    
    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("something went wrong :(");
    }
    
    
}

void initializeTextures(){
    //USE GL_RGBA if the textures provide an alpha channel
    //somehow not needed yet:    //
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object0);
    glBindTexture(texture0.target, texture_object0);
    glTexParameteri(texture0.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture0.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture0.target , 0 , GLint(GL_RGBA) , texture0.width , texture0.height , 0 , GL_RGBA , texture0.channel_type , texture0.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object1);
    glBindTexture(texture1.target, texture_object1);
    glTexParameteri(texture1.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture1.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture1.target , 0 , GLint(GL_RGBA) , texture1.width , texture1.height , 0 , GL_RGBA , texture1.channel_type , texture1.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object2);
    glBindTexture(texture2.target, texture_object2);
    glTexParameteri(texture2.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture2.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture2.target , 0 , GLint(GL_RGBA) , texture2.width , texture2.height , 0 , GL_RGBA , texture2.channel_type , texture2.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object3);
    glBindTexture(texture3.target, texture_object3);
    glTexParameteri(texture3.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture3.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture3.target , 0 , GLint(GL_RGBA) , texture3.width , texture3.height , 0 , GL_RGBA , texture3.channel_type , texture3.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object4);
    glBindTexture(texture4.target, texture_object4);
    glTexParameteri(texture4.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture4.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture4.target , 0 , GLint(GL_RGBA) , texture4.width , texture4.height , 0 , GL_RGBA , texture4.channel_type , texture4.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object5);
    glBindTexture(texture5.target, texture_object5);
    glTexParameteri(texture5.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture5.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture5.target , 0 , GLint(GL_RGBA) , texture5.width , texture5.height , 0 , GL_RGBA , texture5.channel_type , texture5.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object6);
    glBindTexture(texture6.target, texture_object6);
    glTexParameteri(texture6.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture6.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture6.target , 0 , GLint(GL_RGBA) , texture6.width , texture6.height , 0 , GL_RGBA , texture6.channel_type , texture6.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object7);
    glBindTexture(texture7.target, texture_object7);
    glTexParameteri(texture7.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture7.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture7.target , 0 , GLint(GL_RGBA) , texture7.width , texture7.height , 0 , GL_RGBA , texture7.channel_type , texture7.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object8);
    glBindTexture(texture8.target, texture_object8);
    glTexParameteri(texture8.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture8.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture8.target , 0 , GLint(GL_RGBA) , texture8.width , texture8.height , 0 , GL_RGBA , texture8.channel_type , texture8.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_objectU);
    glBindTexture(textureU.target, texture_objectU);
    glTexParameteri(textureU.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(textureU.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(textureU.target , 0 , GLint(GL_RGBA) , textureU.width , textureU.height , 0 , GL_RGBA , textureU.channel_type , textureU.data.data());
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_object3_1);
    glBindTexture(texture3_1.target, texture_object3_1);
    glTexParameteri(texture3_1.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(texture3_1.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(texture3_1.target , 0 , GLint(GL_RGBA) , texture3_1.width , texture3_1.height , 0 , GL_RGBA , texture3_1.channel_type , texture3_1.data.data());
    
    
    
    
    
}

void update_starshaders() {
    try {
        
        // throws exception when compiling was unsuccessfull
        GLuint new_stars_program = shader_loader::program(resource_path + "shaders/stars.vert", resource_path + "shaders/stars.frag");
        
        // free old shader
        glDeleteProgram(stars_program);
        // save new shader
        stars_program = new_stars_program;
        // bind shader
        glUseProgram(stars_program);
        update_uniform_star_locations();
        
        // upload view uniforms to new shader
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        update_projection(window, width, height);
        update_view();
    }
    catch(std::exception&) {
        // dont crash, allow another try
    }
}

void renderStars(){
    
    glUseProgram(stars_program);
    glBindVertexArray(star_object.vertex_AO);
    
    glDrawArrays(gl::GL_POINTS, 0, star_model.vertex_num);
}

void renderPlanetSystem(){
    glUseProgram(simple_program);
    
    glBindVertexArray(planet_object.vertex_AO);
    
    
    //skybox is rendered
    glm::mat4 sky_matrix = glm::translate(glm::mat4{}, glm::vec3{0.0f, 0.0f, 0.0f});
    sky_matrix = glm::scale(sky_matrix, glm::vec3{150.0f, 150.0f, 150.0f});
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(sky_matrix));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_sky_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * sky_matrix);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_sky_matrix));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textureU.target, texture_objectU);
    glUniform1i(glGetUniformLocation(simple_program, "texSampler"), 0);
    
    glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);
    
    //sun is rendered
    glm::mat4 model_matrix = glm::translate(glm::mat4{}, glm::vec3{0.0f, 0.0f, 0.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3{2.0f, 2.0f, 2.0f});
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    
    //send colorInformation of Sun to Shader
    glm::vec3 sunColor = glm::vec3{ 1.0f,0.8f ,0.6f};
    glUniform3fv(location_color, 1, glm::value_ptr(sunColor));
    
    
    //send position Information of Sun to Shader
    //TODO: use different lightning for sun
    glm::vec4 sun_position = ( glm::inverse(camera_transform) * model_matrix) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
    glUniform3fv(location_light, 1, glm::value_ptr(sun_position));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture0.target, texture_object0);
    glUniform1i(glGetUniformLocation(simple_program, "texSampler"), 0);
    
    glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);
    
    
    //earth treated differently
    glm::mat4 earth_matrix = glm::rotate(glm::mat4{}, float(glfwGetTime()+3), glm::vec3{0.0f, 1.0f, 0.0f});
    earth_matrix = glm::translate(earth_matrix, glm::vec3{4.0f +4*3, 0.0f, -1.0f});
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(earth_matrix));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_earth_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * earth_matrix);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_earth_matrix));
    
    glm::vec3 eartColor = glm::vec3{ 0.6f,0.7f ,1.0f  };
    glUniform3fv(location_color, 1, glm::value_ptr(eartColor));
    
    //textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture3.target, texture_object3);
    glUniform1i(glGetUniformLocation(simple_program, "texSampler"), 0);
    
    // draw bound vertex array as triangles using bound shader
    glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);
    
    
    
    
    
    //moon is rendered
    glm::mat4 model_matrix2;
    
    //moon is circulating around earth_matrix
    model_matrix2 = glm::rotate(earth_matrix, float(glfwGetTime()), glm::vec3{0.0f, 1.0f, 0.0f});
    model_matrix2 = glm::translate(model_matrix2, glm::vec3{2.0f, 0.0f, 0.0f});
    model_matrix2 = glm::scale(model_matrix2, glm::vec3{0.5f, 0.5f, 0.5f});
    
    
    
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix2));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_matrix2 = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix2);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix2));
    
    
    glm::vec3 moonColor = glm::vec3{ 0.9f, 0.9f, 0.8f };
    glUniform3fv(location_color, 1, glm::value_ptr(moonColor));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture3_1.target, texture_object3_1);
    glUniform1i(glGetUniformLocation(simple_program, "texSampler"), 0);
    glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);
    
    
    
    //planetes except earth are rendered
    for (int i = 1; i < 8; i = i + 1){
        if (i != 3){
            
            
            
            
            
            glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(glfwGetTime()+i), glm::vec3{0.0f, 1.0f, 0.0f});
            model_matrix = glm::translate(model_matrix, glm::vec3{4.0f +4*i, 0.0f, -1.0f});
            glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
            
            // extra matrix for normal transformation to keep them orthogonal to surface
            glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);
            glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
            
            float rColor = 0.0f;
            float gColor = 0.0f;
            float bColor = 0.0f;
            switch (i) {
                    
                case 1:
                    rColor = 0.9f;
                    gColor = 0.9f;
                    bColor = 0.5f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture1.target, texture_object1);
                    break;
                case 2:
                    rColor = 0.7f;
                    gColor = 0.8f;
                    bColor = 0.5f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture2.target, texture_object2);
                    break;
                    
                    // no case 3 because that's how we did it
                    
                case 4:
                    rColor = 0.7f;
                    gColor = 0.8f;
                    bColor = 0.6f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture4.target, texture_object4);
                    break;
                case 5:
                    rColor = 0.6f;
                    gColor = 0.5f;
                    bColor = 0.6f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture5.target, texture_object5);
                    break;
                case 6:
                    rColor = 0.5f;
                    gColor = 0.2f;
                    bColor = 0.4f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture6.target, texture_object6);
                    break;
                case 7:
                    rColor = 0.4f;
                    gColor = 0.4f;
                    bColor = 0.2f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture7.target, texture_object7);
                    break;
                case 8:
                    rColor = 0.3f;
                    gColor = 0.5f;
                    bColor = 0.5f;
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture8.target, texture_object8);
                    break;
                default:
                    rColor = 0.5f;
            }
            
            glm::vec3 planetColor = glm::vec3{ rColor,gColor ,bColor   };
            glUniform3fv(location_color, 1, glm::value_ptr(planetColor));
            
            glUseProgram(simple_program);
            
            //render texture
            glUniform1i(glGetUniformLocation(simple_program, "texSampler"), 0);
            
            
            glBindVertexArray(planet_object.vertex_AO);
            utils::validate_program(simple_program);
            
            
            
            // draw bound vertex array as triangles using bound shader
            glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);
        }
    }
    
}

void update_uniform_star_locations() {
    //    location_normal_matrix = glGetUniformLocation(stars_program, "NormalMatrix");
    //    location_model_matrix = glGetUniformLocation(stars_program, "ModelMatrix");
    //
    location_star_view_matrix = glGetUniformLocation(stars_program, "ViewMatrix");
    location_star_projection_matrix = glGetUniformLocation(stars_program, "ProjectionMatrix");
}

// update shader uniform locations
void update_uniform_locations() {
    location_normal_matrix = glGetUniformLocation(simple_program, "NormalMatrix");
    location_model_matrix = glGetUniformLocation(simple_program, "ModelMatrix");
    location_view_matrix = glGetUniformLocation(simple_program, "ViewMatrix");
    location_projection_matrix = glGetUniformLocation(simple_program, "ProjectionMatrix");
    
    location_color = glGetUniformLocation(simple_program, "shaderColor");
    location_light = glGetUniformLocation(simple_program, "lightPos");
}

///////////////////////////// misc functions ////////////////////////////////
// handle key input
//Use Q and E for vertical movement of the camera
//Use A and D for horizontal movement of the camera
//Use UP and DOWN for horizontal rotation of the camera
//Use W and S for movement in the depth of the camera
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
    else if(key == GLFW_KEY_R && action == GLFW_PRESS) {
        // upload view uniforms to new shader
        update_shader_programs();
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        update_projection(window, width, height);
        update_view();
        
    }
    else if(key == GLFW_KEY_W && action == GLFW_PRESS) {
        camera_transform = glm::translate(camera_transform, glm::vec3{0.0f, 0.0f, -10.0f});
        update_view();
    }
    else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
        camera_transform = glm::translate(camera_transform, glm::vec3{0.0f, 0.0f, 10.0f});
        update_view();
    }
    else if(key == GLFW_KEY_Q && action == GLFW_PRESS){
        camera_transform = glm::translate(camera_transform, glm::vec3{0.0f, 2.0f, 0.0f});
        update_view();
    }
    else if(key == GLFW_KEY_E && action == GLFW_PRESS){
        camera_transform = glm::translate(camera_transform, glm::vec3{0.0f, -2.0f, 0.0f});
        update_view();
    }
    else if(key == GLFW_KEY_A && action == GLFW_PRESS){
        camera_transform = glm::translate(camera_transform, glm::vec3{-2.0f, 0.0f, 0.0f});
        update_view();
    }
    else if(key == GLFW_KEY_D && action == GLFW_PRESS){
        camera_transform = glm::translate(camera_transform, glm::vec3{2.0f, 0.0f, 0.0f});
        update_view();
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        camera_transform = glm::rotate(camera_transform, float(0.1), glm::vec3{1.0f, 0.0f, 0.0f});
        update_view();
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS){
        camera_transform = glm::rotate(camera_transform, float(-0.1), glm::vec3{1.0f, 0.0f, 0.0f});
        update_view();
    }
}

// calculate fps and show in window title
void show_fps() {
    ++frames_per_second;
    double current_time = glfwGetTime();
    if(current_time - last_second_time >= 1.0) {
        std::string title{"OpenGL Framework - "};
        title += std::to_string(frames_per_second) + " fps";
        
        glfwSetWindowTitle(window, title.c_str());
        frames_per_second = 0;
        last_second_time = current_time;
    }
}

void quit(int status) {
    // free opengl resources
    glDeleteProgram(simple_program);
    glDeleteProgram(stars_program);
    glDeleteProgram(screen_program);
    
    for (int i = 0; i < 8; i = i + 1){
        
        glDeleteBuffers(1, &planets[i].vertex_BO);
        glDeleteVertexArrays(1, &planets[i].element_BO);
        glDeleteVertexArrays(1, &planets[i].vertex_AO);
    }
    // free glfw resources
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::exit(status);
}



float RandomFloat(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
    //    return random;
    
}
