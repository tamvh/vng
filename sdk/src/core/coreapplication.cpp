#include <core/coreapplication.h>
#include <core/coresynchronize.h>
#include <core/coreexecute.h>
#include <core/corevariant.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

namespace iot {
namespace core {
class Application::Private
{
public:

    Private(const std::string &config, Application *instance):
        _config(config) {
        _instance = instance;
    }

    int initialize(Application &app, bool terminate) {
        bool ok;
        core::Variant json = core::Variant::loadJson(_config, &ok);
        if (!ok) {
            printf("*core application* Parse config file failed\r\n");
            return -1;
        }
        const Variant *synchronize = json.value("synchronize");
        if (synchronize == NULL) {
            printf("*core application* 'synchronize' value not found\r\n");
            return -1;
        }

        int error = setup(*synchronize);
        if (error != 0) {
            printf("*core application* Setup failed with error: %d\r\n", error);
            return error;
        }

        const Variant *settings = json.value("settings");
        if (settings == NULL) {
            printf("*core application* 'settings' value not found\r\n");
            _synchronizer->stop();
            delete _synchronizer;
            _synchronizer = NULL;
            return -1;
        }
        error = app.initialize(*_synchronizer, *settings);
        if (error != 0) {
            printf("*core application* initialize failed with error: %d\r\n", error);
            _synchronizer->stop();
            delete _synchronizer;
            _synchronizer = NULL;
            return error;
        }
        if (!terminate)
            return 0;

        struct sigaction action;
        memset(&action, 0, sizeof(action));
        action.sa_flags = SA_NOCLDSTOP;
        action.sa_handler = handleSignal;
        error = sigaction(SIGINT, &action, NULL);

        if (error != 0) {

            app.release();
            _synchronizer->stop();
            delete _synchronizer;
            _synchronizer = NULL;
            return error;
        }
        return 0;
    }

    int exec(Application &app, bool terminate) {
        int error = initialize(app, terminate);
        if (error != 0)
            return error;

        error = _executer.exec();
        release(app);
        return error;
    }

    int schedule(Command *command) {
        int error = _executer.schedule(command, false);
        if (error != 0)
            return error;
        _synchronizer->skip();
        return 0;
    }

    void release(Application &app) {
        app.release();
        _synchronizer->stop();
        delete _synchronizer;
        _synchronizer = NULL;
    }

    inline core::Executer &executer() {
        return _executer;
    }


    void exit(int code) {
        _executer.exit(code);
    }

    static Application *instance() {
        return _instance;
    }

private:

    int setup(const Variant &settings) {

        const Variant *events = settings.value("events");

        if (events == NULL)
            return -1;

        if (!events->isInteger())
            return -1;

        const Variant *timeout = settings.value("timeout");
        if (!timeout->isInteger())
            return -1;

        _synchronizer = new Synchronizer(events->toInt(), timeout->toInt());
        int error = _synchronizer->start(_executer);

        if (error != 0) {
            delete _synchronizer;
            _synchronizer = NULL;
            return error;
        }
        return 0;
    }

private:

    static void handleSignal(int signal) {
        if (signal != SIGINT)
            return;
        printf("\nApplication* Ctrl + C\n");
        Private::_instance->exit(0);
    }

private:
    std::string _config;
    Synchronizer *_synchronizer;
    Executer _executer;
    static Application *_instance;
};

Application *Application::Private::_instance = NULL;

Application::Application(const std::string &config):
    _private(new Private(config, this))
{
}

Application::~Application()
{
    delete _private;
}


int Application::exec(bool terminate)
{
    return _private->exec(*this, terminate);
}

int Application::schedule(Command *command)
{
   return _private->schedule(command);
}

void Application::exit(int code)
{
    _private->exit(code);
}

Application *Application::instance()
{
    return Private::instance();
}

} // namespace Core
} // namespace iot
