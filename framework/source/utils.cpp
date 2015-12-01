#include "utils.hpp"
#include "texture.hpp"

// load glbinding function type
#include <glbinding/Function.h>
// load meta info extension
#include <glbinding/Meta.h>
// load callback support
#include <glbinding/callbacks.h>

#include <iostream>
#include <sstream>
#include <fstream>

using glbinding::Meta;

namespace utils {

GLuint texture_object(texture const& tex) {
  GLuint texture_object = 0;
  glGenTextures(1, &texture_object);

  // bind new texture handle to current unit for configuration
  glBindTexture(tex.target, texture_object);
  // if coordinate is outside texture, use border color
  glTexParameteri(tex.target, GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
  //linear interpolation if texel is smaller/bigger than fragment pixel 
  glTexParameteri(tex.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
  glTexParameteri(tex.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

  // determine format of image data, internal format should be sized
  GLenum internal_format = GL_NONE;
  if (tex.channels == GL_RED) {
    internal_format = GL_R8;
  }
  else if (tex.channels == GL_RG) {
    internal_format = GL_RG8;
  }
  else if (tex.channels == GL_RGB) {
    internal_format = GL_RGB8;
  }
  else if (tex.channels == GL_RGBA) {
    internal_format = GL_RGBA8;
  }

  // create blank texture if struct contains no pixel data
  GLvoid const* data_ptr = nullptr;
  if (!tex.data.empty()) {
    data_ptr = &tex.data[0];
  }

  // define & upload texture data
  if (tex.target == GL_TEXTURE_1D){
    glTexImage1D(tex.target, 0, GLint(internal_format), tex.width, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else if (tex.target == GL_TEXTURE_2D) {
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexImage2D(tex.target, 0, GLint(internal_format), tex.width, tex.height, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else if (tex.target == GL_TEXTURE_3D){
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_R, GLint(GL_CLAMP_TO_EDGE));
    glTexImage3D(tex.target, 0, GLint(internal_format), tex.width, tex.height, tex.depth, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else {
    throw std::logic_error("Unsupported Format " + Meta::getString(tex.target));
  }

  return texture_object;
}

void print_bound_textures() {
  GLint id1, id2, id3, active_unit, texture_units = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &active_unit);
  std::cout << "Active texture unit: " << active_unit - GLint(GL_TEXTURE0) << std::endl;

  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

  for(GLint i = 0; i < texture_units; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &id3);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &id2);
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &id1);
    if(id1 != 0 || id2 != 0 || id3 != 0) {
      std::cout <<"Texture unit " << i << " - ";
      if(id1 != 0) std::cout << "1D: " << id1 << ", ";
      if(id2 != 0) std::cout << "2D: " << id2 << ", ";
      if(id3 != 0) std::cout << "3D: " << id3;
      std::cout << std::endl;
    }
  }
  // reactivate previously active unit
  glActiveTexture(GLenum(active_unit));
}

void glsl_error(int error, const char* description) {
  std::cerr << "GLSL Error " << error << " : "<< description << std::endl;
}

void watch_gl_errors(bool activate) {
  if(activate) {
    // add callback after each function call
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, { "glGetError" });
    glbinding::setAfterCallback(
      [](glbinding::FunctionCall const& call) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          // print name
          std::cerr <<  "OpenGL Error: " << call.function->name() << "(";
          // parameters
          for (unsigned i = 0; i < call.parameters.size(); ++i)
          {
            std::cerr << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
              std::cerr << ", ";
          }
          std::cerr << ")";
          // return value
          if(call.returnValue) {
            std::cerr << " -> " << call.returnValue->asString();
          }
          // error
          std::cerr  << " - " << Meta::getString(error) << std::endl;
          // throw exception to allow for backtrace
          throw std::runtime_error("Execution of " + std::string(call.function->name()));
          exit(EXIT_FAILURE);
        }
      }
    );
  }
  else {
    glbinding::setCallbackMask(glbinding::CallbackMask::None);
  }
}

void validate_program(GLuint program) {
  glValidateProgram(program);
  // check if validation was successfull
  GLint success = 0;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
  if(success == 0) {
    // get log length
    GLint log_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
    // get log
    GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
    glGetProgramInfoLog(program, log_size, &log_size, log_buffer);
    // output errors
    output_log(log_buffer, "program nr. " + std::to_string(program));
    // free broken program
    glDeleteProgram(program);
    free(log_buffer);

    throw std::logic_error("Validation of program nr. " + std::to_string(program));
  }
}

GLint get_bound_VAO() {
  GLint array = -1;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array);

  return array;
}

std::string file_name(std::string const& file_path) {
  return file_path.substr(file_path.find_last_of("/\\") + 1);
}

void output_log(GLchar const* log_buffer, std::string const& prefix) {
  std::string error{};
  std::istringstream error_stream{log_buffer};
  while(std::getline(error_stream, error)) {
    std::cerr << prefix << " - " << error << std::endl;
  }
}

std::string read_file(std::string const& name) {
  std::ifstream file_in{name};
  if(file_in) {
    std::string contents;
    file_in.seekg(0, std::ios::end);
    contents.resize(file_in.tellg());
    file_in.seekg(0, std::ios::beg);
    file_in.read(&contents[0], contents.size());
    file_in.close();
    return(contents);
  }
  else {
    std::cerr << "File \'" << name << "\' not found" << std::endl;
    
    throw std::invalid_argument(name);
  } 
}

};