#include <core/coreexecute.h>
#include <core/corecommand.h>
#include <core/corelock.h>
#include <list>
#include <stdio.h>

namespace iot {
namespace core {
#define EXECUTE_TIMEOUT  1000 /* 1s timeout */
class Executer::Private
{
public:
    typedef std::list<Command *> Commands;
    Private():
        _primary(0) {

    }
    ~Private() {

    }

    int schedule(Command *command, bool primary) {
        _locker.lock();
        if (primary != true) {
            _commands.push_back(command);
        } else {
            if (_primary != command && _primary != 0)
                _commands.push_back(_primary);
            _primary = command;
        }
        _locker.unlock();
        return 0;
    }


    void step() {

        _locker.lock();
        Commands commands = _commands;
        _commands.clear();
        _locker.unlock();

        while (!commands.empty()) {
            Command *command = commands.front();
            command->execute();
            commands.pop_front();
        }


        _locker.lock();
        Command *primary = _primary;
        _primary = 0;
        _locker.unlock();
        if (primary == 0) {
            return;
        }

        primary->execute();

        _locker.lock();
        if (_primary == 0)
            _primary = primary;
        _locker.unlock();


    }

    int exec() {
        _running = true;
        _code = 0;
        while (_running)
            step();
        _commands.clear();
        return _code;
    }


    void exit(int code) {
        _code = code;
        _running = false;
    }
    void clear() {

    }
private:
    bool _running;
    int _code;
    Locker _locker;
    Command *_primary;
    Commands _commands;
};


/**
 * @brief Executer::Executer
 */
Executer::Executer():
    _private(new Private)
{

}

Executer::~Executer()
{
    delete _private;
}

int Executer::schedule(Command *command, bool primary)
{
    return _private->schedule(command, primary);
}

int Executer::exec()
{
    return _private->exec();
}

void Executer::exit(int code)
{
    _private->exit(code);
}

} // namespace Core
} // namespace iot

