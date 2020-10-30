/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * This file was originally generated by cantools version 32.19.0 Wed May 22 20:54:00 2019.
 *
 * Modified to only include parts needed for drive commands
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef EINVAL
#define EINVAL 22
#endif

/* CAN ids. */
typedef uint32_t MotorCanDeviceId;
#define MOTOR_CAN_ID_LEFT_MOTOR_CONTROLLER 0x20
#define MOTOR_CAN_ID_RIGHT_MOTOR_CONTROLLER 0x10

/* Frame ids. */
typedef uint32_t MotorCanFrameId;
#define MOTOR_CAN_LEFT_DRIVE_COMMAND_FRAME_ID (0x21u)
#define MOTOR_CAN_RIGHT_DRIVE_COMMAND_FRAME_ID (0x41u)
#define MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID (0x402u)
#define MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID (0x403u)
#define MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID (0x202u)
#define MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID (0x203u)

/* Frame lengths in bytes. */
#define MOTOR_CAN_DRIVE_COMMAND_LENGTH (8u)

/* Extended or standard frame types. */
#define MOTOR_CAN_DRIVE_COMMAND_IS_EXTENDED (0)

/* Signal choices. */

/**
 * Signals in message Right_DriveCommand.
 *
 * All signal values are as on the CAN bus.
 */
typedef struct MotorCanDriveCommand {
  /**
   * Desired motor velocity set point in metres/second.
   *
   * Range: -
   * Scale: 1
   * Offset: 0
   */
  float motor_velocity;

  /**
   * Desired motor current set point as a percentage of maximum current setting.
   *
   * Range: -
   * Scale: 1
   * Offset: 0
   */
  float motor_current;
} MotorCanDriveCommand;

/**
 * Pack message DriveCommand.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int motor_can_drive_command_pack(uint8_t *dst_p, const MotorCanDriveCommand *src_p, size_t size);
