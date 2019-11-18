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

#ifndef APP_H_
#define APP_H_

#pragma once

#include <fmt/format.h>
#include <fruit/fruit.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <QObject>

#include <QApplication>
#include <QString>

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "general_config.h"

#include "config.h"
#include "logger.h"
#include "state.h"

#include "utils/qspdlog.h"

#include "devices/stepper.h"
#include "mechanisms/finger/movement.h"

#include "windows/main_window.h"

namespace emmerich {
class App : public QObject {
  Q_OBJECT
 public:
  virtual int run() = 0;
  virtual ~App() = default;

 public slots:
  virtual void addLogEntry(const QString& msg) = 0;
};

class AppImpl : public App {
  Q_OBJECT
 private:
  const std::unique_ptr<QApplication>       _qApp;
  const std::unique_ptr<ui::MainWindow>     _window;
  std::shared_ptr<Ui::MainWindow>           _ui;
  Config*                                   _config;
  Logger*                                   _logger;
  State*                                    _state;
  const std::shared_ptr<QSpdlog>    _qSpdlog;
  mechanisms::finger::FingerMovementFactory _fingerMovementFactory;

 private:
  void setupLogger();
  void setupSignalsAndSlots();

 public:
  INJECT(
      AppImpl(ASSISTED(int) argc,
              ASSISTED(char**) argv,
              Config*                                   config,
              Logger*                                   logger,
              State*                                    state,
              mechanisms::finger::FingerMovementFactory fingerMovementFactory));
  virtual ~AppImpl() = default;
  inline virtual int run() override { return _qApp->exec(); }

 public slots:
  virtual void addLogEntry(const QString& msg) override;
};

using AppFactory = std::function<std::unique_ptr<App>(int, char**)>;

fruit::Component<AppFactory> getAppComponent();
}  // namespace emmerich

#endif
