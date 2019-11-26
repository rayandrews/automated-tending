/*
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
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

#ifndef FINGER_MOVEMENT_H_
#define FINGER_MOVEMENT_H_

#pragma once

#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>
#include <QThread>

#include <fruit/fruit.h>

#include "config.h"
#include "logger.h"
#include "state.h"

#include "utils/signal_merge.h"
#include "utils/worker.h"

#include "devices/limit_switch.h"
#include "devices/stepper.h"

namespace emmerich::mechanisms {

class Movement : public QObject {
  Q_OBJECT

 protected:
  std::queue<Point> _paths;
  useconds_t        _delay;

 public:
  Movement() = default;
  virtual ~Movement() = default;

  inline virtual const Movement& setStepDelay(useconds_t delay) {
    _delay = delay;
    return *this;
  }

  virtual const Movement& clearPaths() {
    std::queue<Point> empty;
    std::swap(_paths, empty);
    return *this;
  }

  virtual const Movement& setPaths(const std::queue<Point>& paths) {
    _paths = paths;
    return *this;
  }

  inline virtual const Movement& setPaths(Point paths[]) {
    clearPaths();

    for (int i = 0; i <= *(&paths + 1) - paths; i++) {
      _paths.push(paths[i]);
    }

    return *this;
  }

  virtual const Movement& goTo(const Point& point) = 0;

 public slots:
  virtual void sendProgress(float progress) = 0;
  virtual void run() = 0;
  virtual void finish() = 0;

 signals:
  void progress(int progress);
  void finished();
};

class MovementImpl : public Movement {
  Q_OBJECT

 private:  // injected state
  Config*                                    _config;
  State*                                     _state;
  Logger*                                    _logger;
  const std::unique_ptr<device::Stepper>     _stepperX;
  const std::unique_ptr<device::Stepper>     _stepperY;
  const std::unique_ptr<device::LimitSwitch> _limitSwitch;
  const float                                _xStepPerCm;
  const float                                _yStepPerCm;

 private:  // internal state
  const std::unique_ptr<QMutex> _mutex;
  int                           _currentX = 0;
  int                           _currentY = 0;
  int                           _progress = 0;
  bool                          _moveTogether = false;

 private:
  static inline int cmToSteps(int cm, float stepPerCm) {
    float steps = ceil(cm * stepPerCm);
    return (int)steps;
  }

  void reset();
  void setupStepperX();
  void setupStepperY();

 public:
  INJECT(MovementImpl(Config*                    config,
                      State*                     state,
                      Logger*                    logger,
                      device::StepperFactory     stepperFactory,
                      device::LimitSwitchFactory limitSwitchFactory));

  virtual ~MovementImpl();

  virtual const Movement& goTo(const Point& point) override;

 public slots:
  virtual void sendProgress(float progress) override;
  virtual void run() override;
  virtual void finish() override;
};

fruit::Component<Movement> getMovementComponent();
}  // namespace emmerich::mechanisms

#endif
