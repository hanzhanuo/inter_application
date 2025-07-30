#include "Mutex.hpp"
#include "Condition_Variable.hpp"

namespace ghz{
class UniqueLock{
    public:
    UniqueLock(Mutex& mutex);
    ~UniqueLock();
    private:
    Mutex& m_mutex;
}
}