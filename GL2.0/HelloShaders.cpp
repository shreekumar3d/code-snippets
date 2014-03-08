//
//
// Simple program that shows shader usage in GL 2.0 and above
// Keeping this as a reference. Everytime I come back and 
// write a shader, I make some mistakes in the template.
// Hoping to kick this habit !
//

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

unsigned char imgBuff[512*512*4];

GLuint loadTexture()
{
    FILE *fp = fopen("test-image.raw","rb");
    if(!fp) {
        cerr << "No image" << endl;
        exit(0);
    }

    int ret = fread(imgBuff, sizeof(imgBuff), 1, fp);
    if(ret != 1) { 
        fclose(fp);
        cerr << "Unable to read enough bytes!" << endl;
        exit(0);
    }

    GLuint tex;

    // Create a texture & bind it
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Load texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgBuff);

    // Forgetting these two lines of code can get you staring at DARK screens !
    // Forget them at your own peril ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // No need to set RST wrapping etc for this program...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    return tex;
}

void showInfoLog(GLuint item)
{
    GLint infoLen = 0;
    glGetProgramiv(item, GL_INFO_LOG_LENGTH, &infoLen);
    cout << "ERROR in shader, length of info log = " << infoLen << endl;
    if(infoLen > 1)
    {
        char* infoLog = new char[infoLen];
        glGetProgramInfoLog(item, infoLen, NULL, infoLog);
        cout << infoLog << endl;
        delete []infoLog;
    } else {
        cout << "WOW ! empty log. Ensure you call this only if there's an error!" << endl;
    }
    exit(1);
}

GLuint createShader(GLuint type, const char *shaderText)
{
    GLuint shader = glCreateShader(type);
    GLint status;

    glShaderSource(shader, 1,  (const GLchar**) &shaderText, 0);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(!status) {
        showInfoLog(shader);
    }
    return shader;
}

void display()
{
    static GLuint prog = 0;

    // Only create these things the first time
    if(!prog) { 

        GLuint tex = loadTexture();
        cout << "Texture = " << tex << endl;

        const char* vertShader = 
            "attribute vec4 vPosition;\n"
            "attribute vec2 tex;\n"
            "varying vec2 texCoords;\n"
            "void main() {\n"
            "   gl_Position = vPosition;\n"
            "   texCoords = tex;\n"
            "}";
        const char* fragShader = 
            "varying vec2 texCoords;\n"
            "uniform sampler2D texture;\n"
            "void main() {\n"
            "   gl_FragColor = texture2D(texture, texCoords);\n"
            "}";

        // Load the shaders
        GLuint vShader = createShader(GL_VERTEX_SHADER, vertShader);
        cout << "Vertex shader = " << vShader << endl;
        GLuint fShader = createShader(GL_FRAGMENT_SHADER, fragShader);
        cout << "Fragment shader = " << fShader << endl;

        // Create a "program" and add in the shaders
        prog = glCreateProgram();
        cout << "Program = " << prog << endl;
        glAttachShader(prog, vShader);
        glAttachShader(prog, fShader);

        // You need to bind all attributes before you can use 'em
        // Doing this later post link will have no effect. And worse,
        // you'll get no errors! You'll be one confused dev if you lose
        // out on these
        glBindAttribLocation(prog, 0, "vPosition");
        glBindAttribLocation(prog, 1, "tex");

        // Link and ensure all's well
        GLint status;
        glLinkProgram(prog);
        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if(!status) {
            showInfoLog(prog);
        }

        glUseProgram(prog);

        // Setup the uniforms - the "Link" stage clears these according to the
        // spec
        GLuint texArg = glGetUniformLocation(prog, "texture");

        // This first command below is technically not necessary, as
        // the default texture unit is 0
        //glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(texArg, 0); // indicates we are using texture unit 0
    }

    // This is where do the real drawing...
    glUseProgram(prog);

    GLfloat pos[] = { 
        -1.0, -1.0, 0.0, // LB
        1.0, -1.0, 0.0, // RB
        -1.0, 1.0, 0.0, // LT
        1.0, 1.0, 0.0}; // RT
    GLfloat texCoords[] = {
        0.0, 0.0, // 0,0 corresponds to bottom left
        1.0, 0.0,
        0.0, 1.0,
        1.0, 1.0}; // 1,1 is bottom right
    

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *) pos);
    glEnableVertexAttribArray(0); // we bound this to the vertex array earlier

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLfloat *) texCoords);
    glEnableVertexAttribArray(1); // we bound this to the tex array earlier

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw two triangles covering the screen

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
    glutInitWindowSize(512, 512); 
    glutCreateWindow("Hello GL Shaders");
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
