#include <cstdio>

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#include <execinfo.h>
#include <stdlib.h>


#include "print.h"
#include "tuhh.hpp"

#include "NaoInterface.hpp"


class PIDFile final
{
public:
  PIDFile(const char* path)
    : fd_(open(path, O_CREAT | O_WRONLY, 0600))
  {
    if (fd_ < 0)
    {
      throw std::runtime_error("Could not create PID file!");
    }
    if (flock(fd_, LOCK_EX | LOCK_NB) == -1)
    {
      close(fd_);
      throw std::runtime_error("Could not lock PID file!");
    }
    dprintf(fd_, "%d\n", getpid());
  }
  ~PIDFile()
  {
    close(fd_);
  }
private:
  int fd_;
};

static volatile int keepRunning = 1;
// The PID file cannot reside in /var/run because that directory is not writable.
static const char* pidFilePath = "/tmp/tuhhNao.pid";

void intHandler(int)
{
  keepRunning = 0;
    Log(LogLevel::DEBUG) << "Recieved SIGINT";
}

void segvHandler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main()
{
  setvbuf(stderr, nullptr, _IONBF, 0);
  setvbuf(stdout, nullptr, _IOLBF, 0);

  Log(LogLevel::INFO) << "Starting tuhhNao!";

  PIDFile pidFile(pidFilePath);

  Log(LogLevel::DEBUG) << "Registering signal handlers";
  { // SET SIGINT HANDLER
    struct sigaction sa;
    sa.sa_handler = &intHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
  }

  { // SET SIGSEGV HANDLER
    struct sigaction sa;
    sa.sa_handler = &segvHandler;
    sigemptyset(&sa.sa_mask);
//    sa.sa_flags = SA_STACK;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
  }

  try
  {
    sigset_t mask;
    sigemptyset(&mask);
    NaoInterface robotInterface;
    TUHH tuhh(robotInterface);
    while (keepRunning)
    {
      sigsuspend(&mask);
    }
    Log(LogLevel::INFO) << "Received signal, shutting application down!";
  }
  catch (const std::exception& e)
  {
    Log(LogLevel::ERROR) << "Exception in NaoInterface or TUHH:";
    Log(LogLevel::ERROR) << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    Log(LogLevel::ERROR) << "Unknown exception in NaoInterface or TUHH (which means it could be anywhere)!";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
