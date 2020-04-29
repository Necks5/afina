#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char begin_adress;
    ctx.Hight = StackBottom;
    ctx.Low = &begin_adress;
    uint32_t stack_size = ctx.Hight - ctx.Low;
    std::get<0>(ctx.Stack) = new char[stack_size];
    std::get<1>(ctx.Stack) = stack_size;
    memcpy(std::get<0>(ctx.Stack), ctx.Low, stack_size);
}

void Engine::Restore(context &ctx) {
    char begin_adress;
    cur_routine = &ctx;
    if(&begin_adress <= ctx.Hight && &begin_adress >= ctx.Low) {
        Restore(ctx);
    }
    memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    if(alive == nullptr) {
        return;
    }
    context* next_coroutine = (alive == cur_routine) ? alive->next : alive;
    if(cur_routine != idle_ctx) {
        if(setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }

    Restore(*next_coroutine);
}

void Engine::sched(void *routine_) {
    auto nextCoro = static_cast<context *>(routine_);
    if (nextCoro == nullptr) {
        yield();
    }
    // we will do nothing if the next coroutine is blocked
    if (nextCoro == cur_routine ) {
        return;
    }
    // run the next coroutine
    if (cur_routine != nullptr && cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }
    Restore(*nextCoro);
}

} // namespace Coroutine
} // namespace Afina
