#include "app/app.h"
#include <glad/glad.h>
#include <imgui_impl_sdl2.h>
#include <memory>
#include "core/ecs/components/mesh_component.h"
#include "core/ecs/components/texture_component.h"
#include "core/ecs/entity.h"
#include "core/ecs/main_registry.h"
#include "core/ecs/registry.h"
#include "core/event/app_event.h"
#include "core/event/event_manager.h"
#include "core/input/keyboard_events.h"
#include "core/scene/scene.h"
#include "core/scene/scene_manager.h"
#include "gui/debug_window.h"
#include "gui/file_explorer.h"
#include "gui/imgui_manager.h"
#include "gui/scene_viewport.h"
#include "logger/logger.h"
#include "utilities/shader_manager/shader_manager.h"
#include "utilities/task_manager/task_manager.h"
#include "window/window.h"

using namespace kogayonon_logger;

namespace kogayonon_app
{
App::~App()
{
    cleanup();
}

void App::cleanup()
{
    Logger::info("Closing app and cleaning up");
    Logger::shutdown();
}

void App::pollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL2_ProcessEvent(&e);
        switch (e.type)
        {
        case SDL_WINDOWEVENT: {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int newWidth = e.window.data1;
                int newHeight = e.window.data2;
                kogayonon_core::WindowResizeEvent windowResizeEvent(newWidth, newHeight);
                EVENT_MANAGER()->dispatchEventToListeners(windowResizeEvent);
            }
            break;
        }
        case SDL_QUIT: {
            m_running = false;
            break;
        }
        case SDL_KEYDOWN: {

            // TODO maybe rethink this a bit
            auto keycode = static_cast<kogayonon_core::KeyCode>(e.key.keysym.sym);
            kogayonon_core::KeyPressedEvent keyPressEvent(keycode, 0);
            Logger::info("KeyPressedEvent - ", static_cast<int>(keycode));
            EVENT_MANAGER()->dispatchEventToListeners(keyPressEvent);
            break;
        }
        case SDL_KEYUP: {

            // TODO maybe rethink this a bit
            auto keycode = static_cast<kogayonon_core::KeyCode>(e.key.keysym.sym);
            kogayonon_core::KeyReleasedEvent keyReleaseEvent(keycode);
            Logger::info("KeyReleasedEvent - ", static_cast<int>(keycode));
            EVENT_MANAGER()->dispatchEventToListeners(keyReleaseEvent);
            break;
        }
        }
    }
}

void App::run()
{
    if (!init())
    {
        m_running = false;
    }
    while (m_running)
    {
        pollEvents();
        IMGUI_MANAGER()->draw();
        m_pWindow->swapWindow();
    }
}

bool App::initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Logger::critical("Could not initialize sdl");
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    auto pWinProps = m_pWindow->getWindowProps();

    auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (pWinProps->maximized)
    {
        flags |= SDL_WINDOW_MAXIMIZED;
    }

    auto win = SDL_CreateWindow(pWinProps->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pWinProps->width,
                                pWinProps->height, flags);
    m_pWindow->setWindow(std::move(win));

    auto ctx = SDL_GL_CreateContext(m_pWindow->getWindow());
    m_pWindow->setContext(std::move(ctx));

    SDL_GL_MakeCurrent(m_pWindow->getWindow(), m_pWindow->getContext());
    assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress));

    glEnable(GL_DEBUG_OUTPUT);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    rescaleMainViewport(pWinProps->width, pWinProps->height);
    return true;
}

bool App::initRegistries()
{
    auto& pMainRegistry = REGISTRY();

    // init imgui manager
    auto imguiManager = std::make_shared<kogayonon_gui::ImGuiManager>(m_pWindow->getWindow(), m_pWindow->getContext());
    assert(imguiManager && "could not initialise imgui manager");
    pMainRegistry.addToContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>(std::move(imguiManager));

    // init event manager
    auto eventManager = std::make_shared<kogayonon_core::EventManager>();
    assert(eventManager && "could not initialise event manager");
    pMainRegistry.addToContext<std::shared_ptr<kogayonon_core::EventManager>>(std::move(eventManager));

    // init task manager
    auto taskManager = std::make_shared<kogayonon_utilities::TaskManager>(10);
    assert(taskManager && "could not initialise task manager");
    pMainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::TaskManager>>(std::move(taskManager));

    // init shader manager
    auto shaderManager = std::make_shared<kogayonon_utilities::ShaderManager>();
    assert(shaderManager && "could not initialise shader manager");
    shaderManager->pushShader("resources/shaders/3d_vertex.glsl", "resources/shaders/3d_fragment.glsl", "3d");
    pMainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>(std::move(shaderManager));

    return true;
}

bool App::initGui()
{
    // insert windows
    auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>("Scene");

    // might just get the frame buffer pointer into the renderer and then just bind it, draw, unbind and pass the
    // texture to the scene viewport, oooor since we get a pointer to a member variable from scene viewport (the frame
    // buffer) we can just do all the drawing elsewhere and after that the scene viewport gets the buffer texture
    sceneViewport->setCallback([this]() { callbackTest(); });
    /*
    in renderer or something
    frameBuffer->bind();
    glDraw....
    frameBuffer->unbind();

    in scene viewport
    ImGui::Begin();
    ImGui::AddImage(frameBuffer->getTexture(),...);
    ImGui::End();
    */

    auto debugWindow = std::make_unique<kogayonon_gui::DebugConsoleWindow>("Debug console");

    // root for where the file explorer can see files or folders
    std::string rootPath = "/";
    auto fileExplorerWindow = std::make_unique<kogayonon_gui::FileExplorerWindow>("Assets", std::move(rootPath));

    IMGUI_MANAGER()->pushWindow("Scene", std::move(sceneViewport));
    IMGUI_MANAGER()->pushWindow("Debug console", std::move(debugWindow));
    IMGUI_MANAGER()->pushWindow("Assets", std::move(fileExplorerWindow));

    return true;
}

bool App::initScenes()
{
    auto testScene = std::make_shared<kogayonon_core::Scene>("Test");
    auto entity = std::make_unique<kogayonon_core::Entity>(testScene->getRegistry());
    entity->addComponent<kogayonon_core::TextureComponent>("TextureTest", "resources/textures/TextureTest.png");
    kogayonon_core::SceneManager::getInstance().addScene(testScene, "Test");
    kogayonon_core::SceneManager::getInstance().setCurrentScene("Test");
    return true;
}

bool App::init()
{
#ifdef _DEBUG
    m_pWindow = std::make_shared<kogayonon_window::Window>("kogayonon engine (DEBUG)", 1200, 800, 1, false);
#else
    m_pWindow = std::make_shared<kogayonon_window::Window>("kogayonon engine", 800, 600, 1, false);
#endif

    Logger::initialize("log.txt");

    if (!initSDL() || !initRegistries() || !initGui() || !initScenes())
    {
        return false;
    }

    EVENT_MANAGER()->listenToEvent<kogayonon_core::WindowResizeEvent>([this](const kogayonon_core::Event& e) -> bool {
        return this->onWindowResize((kogayonon_core::WindowResizeEvent&)e);
    });

    auto future = TASK_MANAGER()->enqueue([]() -> int {
        int sum = 0;
        for (int i = 10; i < 30; i++)
        {
            sum += i;
        }
        return sum;
    });
    Logger::info("Task manager test result is ", future.get());

    return true;
}

void App::rescaleMainViewport(int w, int h)
{
    m_pWindow->resize();
    glViewport(0, 0, w, h);
}

bool App::onWindowResize(kogayonon_core::WindowResizeEvent& e)
{
    rescaleMainViewport(e.getWidth(), e.getHeight());
    return true;
}

void App::callbackTest()
{
    auto scene = kogayonon_core::SceneManager::getInstance().getCurrentScene();
    if (!scene.lock())
        return;

    auto& registry = scene.lock()->getRegistry();

    static GLuint quadVAO = 0, quadVBO = 0;
    if (quadVAO == 0)
    {
        float quadVertices[] = {// positions   // texCoords
                                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
                                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f};

        glCreateVertexArrays(1, &quadVAO);
        glCreateBuffers(1, &quadVBO);
        glNamedBufferStorage(quadVBO, sizeof(quadVertices), quadVertices, 0);
        glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 4 * sizeof(float));

        glEnableVertexArrayAttrib(quadVAO, 0);
        glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(quadVAO, 0, 0);

        glEnableVertexArrayAttrib(quadVAO, 1);
        glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
        glVertexArrayAttribBinding(quadVAO, 1, 0);
    }

    SHADER_MANAGER()->bindShader("3d");

    glBindVertexArray(quadVAO);

    auto view = registry.getRegistry().view<kogayonon_core::TextureComponent>();
    for (auto [entity, textureComp] : view.each())
    {
        if (textureComp.m_texture == 0)
            continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureComp.m_texture);

        // Set uniform if your shader needs it
        GLint loc = glGetUniformLocation(SHADER_MANAGER()->getShaderId("3d"), "uTexture");
        if (loc >= 0)
            glUniform1i(loc, 0);

        // Draw full-screen quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    SHADER_MANAGER()->unbindShader("3d");
}
} // namespace kogayonon_app