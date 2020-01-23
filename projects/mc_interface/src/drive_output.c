#include "drive_output.h"

#include "soft_timer.h"
#include "generic_can_msg.h"

#include "can_transmit.h"
#include "critical_section.h"
#include "wavesculptor.h"

static void prv_enable_drive() {
    // alter storage to disable drive-loop
}

static void prv_disable_drive() {
    // alter storage to disable drive-loop
}

static void prv_build_wavesculptor_message(MotorDriveCommand* command, target_speed, target_current) {
    command.motor_velocity = target_speed;
    command.motor_current = target_current;
}

static void prv_handle_drive(SoftTimerId timer_id, void *context) {
    MotorControllerStorage *storage = context;

    GenericCanMsg msg_left = {
        .id = MOTOR_CAN_LEFT_DRIVE_COMMAND_FRAME_ID,
        .dlc = 8u,
        .extended = false,
    };
    GenericCanMsg msg_right = {
        .id = MOTOR_CAN_RIGHT_DRIVE_COMMAND_FRAME_ID,
        .dlc = 8,
        .extended = false,
    };

    const float velocity_lookup[] = {
        [EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL] = 0.0f,
        [EE_DRIVE_OUTPUT_DIRECTION_FORWARD] = WAVESCULPTOR_FORWARD_VELOCITY,
        [EE_DRIVE_OUTPUT_DIRECTION_REVERSE] = WAVESCULPTOR_REVERSE_VELOCITY,
    };

    MotorDriveCommand left_cmd;
    MotorDriveCommand right_cmd;

    target_speed = <target_speed>
    target_current = <target_current>

    if (<state is REVERSE>) {
        target_speed = <throttle>/
    }
    else if (<state is NEUTRAL>)
    {
        target_current = 0.0f;
    }
    else if (<state is DRIVE>)
    {
        target_current = 0.0f  / EE_DRIVE_OUTPUT_DENOMINATOR;
    }
    

    /** Handling message **/
    prv_build_wavesculptor_message(left_cmd, target_speed, target_current);
    prv_build_wavesculptor_message(right_cmd,  target_speed, target_current);

    uint8_t data_left[MOTOR_CAN_LEFT_DRIVE_COMMAND_LENGTH] = { 0 };
    uint8_t data_right[MOTOR_CAN_RIGHT_DRIVE_COMMAND_LENGTH] = { 0 };
    motor_can_left_drive_command_pack(data_left, &left_drive_cmd, sizeof(data_left));
    motor_can_right_drive_command_pack(data_right, &right_drive_cmd, sizeof(data_right));

    memcpy(&msg_left.data, data_left, sizeof(data_left));
    memcpy(&msg_right.data, data_right, sizeof(data_right));

    generic_can_tx(storage->settings.motor_can, &msg_left);
    generic_can_tx(storage->settings.motor_can, &msg_right);

    soft_timer_start_millis(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS, prv_handle_drive, storage, NULL);
}