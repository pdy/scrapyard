#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include <thread>

class InotifyTests : public Application
{
public:
    InotifyTests(int argc, char *argv[]);

protected:
    int main() override;
};


InotifyTests::InotifyTests(int argc, char *argv[])
    : Application(argc, argv, "InotifyTests")
{ 
    Application::showHelpIfNoArguments(); 
    Application::addCmdOption("directory,d", "directory");
//    Application::addCmdOptionFlag("flag,f", "example of cmd flag");
}




namespace {


class Handler
{
public:

};



int fd = -1;
int wd = -1;
uint32_t mask = 0;
int interval = 200000;

bool init(const std::string &path)
{
    fd = inotify_init();
    if(-1 == fd)
    {
        LOG_ERR << "fd == -1 on init";
        return false;
    }

    mask |= IN_CREATE;
    mask |= IN_DELETE;
    mask |= IN_MODIFY;
    mask |= IN_MOVED_FROM;
    mask |= IN_MOVED_TO;

    wd = inotify_add_watch(fd, path.c_str(), mask);

    if(-1 == wd)
    {
        LOG_ERR << "wd == -1 on inotify_add_watch";
        return false;
    }

    return true;
}

void onCreate(const std::string &path)
{
    LOG_INF << "Created " << path;
}
void onDelete(const std::string &path)
{
    LOG_INF << "Deleted " << path;
}

void onModify(const std::string &path)
{
    LOG_INF << "Modified " << path;
}

void onMovedFrom(const std::string &path, uint32_t cookie)
{
    LOG_INF << "Moved from " << path << "cookie " << cookie;
}

void onMovedTo(const std::string &path, uint32_t cookie)
{
    LOG_INF << "MovedTo " << path<< "cookie " << cookie;
}


[[noreturn]] void loop()
{

    std::array<char, 4096> buffer;
    for(;;)
    {

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
       
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = interval;

        if(select(fd + 1, &fds, nullptr, nullptr, &tv) == 1)
        {
            ssize_t n = read(fd, buffer.begin(), buffer.size()); 
            int i = 0;
            while(n > 0)
            {
                struct inotify_event* pEvent = reinterpret_cast<struct inotify_event*>(buffer.begin() + i);
                if(pEvent->len > 0)
                {
                    if(pEvent->mask & IN_CREATE)
                    {
                        onCreate(pEvent->name);
                        
                    }
                    if(pEvent->mask & IN_DELETE)
                    {
                        onDelete(pEvent->name);
                    }
                    if(pEvent->mask & IN_MODIFY)
                    {
                        onModify(pEvent->name);
                    }
                    if(pEvent->mask & IN_MOVED_FROM)
                    {
                        onMovedFrom(pEvent->name, pEvent->cookie);
                    }
                    if(pEvent->mask & IN_MOVED_TO)
                    {
                        onMovedTo(pEvent->name, pEvent->cookie);
                    }
                }

                i += sizeof(inotify_event) + pEvent->len;
                n -= sizeof(inotify_event) + pEvent->len;
            }
        }

   } 
}


}

class DirectoryWatcherImpl;
class DirectoryWatcher final
{
public:
  enum class EventType
  {
    CREATED,
    DELETED,
    MODIFIED,
    MOVED_FROM,
    MOVED_TO
  };
  
  struct Event final
  {
    EventType type;
    std::string name;
  };

  struct Error final
  {
    std::string msg;
    operator bool() const
    {
      return msg.empty();
    }    
  };

  //template<typename OnEventCallable>
  //DirectoryWatcher(OnEventCallable callable);

  ~DirectoryWatcher() = default;
  DirectoryWatcher() = delete;
  DirectoryWatcher(const DirectoryWatcher&) = delete;
  DirectoryWatcher(DirectoryWatcher&&) = delete;
  
  const DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;
  const DirectoryWatcher& operator=(DirectoryWatcher&&) = delete; 

  Error init(const std::chrono::milliseconds &interval, const std::string &path);
  void start();
  void stop();
  bool isRunning() const;

private:

  std::unique_ptr<DirectoryWatcherImpl> m_impl;

  
};


template<typename Derived>
class OSDirWatcher
{
  const Derived& derived() const { return *static_cast<const Derived*>(this); }
  friend Derived;
  OSDirWatcher() = default;

public:
  
  DirectoryWatcher::Error init(const std::chrono::milliseconds &interval, const std::string &path)
  {
    return derived().init(interval, path);
  }
 
  void start()
  {
    return derived().start();
  }

  void stop()
  {
    return derived().stop();
  }

  bool isRunning() const
  {
    return derived().isRunnig();
  }

protected:

};

class LinuxDirWatcher : public OSDirWatcher<LinuxDirWatcher>
{
public:
  ~LinuxDirWatcher()
  {
    LOG_TRC << "destroying LinuxDirWatcher...";
    stop();
    close(m_fd);
    LOG_TRC << "...destroyed";
  }


  DirectoryWatcher::Error init(const std::chrono::milliseconds &interval, const std::string &path)
  {
    if(isRunning())
    {
      return {"Can't call init() on running DirectoryWatcher. Call stop() first."};
    }

    m_fd = inotify_init();
    if(-1 == fd)
    {
      return { "fd == -1 on init" };
    }

    mask |= IN_CREATE;
    mask |= IN_DELETE;
    mask |= IN_MODIFY;
    mask |= IN_MOVED_FROM;
    mask |= IN_MOVED_TO;

    const int wd = inotify_add_watch(fd, path.c_str(), mask);

    if(-1 == wd)
    {
      return { "wd == -1 on inotify_add_watch" };
    }

    m_interval = interval;
    return {};
  }
 
  void start()
  {
    m_thread.reset(new std::thread(&LinuxDirWatcher::loop, this));
  }

  void stop()
  {
    LOG_TRC << "stop()";
    if(!isRunning())
      return;

    m_stopped = true;
    m_thread->join();
    LOG_TRC << "m_thread joined";
  }

  bool isRunning() const
  {
    return !m_stopped;
  }

private:
  void loop()
  {
    m_stopped = false;
    std::array<char, 4096> buffer;
    for(;;)
    {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(m_fd, &fds);
       
      struct timeval tv;
      tv.tv_sec  = 0;
      tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(m_interval).count();

      if(select(m_fd + 1, &fds, nullptr, nullptr, &tv) == 1)
      {
        ssize_t n = read(m_fd, buffer.begin(), buffer.size()); 
        int i = 0;
        while(n > 0)
        {
          struct inotify_event* pEvent = reinterpret_cast<struct inotify_event*>(buffer.begin() + i);
          if(pEvent->len > 0)
          {
            if(pEvent->mask & IN_CREATE)
            {
              onCreate(pEvent->name);            
            }
            if(pEvent->mask & IN_DELETE)
            {
              onDelete(pEvent->name);
            }
            if(pEvent->mask & IN_MODIFY)
            {
              onModify(pEvent->name);
            }
            if(pEvent->mask & IN_MOVED_FROM)
            {
              onMovedFrom(pEvent->name, pEvent->cookie);
            }
            if(pEvent->mask & IN_MOVED_TO)
            {
              onMovedTo(pEvent->name, pEvent->cookie);
            }
          }

          i += sizeof(inotify_event) + pEvent->len;
          n -= sizeof(inotify_event) + pEvent->len;
        }
      }
      
      if(m_stopped)
        break;

    }
  }

  std::chrono::milliseconds m_interval;
  int m_fd {-1};
  bool m_stopped{true};
  std::unique_ptr<std::thread> m_thread;
};

class DirectoryWatcherImpl : public LinuxDirWatcher{};

int InotifyTests::main()
{
    LOG_INF << "InotifyTests";

    const auto path = Application::getCmdOptionValue("directory");
    LOG_INF << "Dir " << path;
    //if(!::init(path)) return 1;
    
    //std::thread t(loop);
    //t.join();
    DirectoryWatcherImpl dw;
    auto err = dw.init(std::chrono::milliseconds(100), path);
    if(err)
    {
      LOG_DBG << err.msg;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}


int main(int argc, char *argv[])
{
    return MainApplication::run<InotifyTests>(argc, argv);
}
