
///////////////////////////////// includes ///////////////////////////////////
#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

#include <cstdlib>
#include <iostream>

// use gl definitions from glbinding 
using namespace gl;

/////////////////////////// variable definitions //////////////////////////////
// vertical field of view of camera
const float camera_fov = glm::radians(60.0f);
// initial window dimensions
const unsigned window_width = 640;
const unsigned window_height = 480;
// the rendering window
GLFWwindow* window;

// variables for fps computation
double last_second_time = 0;
unsigned frames_per_second = 0;

// the main shader program
GLuint simple_program = 0;

// cpu representation of model
model sun_model{};
model moon_model{};
model planet_model{};
model planet_model2{};
model planet_model3{};
model planet_model4{};
model planet_model5{};
model planet_model6{};
model planet_model7{};
model planet_model8{};

model planet_models[8] = {planet_model,planet_model2,planet_model3,planet_model4,planet_model5,planet_model6,planet_model7,planet_model8};
// holds gpu representation of model
struct model_object {
  GLuint vertex_AO = 0;
  GLuint vertex_BO = 0;
  GLuint element_BO = 0;
};

// creating planets
model_object sun_object;
model_object moon_object;
model_object planet_object;
model_object planet_object2;
model_object planet_object3;
model_object planet_object4;
model_object planet_object5;
model_object planet_object6;
model_object planet_object7;
model_object planet_object8;


model_object planets[8] = {planet_object,planet_object2,planet_object3,planet_object4,planet_object5,planet_object6,planet_object7,planet_object8};


// camera matrices
glm::mat4 camera_view = glm::translate(glm::mat4{}, glm::vec3{0.0f, 0.0f, 40.0f});
glm::mat4 camera_projection{1.0f};

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;

// path to the resource folders
std::string resource_path{};

/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_view(GLFWwindow* window, int width, int height);
void update_camera();
void update_uniform_locations();
void update_shader_programs();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_geometry();
void show_fps();
void render();

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
  glfwSetFramebufferSizeCallback(window, update_view);

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
  update_shader_programs();

  // initialize projection and view matrices
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  update_view(window, width, height);
  update_camera();

  // set up models
  initialize_geometry();

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

///////////////////////// initialisation functions ////////////////////////////
// load models
void initialize_geometry() {
    
    //Create Sun seperately from planets and moon
    sun_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL);
    
    // generate vertex array object
    glGenVertexArrays(1, &sun_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(sun_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &sun_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, sun_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sun_model.data.size(), sun_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, sun_model.vertex_bytes, sun_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, sun_model.vertex_bytes, sun_model.offsets[model::NORMAL]);
    
    // generate generic buffer
    glGenBuffers(1, &sun_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sun_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * sun_model.indices.size(), sun_model.indices.data(), GL_STATIC_DRAW);
    
    //Create Moon seperately from planets and sun
    moon_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL);
    
    // generate vertex array object
    glGenVertexArrays(1, &moon_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(moon_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &moon_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, moon_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * moon_model.data.size(), moon_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, moon_model.vertex_bytes, moon_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, moon_model.vertex_bytes, moon_model.offsets[model::NORMAL]);
    
    // generate generic buffer
    glGenBuffers(1, &moon_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * moon_model.indices.size(), moon_model.indices.data(), GL_STATIC_DRAW);
    
    
    //For Loop to create planets
    for (int i = 0; i < 8; i = i + 1){
        
  planet_models[i] = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &planets[i].vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planets[i].vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planets[i].vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planets[i].vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_models[i].data.size(), planet_models[i].data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_models[i].vertex_bytes, planet_models[i].offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_models[i].vertex_bytes, planet_models[i].offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planets[i].element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planets[i].element_BO);
  // configure currently bound array buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_models[i].indices.size(), planet_models[i].indices.data(), GL_STATIC_DRAW);}
}

///////////////////////////// render functions ////////////////////////////////
// render model
void render() {
    
    //sun is rendered
    glm::mat4 model_matrix = glm::translate(glm::mat4{}, glm::vec3{0.0f, 0.0f, 0.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3{2.0f, 2.0f, 2.0f});
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_matrix = glm::inverseTranspose(camera_view * model_matrix);
   glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    
    
    glBindVertexArray(sun_object.vertex_AO);
    utils::validate_program(simple_program);
    glDrawElements(GL_TRIANGLES, GLsizei(sun_model.indices.size()), model::INDEX.type, NULL);
    
    
    //earth treated differently
    glm::mat4 earth_matrix = glm::rotate(glm::mat4{}, float(glfwGetTime()+3), glm::vec3{0.0f, 1.0f, 0.0f});
    earth_matrix = glm::translate(earth_matrix, glm::vec3{4.0f +4*3, 0.0f, -1.0f});
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(earth_matrix));
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_earth_matrix = glm::inverseTranspose(camera_view * earth_matrix);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_earth_matrix));
    
    
    glBindVertexArray(planets[3].vertex_AO);
    utils::validate_program(simple_program);
    // draw bound vertex array as triangles using bound shader
    glDrawElements(GL_TRIANGLES, GLsizei(planet_models[3].indices.size()), model::INDEX.type, NULL);

    
    
    //moon is rendered
    glm::mat4 model_matrix2;
    
   // model_matrix2 = glm::translate(model_matrix2, glm::vec3{7.0f, 7.0f, 8.0f});
    model_matrix2 = glm::rotate(earth_matrix, float(glfwGetTime()), glm::vec3{0.0f, 1.0f, 0.0f});
    model_matrix2 = glm::translate(model_matrix2, glm::vec3{2.0f, 0.0f, 0.0f});
    model_matrix2 = glm::scale(model_matrix2, glm::vec3{0.5f, 0.5f, 0.5f});
    
    
    
    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix2));
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::mat4 normal_matrix2 = glm::inverseTranspose(camera_view * model_matrix2);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix2));
    
    glBindVertexArray(moon_object.vertex_AO);
    utils::validate_program(simple_program);
    glDrawElements(GL_TRIANGLES, GLsizei(moon_model.indices.size()), model::INDEX.type, NULL);
    
    

    
    //planetes except earth are rendered
    for (int i = 0; i < 8; i = i + 1){
        if (i != 3){
  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(glfwGetTime()+i), glm::vec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::vec3{4.0f +4*i, 0.0f, -1.0f});
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrix = glm::inverseTranspose(camera_view * model_matrix);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));


  glBindVertexArray(planets[i].vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_models[i].indices.size()), model::INDEX.type, NULL);
        }
    }}

///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void update_view(GLFWwindow* window, int width, int height) {
  // resize framebuffer
  glViewport(0, 0, width, height);

  float aspect = float(width) / float(height);
  float fov_y = camera_fov;
  // if width is smaller, extend vertical fov 
  if(width < height) {
    fov_y = 2.0f * glm::atan(glm::tan(camera_fov * 0.5f) * (1.0f / aspect));
  }
  // projection is hor+ 
  camera_projection = glm::perspective(fov_y, aspect, 0.1f, 100000.0f);
  // upload matrix to gpu
  glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
}

// update camera transformation
void update_camera() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::mat4 inv_camera_view = glm::inverse(camera_view);
  // upload matrix to gpu
  glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));
}

// load shaders and update uniform locations
void update_shader_programs() {
  try {
    // throws exception when compiling was unsuccessfull
    GLuint new_program = shader_loader::program(resource_path + "shaders/simple.vert",
                                                resource_path + "shaders/simple.frag");
    // free old shader
    glDeleteProgram(simple_program);
    // save new shader
    simple_program = new_program;
    // bind shader
    glUseProgram(simple_program);
    // after shader is recompiled uniform locations may change
    update_uniform_locations();

    // upload view uniforms to new shader
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_view(window, width, height);
    update_camera();
  }
  catch(std::exception&) {
    // dont crash, allow another try
  }
}

// update shader uniform locations
void update_uniform_locations() {
  location_normal_matrix = glGetUniformLocation(simple_program, "NormalMatrix");
  location_model_matrix = glGetUniformLocation(simple_program, "ModelMatrix");
  location_view_matrix = glGetUniformLocation(simple_program, "ViewMatrix");
  location_projection_matrix = glGetUniformLocation(simple_program, "ProjectionMatrix");
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
    update_shader_programs();
  }
  else if(key == GLFW_KEY_W && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, -10.0f});
    update_camera();
  }
  else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, 10.0f});
    update_camera();
  }
  else if(key == GLFW_KEY_Q && action == GLFW_PRESS){
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 2.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_E && action == GLFW_PRESS){
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, -2.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_A && action == GLFW_PRESS){
      camera_view = glm::translate(camera_view, glm::vec3{-2.0f, 0.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_D && action == GLFW_PRESS){
      camera_view = glm::translate(camera_view, glm::vec3{2.0f, 0.0f, 0.0f});
      update_camera();
  }
  else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
      camera_view = glm::rotate(camera_view, float(0.1), glm::vec3{1.0f, 0.0f, 0.0f});
    update_camera();
    }
  else if (key == GLFW_KEY_UP && action == GLFW_PRESS){
      camera_view = glm::rotate(camera_view, float(-0.1), glm::vec3{1.0f, 0.0f, 0.0f});
      update_camera();
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