#define _GNU_SOURCE  // pthread_setaffinity_np(), CPU_SET, etc

#include <time.h>
#include <math.h> // round()
#include <stdatomic.h>
#include <unistd.h>  // sysconf()
#include <pthread.h>
#include <assert.h>
#include <locale.h> // setlocale()

#include "kernelops.h"

#define None (WORD)0

#define _DONE(cont, value) (R){RDONE, (cont), (value)}
#define _CONT(cont, value) (R){RCONT, (cont), (value)}
#define _WAIT(cont, value) (R){RWAIT, (cont), (value)}
#define _EXIT(cont, value) (R){REXIT, (cont), (value)}

char *RTAG_name(RTAG tag) {
    switch (tag) {
        case RDONE: return "RDONE"; break;
        case RCONT: return "RCONT"; break;
        case RWAIT: return "RWAIT"; break;
        case REXIT: return "REXIT"; break;
    }
}

void dump_clos(Clos c);

Clos CLOS1(R (*code)(Clos,WORD), WORD v0) {
    Clos c = CLOS(code, 1);
    c->var[0] = v0;
    return c;
}

Clos CLOS2(R (*code)(Clos,WORD), WORD v0, WORD v1) {
    Clos c = CLOS(code, 2);
    c->var[0] = v0;
    c->var[1] = v1;
    return c;
}
    
Clos CLOS3(R (*code)(Clos,WORD), WORD v0, WORD v1, WORD v2) {
    Clos c = CLOS(code, 3);
    c->var[0] = v0;
    c->var[1] = v1;
    c->var[2] = v2;
    return c;
}

R DONE(Clos this, WORD val) {
    return _DONE(NULL, val);
}

struct Clos doneC = { DONE };

Msg ASYNC(Actor to, Clos c) {
    Msg m = MSG(c);
    m->value = &doneC;
    if (msg_ENQ(m, to)) {
        ready_PUSH(to);
    }
    return m;
}

R AWAIT(Msg m, Clos th) {
    return _WAIT(th, m);
}

_Atomic int loop_count = 0;
_Atomic int wait_count_max = 0;

void loop(int thread_id) {
    printf("[%d] message loop\n", thread_id);

    while (1) {
        Actor current = ready_POP();
        if (current) {
            Msg m = current->msg;
            atomic_fetch_add(&loop_count, 1);

            R r = m->clos->code(m->clos, m->value);

            switch (r.tag) {
                case RDONE: {
                    m->value = r.value;
                    Actor b = waiting_FREEZE(m);
                    uint32_t count = 0;
                    while (b) {
                        ++count;
                        b->msg->value = r.value;
                        Actor next = b->next;  // need to copy b->next; ready_PUSH will reset it
                        ready_PUSH(b);

                        b = next;
                    }
                    if(wait_count_max < count) {  // no, this isn't atomic...
                        wait_count_max = count;
                    }

                    if (msg_DEQ(current)) {
                        ready_PUSH(current);
                    }
                    break;
                }
                case RCONT: {
                    m->clos = r.cont;
                    m->value = r.value;

                    ready_PUSH(current);
                    break;
                }
                case RWAIT: {
                    m->clos = r.cont;
                    Msg x = (Msg)r.value;

                    if (! waiting_ADD(current, x)) {
                        m->value = x->value;
                        ready_PUSH(current);
                    }
                    break;
                case REXIT:
                    exit((int)r.value);
                }
            }
        } else {
            printf("[%d] unemployed!\n", thread_id);
            //getchar();
            static struct timespec idle_wait = { 0, 50000000 };  // 500ms
            nanosleep(&idle_wait, NULL);
       }
    }
}

WORD bootstrap(Clos c) {
    WORD v = &doneC;
    while (1) {
        R r = c->code(c, v);
        if (r.tag == RDONE)
            return r.value;
        c = r.cont;
        v = r.value;
    }
}

const int PRINT_INTERVAL = 500000;
int PING_LIMIT = PRINT_INTERVAL * 6;  // must be multiple of PRINT_INTERVAL

#include "pingpong2.c"

void *thread_main(void *arg) {
    loop((int)arg);

    return NULL;
}

double timestamp() {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    time_t s = t.tv_sec;
    long µs = round(t.tv_nsec / 1.0e3);   // ns -> µs
    if (µs > 999999) {
        ++s;
        µs = 0;
    }
    return (double)s + µs/1e6;
}

static double t0 = 0.0;

void cleanup() {
    printf("======================================================================\n");

    double t = timestamp();
    printf("total duration:      \x1b[1m%.3f\x1b[m seconds\n", t - t0);
    printf("total loops:         \x1b[1m%'d\x1b[m   \x1b[33;1m%.3f\x1b[m Mloops/s\n", loop_count, (loop_count/1e6)/(t - t0));
    printf("messages created:    \x1b[1m%'d\x1b[m   \x1b[33;1m%.3f\x1b[m Mmsg/s\n", msg_created, (msg_created/1e6)/(t - t0));
    printf("waiting_FREEZEs:     \x1b[1m%'d\x1b[m   \x1b[33;1m%.3f\x1b[m Mfreeze/s\n", wait_freeze_count, (wait_freeze_count/1e6)/(t - t0));
    printf("msg_ENQs:            \x1b[1m%'d\x1b[m   \x1b[33;1m%.3f\x1b[m Mmsg/s\n", msg_enq_count, (msg_enq_count/1e6)/(t - t0));
    printf("ready Q max size:    \x1b[1m%'d\x1b[m\n", readyQ_max);
    printf("msg Q max size:      \x1b[1m%'d\x1b[m\n", msgQ_max);
    printf("waiting max size:    \x1b[1m%'d\x1b[m\n", wait_count_max);
}


///////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[]) {
    atexit(cleanup);
    setlocale(LC_ALL, "");  // for printf's thousand separators to work

    long num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
    long num_threads = num_cpu;
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0) {
            num_threads = n;
        }
    }

    Actor roots[num_threads];
    for (int i = 0; i<num_threads; i++)
        roots[i] = bootstrap(BOOSTRAP_CLOSURE);

    PING_LIMIT = PRINT_INTERVAL + PRINT_INTERVAL*(num_cpu - num_threads);
    if (PING_LIMIT < 0) {
        PING_LIMIT = PRINT_INTERVAL;
    }
    printf("\x1b[34mPing limit:\x1b[m \x1b[1m%'d\x1b[m  \x1b[34m~~  Print interval:\x1b[m \x1b[1m%'d\x1b[m\n", PING_LIMIT, PRINT_INTERVAL);

    printf("\x1b[34mWorker threads:\x1b[m \x1b[1m%'ld\x1b[m  \x1b[34m~~  CPU cores: \x1b[1m%ld\x1b[m\n", num_threads, num_cpu);

    t0 = timestamp();

    // start worker threads, one per CPU
    pthread_t threads[num_threads];
    pthread_attr_t attrs;
    cpu_set_t cpu_set;
    for(int th_id = 0; th_id < num_threads; ++th_id) {
        pthread_attr_init(&attrs);
        CPU_ZERO(&cpu_set);
        int core_id = th_id % num_cpu;
        CPU_SET(core_id, &cpu_set);
#if defined(__linux__)
        printf("[%d] CPU affinity: %d\n", th_id, core_id);
        pthread_attr_setaffinity_np(&attrs, sizeof(cpu_set_t), &cpu_set);
#else
        printf("setting thread affinity is not supported for your OS\n");
        // __APPLE__
        // __FreeBSD__
        // __unix__
#endif

        pthread_create(&threads[th_id], &attrs, thread_main, (void *)th_id);
    }
    
    // TODO: run I/O thread

    for(int th_id = 0; th_id < num_threads; ++th_id) {
        pthread_join(threads[th_id], NULL);
    }
}


void dump_clos(Clos c) {
    if (c == NULL) {
        printf("<NULL cont>");
    } else {
        printf("[");
        for (int idx = 0; idx < c->nvar; ++idx) {
            if (idx > 0) printf(", ");
            printf("%p", c->var[idx]);
        }
        printf("]");
    }
    printf("\n");
}
