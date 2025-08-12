#ifndef __Command_H__
#define __Command_H__

#include <functional>

namespace ghz
{

using Command=std::function<void()>;


class MyCommand
{
public:
    MyCommand(Command&& cmd)
    : _cmd(std::move(cmd))
    {

    }

    void execute() {
        if(_cmd) {
            _cmd();   //每个command都是一个命令行语句，暂时先这样写，待会再改这个的代码逻辑
        }
    }

private:
    Command _cmd;
};

}//end of namespace ghz



#endif   //__Command_H__