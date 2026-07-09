#pragma once

#include <enkiTS/TaskScheduler.h>
#include <utility>

class ThreadManager
{
public:
    ThreadManager() = delete;

    static inline void Init() { scheduler.Initialize(); }
    static inline void Shutdown() { WaitAll(); }

    // Используем шаблон, чтобы избежать тяжелого std::function
    template<typename F>
    static void Spawn(F&& fn)
    {
        // Создаем задачу конкретно под тип нашей лямбды
        struct AsyncTask final : public enki::ITaskSet {
            // Сохраняем саму лямбду напрямую, без аллокаций std::function
            typename std::decay<F>::type work;

            explicit AsyncTask(F&& f) : work(std::forward<F>(f)) {}

            void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
                work();
                delete this; // Мгновенное удаление
            }

            // Оптимизируем выделение памяти для самой структуры.
            // Вместо тяжелого системного malloc, enkiTS предоставляет кастомную быструю аллокацию
            void* operator new(size_t size) {
                return enki::GetTaskSchedulerWithCustomAllocations()->GetCustomAllocator().alloc(size, nullptr);
            }
            void operator delete(void* ptr) {
                enki::GetTaskSchedulerWithCustomAllocations()->GetCustomAllocator().free(ptr, nullptr);
            }
        };

        // За счет кастомного new, эта строчка теперь выполняется мгновенно
        auto* task = new AsyncTask(std::forward<F>(fn));
        scheduler.AddTaskSetToPipe(task);
    }

    static inline void Yield() { scheduler.RunJobs(); } // откладывает выполнение. Полезно если после проверки выяснилось что данные не пришли или задача не выполнена
    static inline void WaitAll() { scheduler.WaitAll(); }

    static inline enki::TaskScheduler& GetScheduler() { return scheduler; }

private:
    static inline enki::TaskScheduler scheduler;
};
