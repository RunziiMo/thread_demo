/******************************************************************************
 * Copyright 2017-2018 Baidu Robotic Vision Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Tod E. Kurt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/
//
// arduino-serial-lib -- simple library for reading/writing serial ports
//
// 2006-2013, Tod E. Kurt, http://todbot.com/blog/
//

#include <XP/helper/serial-lib.h>
#include <stdio.h>    // Standard input/output definitions
#include <unistd.h>   // UNIX standard function definitions
#include <fcntl.h>    // File control definitions
#include <errno.h>    // Error number definitions
#include <termios.h>  // POSIX terminal control definitions
#include <string.h>   // String function definitions
#include <sys/ioctl.h>

#include <glog/logging.h>
// uncomment this to debug reads
#define SERIALPORTDEBUG

namespace XP {

// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud) {
  struct termios toptions;
  int fd;

  // fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
  fd = open(serialport, O_RDWR | O_NONBLOCK | O_NOCTTY);

  if (fd == -1)  {
    LOG(ERROR) << "serialport_init: Unable to open " << serialport;
    return -1;
  }

  // int iflags = TIOCM_DTR;
  // ioctl(fd, TIOCMBIS, &iflags);  // turn on DTR
  // ioctl(fd, TIOCMBIC, &iflags);  // turn off DTR

  if (tcgetattr(fd, &toptions) < 0) {
    perror("serialport_init: Couldn't get term attributes");
    return -1;
  }
  speed_t brate = baud;  // let you override switch below if needed
  switch (baud) {
    case 4800:
      brate = B4800;
      break;
    case 9600:
      brate = B9600;
      break;
#ifdef B14400
    case 14400:
      brate = B14400;
      break;
#endif
    case 19200:
      brate = B19200;
      break;
#ifdef B28800
    case 28800:
      brate = B28800;
      break;
#endif
    case 38400:
      brate = B38400;
      break;
    case 57600:
      brate = B57600;
      break;
    case 115200:
      brate = B115200;
      break;
    }
  cfsetispeed(&toptions, brate);
  cfsetospeed(&toptions, brate);

  // 8N1
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  // no flow control
  toptions.c_cflag &= ~CRTSCTS;

  // toptions.c_cflag &= ~HUPCL;  // disable hang-up-on-close to avoid reset

  toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
  toptions.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR);  // turn off s/w flow ctrl

  toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // make raw
  toptions.c_oflag &= ~OPOST;  // make raw

  // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
  toptions.c_cc[VMIN]  = 0;
  toptions.c_cc[VTIME] = 1;
  // toptions.c_cc[VTIME] = 20;

  tcsetattr(fd, TCSANOW, &toptions);
  if (tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
    perror("init_serialport: Couldn't set term attributes");
    return -1;
  }
  return fd;
}

//
int serialport_close(int fd) {
  return close(fd);
}

//
int serialport_writebyte(int fd, uint8_t b) {
  int n = write(fd, &b, 1);
  if (n != 1) {
    return -1;
  }
  return 0;
}

//
int serialport_write(int fd, const char* str) {
  int len = strlen(str);
  int n = write(fd, str, len);
  if (n != len) {
    LOG(ERROR) << "serialport_write: couldn't write whole string";
    return -1;
  }
  return 0;
}
int serialport_write_n(int fd, const uint8_t* str, int len) {
  int n = write(fd, str, len);
  if (n != len) {
    LOG(ERROR) << "serialport_write_n: couldn't write whole string";
    return -1;
  }
  return 0;
}

//
int serialport_read_until(int fd, char* buf, char until, int buf_max, int timeout) {
  char b[1];  // read expects an array, so we give it a 1-byte array
  int i = 0;
  do {
    int n = read(fd, b, 1);  // read a char at a time
    if (n == -1) return -1;   // couldn't read
    if (n == 0) {
      usleep(1 * 1000);  // wait 1 msec try again
      timeout--;
      if (timeout == 0) return -2;
      continue;
    }
#ifdef SERIALPORTDEBUG
    VLOG(1) << "serialport_read_until: i="  << i << " n= " << n << " b= " << b[0];
#endif
    buf[i] = b[0];
    i++;
  } while (b[0] != until && i < buf_max && timeout>0);

  buf[i] = 0;  // null terminate the string
  return 0;
}
int serialport_read_hex_n(int fd, uint8_t* buf, int read_num, int timeout) {
  int count = 0;
  while (count < read_num && timeout >= 0) {
    int res = read(fd, buf + count, 1);
    if (res != 1) {
      usleep(1 * 1000);  // wait 1 msec try again
      timeout--;
#ifdef SERIALPORTDEBUG
      VLOG(2) << "read failed code " << res;
#endif
    } else {
      ++count;
    }
  }
  if (count == read_num) {
#ifdef SERIALPORTDEBUG
    VLOG(1) << "serialport_read_n done";
#endif
    return 0;
  }
#ifdef SERIALPORTDEBUG
  LOG(ERROR) << "serialport_read_hex " << count << " / " << read_num << " timeout";
#endif
  return 1;
}
//
int serialport_flush(int fd, int sleep_usec) {
  usleep(sleep_usec);  // required to make flush work, for some reason
  return tcflush(fd, TCIOFLUSH);
}

}  // namespace XP
