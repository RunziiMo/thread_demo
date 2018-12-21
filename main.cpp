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

  auto thread2 = std::thread([]() {
    int count = 0;
    while (count++ <= 10) {
      // todo(tingting) you can add XP::serialport_write and XP::serialport_read function here
      usleep(1000);
      std::cout << "not join while " << count << std::endl;
    }
  });
  std::cout << "Hello, World! after not join" << std::endl;
  thread2.join();
  return 0;
}