#include "Command.hpp"
#include "Condition.hpp"
#include "MutexLock.hpp"

#include <queue>


namespace ghz{

using ElemType=Command;

using std::queue;
using group_6::MutexLockGuard;
using group_6::MutexLock;
using group_6::Condition;

class CommandQueue
{
    public:
        CommandQueue(int queSize);
        void push(ElemType cmd);
        ElemType pop();
        bool empty() const;
        bool full() const;

        void wakeup();

    private:
        queue<ElemType>  _que;
        int         _queSize;
        MutexLock   _mutex;
        Condition   _notFull;
        Condition   _notEmpty;
        bool        _flag;
    };
}