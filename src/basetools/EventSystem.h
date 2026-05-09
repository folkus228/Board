#pragma once
#include <vector>
#include <algorithm>

template<typename... Args>
class Event
{
private:
    using Callback = void(*)(void*, Args...);

    struct Slot
    {
        size_t id;
        void* obj;
        Callback func;
    };

    std::vector<Slot> _slots;
    size_t nextId = 0;
public:
    template<typename T, void(T::* Method)(Args...)>
    size_t connect(T* instance)
    {
        size_t id = nextId++;
        _slots.push_back({ id, instance,
            [](void* obj, Args... args)
            {
                (static_cast<T*>(obj)->*Method)(args...);
            }
            });
        return id;
    }

    void disconnect(size_t id)
    {
        _slots.erase(
            std::remove_if
            (
                _slots.begin(), _slots.end(),
                [id](const Slot& s)
                {
                    return s.id == id;
                }
            ),
            _slots.end()
        );
    }

    void emit(Args... args)
    {
        for (size_t i = 0; i < _slots.size(); ++i)
        {
            _slots[i].func(_slots[i].obj, args...);
        }
    }
};