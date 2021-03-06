
#include <gmock/gmock.h>


#include <glbinding/gl/functions.h>  // < imagine this was included by a 3rd party library (e.g., globjects.hpp)

#include <glbinding/gl/types45core.h>
#include <glbinding/gl/functions45core.h>
#include <glbinding/gl/enum45core.h>


using namespace gl45core;

void issue_82()   // Argument-dependent lookup (Koenig lookup)
{
    GLuint index = 0;
    glGetUniformSubroutineuiv(GL_VERTEX_SHADER, 0, &index);
}

TEST(Regression_82, Compilation)
{
    SUCCEED();  // compiling this file results in successful test
}
