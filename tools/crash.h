
///////////////////
// CRASH SANDBOX //
///////////////////
typedef enum {
  UBENCH_CRASH_NONE = 0,
  UBENCH_CRASH_SEGV,
  UBENCH_CRASH_BUS,
  UBENCH_CRASH_FPE,
  UBENCH_CRASH_ILL,
} ubench_crash_kind_t;

typedef void (*ubench_crash_body_t)(void *userdata);

#if defined(SP_LINUX) || defined(SP_MACOS)
#include <signal.h>
#include <setjmp.h>

struct ubench_crash_ctx_s {
  sigjmp_buf jmp;
  /* Written by the async signal handler; volatile per C99 7.13.2.1. */
  volatile sig_atomic_t kind;
  s32 active;
  struct ubench_crash_ctx_s *prev;
};

static SP_THREAD_LOCAL struct ubench_crash_ctx_s *ubench_crash_current =
    SP_NULLPTR;

static void ubench_crash_handler(s32 sig) {
  struct ubench_crash_ctx_s *c = ubench_crash_current;
  if (c == SP_NULLPTR || !c->active) {
    /* Crash outside a sandbox - fall through to default termination. */
    signal(sig, SIG_DFL);
    raise(sig);
    return;
  }
  switch (sig) {
    case SIGSEGV: c->kind = UBENCH_CRASH_SEGV; break;
    case SIGBUS:  c->kind = UBENCH_CRASH_BUS;  break;
    case SIGFPE:  c->kind = UBENCH_CRASH_FPE;  break;
    case SIGILL:  c->kind = UBENCH_CRASH_ILL;  break;
    default:      c->kind = UBENCH_CRASH_SEGV; break;
  }
  siglongjmp(c->jmp, 1);
}

static ubench_crash_kind_t ubench_crash_try(ubench_crash_body_t body,
                                            void *userdata) {
  struct ubench_crash_ctx_s ctx;
  struct sigaction sa = sp_zero;
  struct sigaction old_segv = sp_zero, old_bus = sp_zero;
  struct sigaction old_fpe  = sp_zero, old_ill = sp_zero;

  ctx.kind   = UBENCH_CRASH_NONE;
  ctx.active = 0;
  ctx.prev   = ubench_crash_current;

  sa.sa_handler = ubench_crash_handler;
  /* SA_NODEFER lets a fault during the handler re-enter rather than deadlock
     on a masked signal. */
  sa.sa_flags = SA_NODEFER;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, &old_segv);
  sigaction(SIGBUS,  &sa, &old_bus);
  sigaction(SIGFPE,  &sa, &old_fpe);
  sigaction(SIGILL,  &sa, &old_ill);

  ubench_crash_current = &ctx;
  if (sigsetjmp(ctx.jmp, 1) == 0) {
    ctx.active = 1;
    body(userdata);
  }
  ctx.active = 0;
  ubench_crash_current = ctx.prev;

  sigaction(SIGSEGV, &old_segv, SP_NULLPTR);
  sigaction(SIGBUS,  &old_bus,  SP_NULLPTR);
  sigaction(SIGFPE,  &old_fpe,  SP_NULLPTR);
  sigaction(SIGILL,  &old_ill,  SP_NULLPTR);
  return (ubench_crash_kind_t)ctx.kind;
}

#elif defined(SP_WIN32)
#include <setjmp.h>

struct ubench_crash_ctx_s {
  jmp_buf jmp;
  volatile LONG kind;
  s32 active;
  struct ubench_crash_ctx_s *prev;
};

static SP_THREAD_LOCAL struct ubench_crash_ctx_s *ubench_crash_current =
    SP_NULLPTR;

static LONG CALLBACK ubench_crash_veh(EXCEPTION_POINTERS *ep) {
  struct ubench_crash_ctx_s *c = ubench_crash_current;
  ubench_crash_kind_t k;
  if (c == SP_NULLPTR || !c->active) return EXCEPTION_CONTINUE_SEARCH;
  switch (ep->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
      k = UBENCH_CRASH_SEGV; break;
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      k = UBENCH_CRASH_BUS; break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_STACK_CHECK:
      k = UBENCH_CRASH_FPE; break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
      k = UBENCH_CRASH_ILL; break;
    default:
      return EXCEPTION_CONTINUE_SEARCH;
  }
  c->kind = (LONG)k;
  /* longjmp out of the VEH callback; on x64 setjmp/longjmp are unwind-aware
     under both MSVC and MinGW-w64, so the OS dispatcher frames unwind. */
  longjmp(c->jmp, 1);
}

static ubench_crash_kind_t ubench_crash_try(ubench_crash_body_t body,
                                            void *userdata) {
  struct ubench_crash_ctx_s ctx;
  PVOID h;

  ctx.kind   = UBENCH_CRASH_NONE;
  ctx.active = 0;
  ctx.prev   = ubench_crash_current;

  /* First-handler (1) so we get the exception before any debugger or other
     handler claims it. */
  h = AddVectoredExceptionHandler(1, ubench_crash_veh);
  ubench_crash_current = &ctx;
  if (setjmp(ctx.jmp) == 0) {
    ctx.active = 1;
    body(userdata);
  }
  ctx.active = 0;
  ubench_crash_current = ctx.prev;
  RemoveVectoredExceptionHandler(h);
  return (ubench_crash_kind_t)ctx.kind;
}

#else
static SP_INLINE ubench_crash_kind_t
ubench_crash_try(ubench_crash_body_t body, void *userdata) {
  body(userdata);
  return UBENCH_CRASH_NONE;
}
#endif
