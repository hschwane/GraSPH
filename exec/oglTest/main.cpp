#include <iostream>
#include <random>
#include <vector>
#include <list>

#include <Graphics/Graphics.h>
#include <ctime>
#include <Log/Log.h>
#include <Log/ConsoleSink.h>

#include "ShaderTools.h"

constexpr int HEIGHT = 600;
constexpr int WIDTH = 800;
constexpr int NUM_PARTICLES = 300;
constexpr double G_CONST = 6.6740831e-11;
constexpr double MASS = 0.00003;
constexpr double RADIUS = 0.00001;
constexpr double DT = 0.001;
constexpr int RESPAWN = 30;
constexpr double RESPAWN_RADIUS = 0.1;
constexpr float SPEED_DIVIDER = 2;

struct particle
{
public:
    glm::vec2 m_pos;
    glm::vec2 m_vel;
    glm::vec2 m_acc;
    float m_mass;
    float m_radius;
};

void spawnParticles(int numOfParticles, std::list<particle> &particleList, float mass, float radius, glm::vec2 lower, glm::vec2 upper, glm::vec2 speed = glm::vec2())
{
    for(int i = 0; i < numOfParticles; ++i)
    {
        particle p;

        p.m_pos.x = lower.x + static_cast<float>(rand()) / (static_cast <float>(RAND_MAX/(upper.x-lower.x)));
        p.m_pos.y = lower.y + static_cast<float>(rand()) / (static_cast <float>(RAND_MAX/(upper.y-lower.y)));

        std::cout << "spawning: " << p.m_pos.x << " | " << p.m_pos.y << std::endl;

        p.m_vel = speed;
        p.m_mass = mass;
        p.m_radius = radius;
        particleList.push_back(p);
    }
}

void drawParticles(std::list<particle> &particleList, GLuint vbo)
{
    std::vector<glm::vec2> vert;
    for(auto &&item : particleList)
    {
        vert.push_back(item.m_pos);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*vert.size(), vert.data(), GL_STATIC_DRAW);
    glDrawArrays( GL_POINTS, 0, vert.size());
}

void animateParticles(std::list<particle> &particleList, float dt)
{
    for(auto p = particleList.begin(); p != particleList.end(); ++p)
    {
        for(auto pp = particleList.begin(); pp != p;)
        {
            glm::vec2 rv = pp->m_pos - p->m_pos; // vector from p to pp
            float r12 = glm::dot(rv,rv); // distance squared

            // check collision
            if(r12 <= (p->m_radius+pp->m_radius)) // not corect but faster
            {
                // particles colliding
                glm::vec2 impuls = p->m_mass * p->m_vel + pp->m_mass * pp->m_vel;
                p->m_mass += pp->m_mass;
                p->m_vel = impuls / p->m_mass;

                pp = particleList.erase(pp);
            }
            else
            {
                // calculate gravity
                glm::vec2 gravity = (p->m_mass * pp->m_mass / r12) * glm::normalize(rv); // gravity

                p->m_acc += gravity / p->m_mass;
                pp->m_acc += -1.0f * gravity / pp->m_mass;
                ++pp;
            }
        }
    }

    for(auto &&item : particleList)
    {
        item.m_vel += item.m_acc * dt;
        item.m_pos += item.m_vel * dt;
        item.m_acc = glm::vec2(0,0);
    }
}

int main()
{
    // initialise log
    mpu::Log mainLog(mpu::ALL, mpu::ConsoleSink());

    // create window and init gl
       mpu::gph::Window window(WIDTH,HEIGHT,"GravitySim");
    window.setPosition(glm::ivec2(600,50));


    mpu::gph::Buffer b;
    b.allocate<uint8_t>(30, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    {
        auto data =  b.map<uint8_t>(30, 0, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

        data[0] = 5;
        data[1] = 3;

        for(auto it = data.begin()+2; it != data.end(); ++it)
        {
            *it = 8;
        }
    } // buffer is unmapped here

    {
        auto data = b.map<const uint8_t>(30,0, GL_MAP_READ_BIT);
        for(auto it = data.begin(); it != data.end(); ++it)
        {
            std::cout << int(*it) << "\n";
        }
    } // buffer is unmapped here

/*

    // set bg and shader
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    ShaderTools::createColorShaderProgram(1.0f,0.0f,0.0f);

    // particles
    srand(std::time(nullptr));
    std::list<particle> ptls;
    spawnParticles(NUM_PARTICLES,ptls,MASS,RADIUS,glm::vec2(-0.75f,-0.75), glm::vec2(0.75f,0.75));

    std::cout << "created particles" << std::endl;

    // vertex buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    GLuint vertexarray;
    glGenVertexArrays(1, &vertexarray);
    glBindVertexArray(vertexarray);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glPointSize( 2);

    double oldT = glfwGetTime();
    double lag = 0.0;

    bool spawning = false;
    glm::vec2 spawnpos;

    int nbframes =0;
    double lastPerfT = oldT;
    while( !glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // timing
        double time = glfwGetTime();
        double elapsed = time - oldT;
        oldT = time;
        lag += elapsed;

        // get input
        if(!spawning && glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            spawning = true;
            glm::f64vec2 cpos;
            glfwGetCursorPos(window, &cpos.x, &cpos.y);
            cpos.x = (cpos.x / WIDTH)*2 -1;
            cpos.y = (-cpos.y / HEIGHT)*2 +1;
            spawnpos = cpos;
        }
        else if(spawning && glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        {
            glm::f64vec2 cpos;
            glfwGetCursorPos(window, &cpos.x, &cpos.y);
            cpos.x = (cpos.x / WIDTH)*2 -1;
            cpos.y = (-cpos.y / HEIGHT)*2 +1;
            glm::vec2 speed = glm::vec2(cpos) - spawnpos / SPEED_DIVIDER;


            spawnParticles(RESPAWN,ptls,MASS,RADIUS, glm::vec2(spawnpos)-glm::vec2(RESPAWN_RADIUS), glm::vec2(spawnpos)+glm::vec2(RESPAWN_RADIUS), speed);
            spawning = false;
        }

        // animate particles
        while(lag > DT)
        {
            animateParticles(ptls, DT);
            lag -= DT;
        }

        // draw particles
        drawParticles( ptls, vertexbuffer);

        // performance display
        nbframes++;
        double elapsedPerT = time-lastPerfT;
        if(elapsedPerT >= 1.0)
        {
            printf("%f ms/frame\n", 1000.0*elapsedPerT/double(nbframes));
            nbframes = 0;
            lastPerfT += elapsedPerT;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

    */
}