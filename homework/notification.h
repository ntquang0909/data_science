#ifndef _EPOLL_THREADPOOL_NOTIFICATION_H_
#define _EPOLL_THREADPOOL_NOTIFICATION_H_

#include "eventmanager.h"
#include <glog/logging.h>

namespace epoll_threadpool {

/**
 * Simple pthread based notification object.
 * Similar to pthread_cond_t but it can only be fired once and it will
 * stay triggered once fired.
 */
class Notification {
 public:
  Notification() {
    pthread_mutex_init(&_mutex, 0);
    pthread_cond_init(&_cond, 0);
    _signaled = false;
  }
  virtual ~Notification() {
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_cond);
  }

  void signal() {
    pthread_mutex_lock(&_mutex);
    _signaled = true;
    pthread_cond_broadcast(&_cond);
    pthread_mutex_unlock(&_mutex);
  }

  void unsignal() {
    pthread_mutex_lock(&_mutex);
    _signaled = false;
    pthread_mutex_unlock(&_mutex);
  }
    
  bool tryWait(EventManager::WallTime when) {
    pthread_mutex_lock(&_mutex);
    if (_signaled) {
      pthread_mutex_unlock(&_mutex);
      return true;
    }
    struct timespec ts = { (int64_t)when, (when - (int64_t)when) * 1000000000 };
    int ret = pthread_cond_timedwait(&_cond, &_mutex, &ts);
    pthread_mutex_unlock(&_mutex);
    return ret == 0;
  }
  void wait() {
    pthread_mutex_lock(&_mutex);
    if (_signaled) {
      pthread_mutex_unlock(&_mutex);
      return;
    }
    int ret = pthread_cond_wait(&_cond, &_mutex);
    pthread_mutex_unlock(&_mutex);
  }
 protected:
  volatile bool _signaled;
  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
};

/**
 * Similar to Notification but required a set number of calls to signal() 
 * before becoming 'signalled'.
 * TODO(aarond10): Fix terminology. Overloaded use of the word "signal".
 */
class CountingNotification : public Notification {
 public:
  /**
   * Creates a notification that will only be signalled after num calls to
   * signal().
   */
  CountingNotification(int num) : _num(num) { }
  virtual ~CountingNotification() { }

  void signal() {
    pthread_mutex_lock(&_mutex);
    if (--_num <= 0) {
      _signaled = true;
      pthread_cond_broadcast(&_cond);
    }
    pthread_mutex_unlock(&_mutex);
  }
 private:
  int _num;
};
}
#endif
