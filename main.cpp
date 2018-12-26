#include <iostream>
#include <thread>
#include <unistd.h>
#include <XP/helper/serial-lib.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "boost/filesystem.hpp"

int fd_;
uint8_t response[20];
bool ESC_pressed = false;

int create_map (std::string record_path) {
  while (!ESC_pressed) {
    std::cout << "jgjidjifji" << std::endl;
  }
}

int auto_navigator (std::string navigation_folder) {
  while (!ESC_pressed) {
    std::cout << "jgjidjifji" << std::endl;
  }
}


int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);

  /*** setting the record_path ***/
  int hour;
  std::string record_path = "";
  std::string navigation_folder = "";
  std::time_t t = std::time(NULL);
  char buf[32];
  std::strftime(buf, sizeof(buf), "%H", std::localtime(&t));
  hour = atoi(buf);
  namespace fs = boost::filesystem;

  if (record_path.empty()) {
    const char *env_p = std::getenv("HOME");
    if (env_p == nullptr) {
      LOG(ERROR) << "You need to set record_path";
      return -1;
    }

    if ((hour>=0) && (hour<6))
      record_path = std::string(env_p) + "/Boteye/data/seq/morning";
    else if ((hour>=6) && (hour<13))
      record_path = std::string(env_p) + "/Boteye/data/seq/am";
    else if ((hour>=13) && (hour<18))
      record_path = std::string(env_p) + "/Boteye/data/seq/pm";
    else if ((hour>=18) && (hour<24))
      record_path = std::string(env_p) + "/Boteye/data/seq/night";

    std::cout << "Rename record path to " << record_path << "\n";

    if(!fs::exists(record_path)) {
      fs::create_directories(record_path);
    }
    printf("FLAGS_record_path=%s\n",record_path.c_str());
  }

  fd_ = XP::serialport_init("/dev/ttyS2", 115200);
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
      if (XP::serialport_read_hex_n(fd_, response, 11, 10000) != 0) {
        LOG(ERROR) << "XP::serialport_read_hex_n failed";
      } else {
        std::cout << "##### thread_sub  response[2] = #####" << response[2]<< std::endl;
        std::cout << "##### thread_sub  response[4] = #####" << response[4]<< std::endl;
        if (response[2] == 0x50) {
          if (response[4] == 1) {
            // start create Map
//              std::cout << "##### thread_sub  response[5] = #####" << response[5]<< std::endl;
            mode = 1;
          } else if (response[4] == 2) {
            ESC_pressed = true;
            is_running = false;
          }
        } else if (response[2] == 0x51) {
          std::cout << "response[4] = " << response[4] << std::endl;
          mode = 2;
        } else if(response[2] == 0x70) {
          if (response[4] == 1) {
            //监听到导航
            mode = 3;
          } else if (response[4] == 2) {
            ESC_pressed = true;
            is_running = false;
          }
        } else if (response[2] == 0x72) {
          mode = 4;
        }
      }
      usleep(1000);
    }
  });
  std::cout << "Hello, World! after not join" << std::endl;

  auto thread_main = std::thread([&mode, &is_running, &record_path, &navigation_folder]() {
    while (is_running) {
//      if (mode == 1) {
//        // todo(jingjing) record data
//        std::cout << "start record data" << std::endl;
//      } else if (mode == 2) {
//        // todo (jingjing) start navigation
//        std::cout << "start navigation" << std::endl;
//      } else {
//        std::cout << "mode is still -1, please choose mode" << std::endl;
//      }
      if (mode == 1) {
        // todo(jinjin) record data
        std::cout << "start record data" << std::endl;
        // 监听到建图
        printf("######## receive create map data ##########\n");
        printf("start create Map\n");
        if(!fs::exists(record_path + "/live.pb")) {
          create_map(record_path);
        } else {
          printf("Map already, whether to recreate y/n\n");
          if (response[5] == 1) {
            fs::remove(record_path + "/live.pb");
            create_map(record_path);
          } else if (response[5] == 2) {
            LOG(ERROR) << "Existing map, no longer created\n";
//            is_running = false;
          }
        }
      } else if (mode == 3) {
        // todo (jinjin) start navigation
        std::cout << "start navigation" << std::endl;
        //监听到导航
        navigation_folder = record_path + "/navigation";
        if(!fs::exists(navigation_folder)) {
          fs::create_directories(navigation_folder);
          printf("FLAGS_navigation_folder=%s\n",navigation_folder.c_str());
        }
        if(!fs::exists(navigation_folder + "/navi.pb")) {
          if(!fs::exists(record_path + "/live.pb")) {
            create_map(record_path);
          }
        } else {
          fs::remove(navigation_folder + "/navi.pb");
        }
        auto_navigator(navigation_folder);
      } else {
//        std::cout << "mode is still -1, please choose mode" << std::endl;
      }

      usleep(1000000);
    }
  });
  thread_main.join();
  return 0;
}