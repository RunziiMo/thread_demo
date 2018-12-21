#include <iostream>
#include <thread>
#include <unistd.h>
#include <XP/helper/serial-lib.h>
#include <glog/logging.h>

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  XP::serialport_init("/dev/ttyUSB0", 115200);
  std::thread([]() {
    int count = 0;
    while (count++ <= 10) {
      usleep(1000);
      std::cout << "join while " << count << std::endl;
    }
  }).join();
  std::cout << "Hello, World! after join" << std::endl;

  bool is_running = true;
  int mode = -1;

  auto thread2 = std::thread([&mode, &is_running]() {
    while (is_running) {
      // todo(jingjing) you can add XP::serialport_write and XP::serialport_read function here
      // todo(jingjing) when read serial done change mode type
      usleep(1000);
    }
  });
  std::cout << "Hello, World! after not join" << std::endl;

  auto thread_main = std::thread([&mode, &is_running]() {
    while (is_running) {
      if (mode == 1) {
        // todo(jingjing) record data
        std::cout << "start record data" << std::endl;
      } else if (mode == 2) {
        // todo (jingjing) start navigation
        std::cout << "start navigation" << std::endl;
      } else {
        std::cout << "mode is still -1, please choose mode" << std::endl;
      }
      usleep(1000000);
    }
  });
  thread_main.join();
  return 0;
}