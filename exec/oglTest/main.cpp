#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>
#include "Graphics/Geometry/Cube.h"

constexpr int HEIGHT = 800;
constexpr int WIDTH = 800;

int main()
{

    // initialise log
    mpu::Log mainLog(mpu::ALL, mpu::ConsoleSink());

    // create window and init gl
    mpu::gph::Window window(WIDTH,HEIGHT,"GravitySim");

    // create a cube
    mpu::gph::Buffer vbo(mpu::gph::Geometry::Cube::position());
    mpu::gph::VertexArray vao;
    vao.addAttributeBufferArray(0,vbo,0,3*sizeof(GLfloat),3,0);
    vao.bind();

    // create a mvp in gpu memory
    mpu::gph::Buffer ubo;
    ubo.allocate<mpu::gph::ModelViewProjection>(1,GL_MAP_READ_BIT|GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT);
    ubo.bindBase( 0, GL_UNIFORM_BUFFER);
    auto mvp = ubo.map<mpu::gph::ModelViewProjection>(1,0,GL_MAP_READ_BIT|GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT);
    mvp[0] = mpu::gph::ModelViewProjection();

    // create camera
    mpu::gph::Camera camera(&window);
    camera.setMVP(&mvp[0]);

    // set gl options
    glClearColor(1, 1, 1, 1);
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);
    //mpu::gph::enableVsync(true);
    glfwSwapInterval(0);

    // create a shader
    mpu::gph::addShaderIncludePath(LIB_SHADER_PATH);
    mpu::gph::ShaderProgram shader({ {LIB_SHADER_PATH"simple.frag"},
                                     {LIB_SHADER_PATH"simple.vert"} });
    shader.use();

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    while( window.update())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFinish();

        dt = timer.getDeltaTime();
        camera.update(dt);

        shader.uniform4f("color",{1,0,0,1});
        mvp[0].setModel(glm::translate(glm::mat4(),{0,0,0}));
        glDrawArrays(GL_TRIANGLES,0,mpu::gph::Geometry::Cube::vertexCount);
        glFinish();

        shader.uniform4f("color",{0,1,0,1});
        mvp[0].setModel(glm::translate(glm::mat4(),{2,0,0}));
        glDrawArrays(GL_TRIANGLES,0,mpu::gph::Geometry::Cube::vertexCount);
        glFinish();

        shader.uniform4f("color",{0,0,1,1});
        mvp[0].setModel(glm::translate(glm::mat4(),{-2,0,0}));
        glDrawArrays(GL_TRIANGLES,0,mpu::gph::Geometry::Cube::vertexCount);


        // performance display
        nbframes++;
        elapsedPerT += dt;
        if(elapsedPerT >= 1.0)
        {
            printf("%f ms/frame -- %f fps\n", 1000.0*elapsedPerT/double(nbframes), nbframes/elapsedPerT);
            nbframes = 0;
            elapsedPerT = 0;
        }

    }

    return 0;
}