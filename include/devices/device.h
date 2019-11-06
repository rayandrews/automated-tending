/*
 * Licensed under the MIT License <http: //opensource.org/licenses/MIT>.
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2019 Ray Andrew
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#pragma once

#include <exception>
#include <iostream>
#include <sstream>

#include "gpio.h"

namespace emmerich::device {
enum class device_mode {
  INPUT,
  OUTPUT,
};

inline const std::string getDeviceModeString(const device_mode& mode) {
  if (mode == device_mode::INPUT) {
    return "input";
  } else {
    return "output";
  }
}

enum class device_output {
  LOW,
  HIGH,
};

inline const std::string getOutputModeString(const device_output& output) {
  if (output == device_output::LOW) {
    return "low";
  } else {
    return "high";
  }
}

class DeviceException : public std::exception {
 private:
  const std::string                     _message;
  const std::shared_ptr<spdlog::logger> _logger;
  const int                             _pin;
  const device_mode                     _mode;
  const device_output                   _output;

 public:
  DeviceException(const std::string&   message,
                  const int            pin,
                  const device_mode&   mode,
                  const device_output& output);
  const char* what() const noexcept;
};

class Device {
 private:
  const int   _pin;
  device_mode _mode;

 public:
  Device(int pin);
  Device(int pin, device_mode mode);

  void               setMode(device_mode mode);
  void               write(device_output level);
  device_output      read() const;
  inline int         getPin() const { return _pin; }
  inline device_mode getMode() const { return _mode; }
};
}  // namespace emmerich::device

#endif
