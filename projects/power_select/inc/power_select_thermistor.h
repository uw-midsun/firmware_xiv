#pragma once

double resistance_to_temp(double resistance);

#define voltage_to_res(r) 30000.0 / (double)((r) / 1000.0) - 10000
