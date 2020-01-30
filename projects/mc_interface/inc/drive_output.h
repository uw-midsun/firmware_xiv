#include "motor_controller.h"
#include "status.h"

static void drive_output_enable(MotorControllerStorage *storage);

static void drive_output_disable(MotorControllerStorage *storage);

StatusCode mci_broadcast_init(MotorControllerStorage *controller);