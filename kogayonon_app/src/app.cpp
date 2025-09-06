#include "app/app.h"
#include <glad/glad.h>
#include <imgui_impl_sdl2.h>
#include <memory>
#include "core/ecs/registry_manager.h"
#include "core/event/app_event.h"
#include "core/event/event_manager.h"
#include "core/input/keyboard_events.h"
#include "gui/debug_window.h"
#include "gui/imgui_manager.h"
#include "gui/scene_viewport.h"
#include "logger/logger.h"
#include "utilities/task_manager/task_manager.h"
#include "window/window.h"

using namespace kogayonon_logger;

namespace kogayonon_app {
App::App()
{
#ifdef _DEBUG
    m_pWindow = std::make_shared<kogayonon_window::Window>("kogayonon engine (DEBUG)", 800, 600, 1, false);
#else
    m_pWindow = std::make_shared<kogayonon_window::Window>("kogayonon engine", 800, 600, 1, false);
#endif
}

App::~App()
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
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int newWidth = e.window.data1;
                int newHeight = e.window.data2;
                kogayonon_core::WindowResizeEvent e(newWidth, newHeight);
                EVENT_MANAGER()->dispatch(e);
            }
            break;
        case SDL_QUIT:
            m_running = false;
            break;
        case SDL_KEYDOWN:

            // TODO maybe rethink this a bit
            auto keycode = static_cast<kogayonon_core::KeyCode>(e.key.keysym.sym);
            kogayonon_core::KeyPressedEvent e(keycode, 0);
            EVENT_MANAGER()->dispatch(e);
            break;
        }
    }
}

void App::run()
{
    if (!initialise())
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

bool App::initialise()
{
    Logger::initialize("log.txt");

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

    auto win =
        SDL_CreateWindow(pWinProps->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pWinProps->width, pWinProps->height, flags);
    m_pWindow->setWindow(std::move(win));

    auto ctx = SDL_GL_CreateContext(m_pWindow->getWindow());
    m_pWindow->setContext(std::move(ctx));

    SDL_GL_MakeCurrent(m_pWindow->getWindow(), m_pWindow->getContext());
    assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress));

    glEnable(GL_DEBUG_OUTPUT);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    rescaleMainViewport(pWinProps->width, pWinProps->height);

    // initialise the managers
    auto imguiManager = std::make_shared<kogayonon_gui::ImGuiManager>(m_pWindow->getWindow(), m_pWindow->getContext());
    REGISTRY().addToContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>(std::move(imguiManager));

    auto eventManager = std::make_shared<kogayonon_core::EventManager>();
    REGISTRY().addToContext<std::shared_ptr<kogayonon_core::EventManager>>(std::move(eventManager));

    auto taskManager = std::make_shared<kogayonon_utilities::TaskManager>(10);
    REGISTRY().addToContext<std::shared_ptr<kogayonon_utilities::TaskManager>>(std::move(taskManager));

    EVENT_MANAGER()->subscribe<kogayonon_core::WindowResizeEvent>(
        [this](const kogayonon_core::Event& e) -> bool { return this->onWindowResize((kogayonon_core::WindowResizeEvent&)e); });

    auto future = TASK_MANAGER()->enqueue([]() -> int {
        int sum = 0;
        for (int i = 10; i < 30; i++)
        {
            sum += i;
        }
        return sum;
    });
    Logger::info("Task manager test result is ", future.get());

    // insert windows
    auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>("Scene");
    auto debugWindow = std::make_unique<kogayonon_gui::DebugConsoleWindow>("Debug console");

    IMGUI_MANAGER()->push_window("Scene", std::move(sceneViewport));
    IMGUI_MANAGER()->push_window("Debug console", std::move(debugWindow));

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
} // namespace kogayonon_app