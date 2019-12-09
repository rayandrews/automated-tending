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

#include "services/tending_service.h"

namespace emmerich::service {

TendingServiceImpl::TendingServiceImpl(
    Logger*                    logger,
    State*                     state,
    mechanism::MovementFactory movementMechanismFactory,
    mechanism::RotationFactory rotationMechanismFactory)
    : _logger(std::move(logger)),
      _state(std::move(state)),
      _movementMechanism(movementMechanismFactory()),
      _rotationMechanism(rotationMechanismFactory()) {
  setupMovementMechanism();
  setupRotationMechanism();

  _signalMergeStopped->connect(_movementMechanism.get(), SLOT(stopped()));
  _signalMergeStopped->connect(_rotationMechanism.get(), SLOT(stopped()));
  connect(_signalMergeStopped.get(), &SignalMerge::merged, this,
          &TendingServiceImpl::onStopped);

  // _signalMergeFinished->connect(_movementMechanism.get(), SLOT(finished()),
  //                               Qt::DirectConnection);
  // _signalMergeFinished->connect(_rotationMechanism.get(), SLOT(finished()),
  //                               Qt::DirectConnection);
  // connect(_signalMergeFinished.get(), &SignalMerge::merged, this,
  //         &TendingServiceImpl::onFinish);
}

TendingServiceImpl::~TendingServiceImpl() {
  _movementThread->quit();
  _rotationThread->quit();
}

void TendingServiceImpl::setupMovementMechanism() {
  _movementMechanism->moveToThread(_movementThread.get());

  connect(_movementMechanism.get(), SIGNAL(started()), _movementThread.get(),
          SLOT(start()));
  connect(_movementThread.get(), &QThread::started, _movementMechanism.get(),
          &mechanism::Movement::followPaths);

  connect(_movementMechanism.get(), &mechanism::Movement::finished, this,
          &TendingServiceImpl::onFinish);

  connect(_movementMechanism.get(), &mechanism::Movement::finished,
          _movementThread.get(), &QThread::quit, Qt::DirectConnection);
  connect(_movementMechanism.get(), &mechanism::Movement::stopped,
          _movementThread.get(), &QThread::quit, Qt::DirectConnection);
}

void TendingServiceImpl::setupRotationMechanism() {
  _rotationMechanism->moveToThread(_rotationThread.get());

  connect(_rotationMechanism.get(), SIGNAL(started()), _rotationThread.get(),
          SLOT(start()));
  connect(_rotationThread.get(), &QThread::started, _rotationMechanism.get(),
          &mechanism::Rotation::run);

  connect(_movementMechanism.get(), &mechanism::Movement::finished,
          [this]() { _rotationMechanism->finish(); });

  connect(_rotationMechanism.get(), &mechanism::Rotation::finished,
          _rotationThread.get(), &QThread::quit, Qt::DirectConnection);
  connect(_rotationMechanism.get(), &mechanism::Rotation::stopped,
          _rotationThread.get(), &QThread::quit, Qt::DirectConnection);
}

void TendingServiceImpl::run() {
  _rotationMechanism->start();
  QThread::usleep(200);
  _movementMechanism->start();
}

void TendingServiceImpl::onStart() {
  _logger->debug("Tending Service is starting");
  QThread::msleep(100);
}

void TendingServiceImpl::onStopped() {
  _logger->debug("Tending Service is stopped!");
}

void TendingServiceImpl::onFinish() {
  // set state back to idle
  _state->setMachineState(task_state::IDLE);
  _logger->debug("Tending Service is finished");
}

void TendingServiceImpl::execute() {
  onStart();
  run();
}

void TendingServiceImpl::stop() {
  _movementMechanism->stop();
  _rotationMechanism->stop();
}

fruit::Component<fruit::Annotated<TendingService, Service>>
getTendingServiceComponent() {
  return fruit::createComponent()
      .bind<fruit::Annotated<TendingService, Service>, TendingServiceImpl>()
      .install(getStateComponent)
      .install(getLoggerComponent)
      .install(mechanism::getMovementMechanismComponent)
      .install(mechanism::getRotationMechanismComponent);
}
}  // namespace emmerich::service