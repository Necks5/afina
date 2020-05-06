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

void Engine::block(void *coro) {
    context* for_block = static_cast<context *>(coro);
    if(coro == nullptr) {
        for_block = cur_routine;
    }
    if(for_block->is_bloking) {
        return;
    }
    for_block->is_bloking = true;
    // delete coroutine from the list of alive coroutines
    if (alive == for_block) {
        alive = alive->next;
    }
    if (for_block->prev != nullptr) {
        for_block->prev->next = for_block->next;
    }
    if (for_block->next != nullptr) {
        for_block->next->prev = for_block->prev;
    }
    // add coroutine to the list of blocked coroutines
    for_block->prev = nullptr;
    for_block->next = blocked;
    blocked = for_block;
    if (blocked->next != nullptr) {
        blocked->next->prev = for_block;
    }
    if (for_block == cur_routine) {
        if (cur_routine != nullptr && cur_routine != idle_ctx) {
            if (setjmp(cur_routine->Environment) > 0) {
                return;
            }
            Store(*cur_routine);
        }
        cur_routine = nullptr;
        Restore(*idle_ctx);
    }

}

void Engine::unblock(void *coro) {
    context * for_unblock = static_cast<context *>(coro);
    if (for_unblock == nullptr || !for_unblock->is_bloking) {
        return;
    }
    for_unblock->is_bloking = false;
    if (blocked == for_unblock) {
        blocked = blocked->next;
    }
    if (for_unblock->prev != nullptr) {
        for_unblock->prev->next = for_unblock->next;
    }
    if (for_unblock->next != nullptr) {
        for_unblock->next->prev = for_unblock->prev;
    }
    // add coroutine to the list of alive coroutines
    for_unblock->prev = nullptr;
    for_unblock->next = alive;
    alive = for_unblock;
    if (alive->next != nullptr) {
        alive->next->prev = for_unblock;
    }


}


} // namespace Coroutine
} // namespace Afina
