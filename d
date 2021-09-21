[33mcommit 7171ca8e9e12b5fc1dec28e99e3463a26316385b[m[33m ([m[1;36mHEAD -> [m[1;32msoft_999_Elvis_Cheung_fw_102[m[33m)[m
Author: Elvis <elvis.cheung8@gmail.com>
Date:   Sun Sep 19 00:03:32 2021 -0400

    fw 102

[33mcommit cafeabeeb318ccdf9c042ef617fa696f5c9a0fef[m[33m ([m[1;33mtag: DBC-17-cafeabeeb318ccdf9c042ef617fa696f5c9a0fef[m[33m, [m[1;31morigin/master[m[33m, [m[1;31morigin/HEAD[m[33m, [m[1;32mwip_getting_started[m[33m, [m[1;32mmaster[m[33m)[m
Author: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>
Date:   Thu Sep 16 19:09:12 2021 -0400

    [SOFT-538] Remove references to old firmware repo, add GitHub Actions to README (#439)
    
    - the git hooks were trying to cd into the old firmware repo, update them to firmware_xiv
    - DEVELOPING.md exists, and hadn't been updated since MSXII; update it
    - update README.md to talk about GitHub Actions rather than Travis CI, which has been gone for a while

[33mcommit 9ea126a285cf637ad08bed6eebe576a84829644f[m[33m ([m[1;33mtag: DBC-16-9ea126a285cf637ad08bed6eebe576a84829644f[m[33m)[m
Author: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>
Date:   Thu Sep 16 18:02:19 2021 -0400

    [SOFT-539] Fix CI certificate issue (#440)
    
    For some reason, the fiam/arm-none-eabi-gcc action in CI was failing with a certificate issue. To fix it, we just install the arm-none-eabi toolchain ourselves instead of using the third-party action.
    
    Also, added a copy of the nanopb license to libraries/nanopb so we don't violate any license terms.

[33mcommit 4533b39ca44c74fefc166d915bbf4d15a187bbd3[m
Author: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>
Date:   Wed Sep 15 18:56:48 2021 -0400

    [SOFT-536] Fix flaky lights_signal_fsm test (#438)
    
    test_lights_signal_fsm_sends_sync_msgs was occasionally failing in CI. This attempts to fix that by increasing the buffer between when a blink event should be raised and when we check that it is raised.

[33mcommit 7419fe32ea65fa9e830876b6618d60dba4df26f6[m[33m ([m[1;33mtag: DBC-14-7419fe32ea65fa9e830876b6618d60dba4df26f6[m[33m)[m
Author: ShiCheng-Lu <79655657+ShiCheng-Lu@users.noreply.github.com>
Date:   Fri Sep 10 22:09:42 2021 -0400

    added nanopb library and adjusted make files to ignore nanopb library in linting/formatting. (#434)
    
    * added nanopb library
    
    * added quick-lint ignore

[33mcommit 50ed21fe0117b44388b0f3ea378d2f7f160774de[m[33m ([m[1;33mtag: DBC-13-50ed21fe0117b44388b0f3ea378d2f7f160774de[m[33m)[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Thu Sep 9 21:34:24 2021 -0400

    [SOFT-514] Improve MCI status broadcast (#433)
    
    This improves the MCI status broadcast so it sends more useful info.
    
    In short, SYSTEM_CAN_MESSAGE_MOTOR_STATUS now sends each WaveSculptor's status/limit flags, as well as any board overtemperatures and overtemperatures from the WaveSculptor's temperature messages (to be added in SOFT-534).

[33mcommit bb06c5f88ded611923123f04a6b0315c3dc9db09[m[33m ([m[1;33mtag: DBC-12-bb06c5f88ded611923123f04a6b0315c3dc9db09[m[33m)[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Thu Sep 9 20:45:16 2021 -0400

    [SOFT-495] Migrate bootloader from protobuf-c to nanopb (#426)
    
    Nanopb is a more embedded-friendly protobuf implementation, so we use it instead of protobuf-c.

[33mcommit ebadd85e42adc7c8e9a4ce654c2cfae986e043ad[m[33m ([m[1;33mtag: DBC-11-ebadd85e42adc7c8e9a4ce654c2cfae986e043ad[m[33m)[m
Author: ShiCheng-Lu <79655657+ShiCheng-Lu@users.noreply.github.com>
Date:   Sun Aug 29 22:06:20 2021 -0400

    [SOFT-530] Datagram testing module (#430)

[33mcommit 7805bd416fc2a2843259a362ea824487fab73fc1[m[33m ([m[1;33mtag: DBC-10-7805bd416fc2a2843259a362ea824487fab73fc1[m[33m)[m
Author: YiJie Zhu <64090163+YiJie-Zhu@users.noreply.github.com>
Date:   Sat Aug 28 12:39:40 2021 -0400

    [SOFT 421] Transmit motor temp (#408)
    
    This adds transmitting motor temperature over CAN from MCI.

[33mcommit 401a05e912d25e4c199d13c87fd0ef78a9fbe4e5[m[33m ([m[1;33mtag: DBC-9-401a05e912d25e4c199d13c87fd0ef78a9fbe4e5[m[33m)[m
Author: Vaaranan Yogalingam <69359660+vaaranan-y@users.noreply.github.com>
Date:   Sat Aug 28 12:19:12 2021 -0400

    [SOFT-527] Update NTC thermistor curve (#428)
    
    It now obeys the experimental curve rather than the theoretical one (commented out). See https://www.desmos.com/calculator/fqoelc5mkj.

[33mcommit 41ce0809e4a763232bbbd63c54cb718004ffdd3c[m[33m ([m[1;33mtag: DBC-8-41ce0809e4a763232bbbd63c54cb718004ffdd3c[m[33m)[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sat Aug 7 18:40:01 2021 -0400

    [SOFT-516] Update steering: DRL behaviour (#425)

[33mcommit 8cdcaa08863e426a2340c7aa9ab40c4a89ffb071[m[33m ([m[1;33mtag: DBC-7-8cdcaa08863e426a2340c7aa9ab40c4a89ffb071[m[33m)[m
Author: ShiCheng-Lu <79655657+ShiCheng-Lu@users.noreply.github.com>
Date:   Thu Aug 5 22:00:02 2021 -0400

    [SOFT-507] Bootloader ping (#417)
    
    The ping operation for the bootloader firmware + dispatcher for the datagrams for the bootloader project.

[33mcommit f8663efe812de4e1a923a275a23b9933bf3a287a[m[33m ([m[1;33mtag: DBC-6-f8663efe812de4e1a923a275a23b9933bf3a287a[m[33m)[m
Author: Keith Choa <keithchoa@gmail.com>
Date:   Sat Jul 31 11:58:00 2021 -0700

    [SOFT-431] Python CAN Datagram Library (#405)

[33mcommit d7819e68a80bf245bdf9a985c6f1efdef85e5baf[m[33m ([m[1;33mtag: DBC-5-d7819e68a80bf245bdf9a985c6f1efdef85e5baf[m[33m)[m
Author: ShiCheng-Lu <79655657+ShiCheng-Lu@users.noreply.github.com>
Date:   Sat Jul 31 11:02:30 2021 -0400

    [SOFT-511] Datagram node 0 behaviours (#424)
    
    If 0 is in a destination_nodes of a datagram, the datagram is received by all.
    If 0 is the node id, it receives all datagrams.

[33mcommit c49fcda8cb923e16a1d219508a992632c0be20c0[m[33m ([m[1;33mtag: DBC-4-c49fcda8cb923e16a1d219508a992632c0be20c0[m[33m)[m
Author: Faraz Khoubsirat <58580514+farazkh80@users.noreply.github.com>
Date:   Wed Jul 28 18:49:59 2021 -0700

    [SOFT-452] Regen braking toggling: MCI (#416)
    
    Implemented the regen braking on MCI side by
     - Adding a regen braking state in regen_braking.c
     - Making test cases in test_regen_braking.c to ensure the functionality of regen_braking.c
     - Adding the internal regen_brake state to mci_output.c to set the current to zero when regen is diabled and the target velocity is less than the current velocity.
     - Added additional test cases to 'test_mci_output.c` to test all driving and pedal modes both when regen barking is enabled and disabled.

[33mcommit 985d582e9e9f628d4cd908b57607bf356d03149f[m[33m ([m[1;33mtag: DBC-3-985d582e9e9f628d4cd908b57607bf356d03149f[m[33m)[m
Author: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>
Date:   Sun Jul 25 19:14:48 2021 -0400

    [SOFT-513] Fix steering pins (#423)
    
    * Fix pins in steering and smoke_steering
    
    * Switch smoke_steering names + interrupt edges to match steering
    
    These were updated on steering a while ago but smoke_steering wasn't
    updated for it.

[33mcommit 267896bcabaace3fee7c75b2f0afbf7c8f23a047[m[33m ([m[1;33mtag: DBC-2-267896bcabaace3fee7c75b2f0afbf7c8f23a047[m[33m)[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sat Jul 24 12:35:41 2021 -0400

    [SOFT-504] Change front PD adc read scale (#420)
    
    For front PD, we've currently got a linear scale with 0 to the ADC reference voltage mapping to 0-100% fan speed. Per Aashmika and onsite testing, we need 0-1.65V to map to 0-100% fan speed. (1.65V or 1650mV should be a configurable parameter in a #define at the top of the file.) Also,
    > if it is 3.3V the fans should be 0% or off (because the spst (switch) is open)
    But the ADC can only detect up to its reference voltage, which is ~2V or so usually. So instead say that if the voltage is >1.65V (by some epsilon, maybe 50mV) and within an epsilon (again maybe 50mV) of min(reference voltage, 3.3V) then say that the switch is open and shut the fans off.

[33mcommit 70d2d994e57082ad8ff643c77c4f8683f244a9bd[m[33m ([m[1;33mtag: DBC-1-70d2d994e57082ad8ff643c77c4f8683f244a9bd[m[33m)[m
Author: benlin1221 <47359563+benlin1221@users.noreply.github.com>
Date:   Fri Jul 23 13:57:27 2021 -0400

    [SOFT-459] Add dbc releaser action (#422)
    
    Creates a release containing the system_can.dbc file under tag DBC-[action run #]-[commit id], for every push to master.

[33mcommit 11d7e564592a14c6afa2835d1404ed73cc54a17e[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Wed Jul 21 20:31:15 2021 -0400

    [SOFT-275] MCI Fan Control (#419)
    
    Fan control module for MCI. The way this works is pretty simple -- there's a single pin, MCI_FAN_EN_ADDR, that is used to turn on/off the fans. This is abstracted by mci_fan_set_state(). Hardware wants the fan state based on the temperature measurements from the motor controllers, so that's for a future ticket.
    
    There are also four pins that go high when the corresponding thermistor on the MCI board hits a certain threshold. Eventually we should log these over CAN using SYSTEM_CAN_MESSAGE_MOTOR_STATUS, but for now we just track their status in a fault bitset that can be exported using mci_fan_get_fault_bitset(). A fault callback can also be passed in on initialization, and will be called each time the fault bitset updates.
    
    The test sequence for this is pretty short, as the module itself is quite simple.

[33mcommit cbc957a36eee29c37f180fcd8cfa72ef73358be2[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Sat Jul 17 13:05:47 2021 -0400

    [SOFT-500] Add test coverage (#415)

[33mcommit dab0a5335a64de4c7baf7d2f79d20ba07750db9e[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Sat Jul 17 12:18:06 2021 -0400

    [SOFT-491] Add parking brake sensor values (#418)

[33mcommit c82f2b98a98820e299c2869eabb9aee7a8f4ea0c[m
Author: Michael Ding <m28ding@uwaterloo.ca>
Date:   Sat Jul 17 10:20:44 2021 -0400

    [SOFT-503] Address event queue overloading on PD (#411)
    
    * data is TXed every 1.5s rather than every 0.5s
    * 4 messages are TXed at a time
    * messages are TXed 200ms apart rather than 100ms

[33mcommit 84e470aa7cc099cd421036d2e859c4fd9ecc31e5[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Sat Jul 10 14:01:26 2021 -0400

    [SOFT-183] smoke steering (#253)
    
    * Started smoketest with checking for digital input
    
    * Added functionality for control stalk
    
    * Ported some changes from soft 151
    
    * Fix smoke_steering based on onsite testing
    
    * merge master steering into branch
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit dcda2d08f03748c08ebe40fbc1a6dca9c9cbc53e[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Wed Jul 7 15:55:25 2021 -0700

    [SOFT-415] CAN Datagram: C (#375)
    
    This adds the CAN datagram library for firmware. See https://uwmidsun.atlassian.net/l/c/6M123FHA.

[33mcommit 371b30f0d9ea77c33f0223eb059a40603dc179a6[m
Author: Henry Zhou <43188301+hzhou0@users.noreply.github.com>
Date:   Sat Jul 3 11:59:10 2021 -0400

    [SOFT-434] Add interrupt function to PCA9539R driver (#288)
    
    Co-authored-by: Eddy Kim <Eddy.M.K@outlook.com>
    Co-authored-by: Frank Yan <frankyan2351@gmail.com>
    Co-authored-by: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>

[33mcommit ed9f02f91012abb2bce40f8a06ded6bd83a64d04[m
Author: Daniel Ye <danielye4@gmail.com>
Date:   Wed Jun 30 19:02:49 2021 -0400

    [Soft 483] Regen braking toggling: centre console (#390)
    
    This adds the regen braking toggling handling logic to centre console.

[33mcommit 2c2e012b35e2f4590712a1d85453d9599607087a[m
Author: Faraz Khoubsirat <58580514+farazkh80@users.noreply.github.com>
Date:   Wed Jun 30 15:43:44 2021 -0700

    [SOFT-461] Assert when solar relay opens (#399)
    
    When the solar relay gets opened, we wait an amount of time then assert that it really was opened. We declare a failure and send a `EE_SOLAR_RELAY_OPEN_ERROR` fault if the current through the array is greater than a threshold, there was a previous DRV120 relay error, or the current isn't set after a while. Otherwise, we send a "ready to drive" message.

[33mcommit c182b732eb98cc9d9d35dc2dd88e1fb9a8b03ccd[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Wed Jun 23 20:56:33 2021 -0400

    [SOFT-498] Ready-to-drive CAN message (#403)

[33mcommit 214d33e010109bbebee6cb879c5c09e4bd954aa1[m
Author: Daniel Zeng <78101830+PurpleRhythms@users.noreply.github.com>
Date:   Wed Jun 23 19:51:43 2021 -0400

    [SOFT-362] Removing ebrake from centre_console (#389)
    
    Current issue:
    drive_fsm not transitioning from neutral discharged state to set precharge despite this identical transition being made multiple times beforehand successfully. In fact, the drive_fsm does not transition at all after receiving a drive event. Event queue is not overflowing and neutral_discharge_state can only transition to set_precharge (in all four events drive, reverse, parking, and neutral). Neutral discharged is also the initial state and has no callback function which could complicate the state transition. Furthermore, the transition of neutral discharge to set precharge through a drive event was not modified during this ticket and should not have failed.
    
    Description:
    ebrake_tx.c alongside its header file & tests are deleted.
    
    "Cascade deleted" instances of ebrake including ebrake states, enums, and events.
    
    Restructured drive_fsm to remove set_ebrake state. See: https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/2687074370/Drive+Finite+State+Machine+drive+fsm
    
    I noticed a possible enum numbering issue where multiple enums in centre_console_events.h would have the same numberation.

[33mcommit b1601855d0ed393b3946ea2ab7f307887bc5cf3e[m
Author: Michael Ding <mding8166@gmail.com>
Date:   Wed Jun 16 20:36:23 2021 -0400

    [SOFT-482] Steering: regen brake toggling (#372)

[33mcommit 9c9180e7d696a1f19509f99d8b44082c31fba1fe[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Wed Jun 9 19:59:34 2021 -0400

    [SOFT-463] Add interrupt to can_hw_receive (#393)
    
    This fixes the wait tests getting stuck and taking forever!

[33mcommit 30fa0974b20f157880cbdaed472859e10b55c69d[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Mon Jun 7 20:07:59 2021 -0400

    [SOFT-496] Fix tutorial board adc (#392)

[33mcommit fed4e3a86afcb5dd8050cf41c138b7c4f6f3d411[m
Author: Abdullah Shahid <74064210+abdullahs26@users.noreply.github.com>
Date:   Wed Jun 2 22:43:00 2021 -0400

    [SOFT-354] Babydriver: I2C read firmware (#368)
    
    This adds the firmware implementation for the `i2c_read` Babydriver function, completing Babydriver!

[33mcommit 1ae6a6df61bc745218aafc955979b7a2d15ae55c[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Sat May 29 15:16:44 2021 -0400

    [SOFT-485] Create MCI readme (#384)

[33mcommit 46ee60a20ab58104e22a1880299e05c4bb3cecd5[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Sat May 29 14:47:07 2021 -0400

    [SOFT-481] Update wavesculptor.h for WaveSculptor 22 (#381)
    
    There were some outdated message definitions in wavesculptor.h (I assume from when we used a WS20 motor controller). This updates them to work with the WS22:
    * Remove WaveSculptorFanSpeedMeasurement, WaveSculptorAirInCpuTempMeasurement, WaveSculptorAirOutCapTempMeasurement
    * Add WaveSculptorDspTempMeasurement (I believe this is similar to WaveSculptorAirInCpuTempMeasurement), WaveSculptorSlipSpeedMeasurement, WaveSculptorActiveMotorChange
    * Update other message internals where needed
    * Define message offsets in hex to match format in WS user manual
    * Update references

[33mcommit 52bee7de165e0eef16809d0401c499867871fece[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Wed May 26 17:21:55 2021 -0700

    [SOFT-381] Add mcp3427 MU support and Solar sim (#379)
    
    Added Mcp3427 driver sim with tests, allowing us to write the values that the adc will read (on both channels) as well as trigger the fault callback.
    
    As it is an adc, there is no need to assert values, only write them, and the test verifies it is working by the smoketest output matching what is expected.
    
    Also added solar sim in same format as bms carrier, as from what I can see this is where the driver is used

[33mcommit 7a18e6d285d616be22bc4e5cc1f163f844ee6496[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Sat May 22 14:23:16 2021 -0400

    [SOFT-446] Bootloader protos makefile changes and added .proto files (#378)

[33mcommit 945f18d926bb8d4485454026abf2990b1815adf5[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Sat May 22 12:40:21 2021 -0400

    [SOFT-457] Added pd readme (#371)
    
    * Added pd readme
    
    * Resolved PR comments
    
    Co-authored-by: Ryan Dancy <41841896+ryandancy@users.noreply.github.com>

[33mcommit 25658532df95f716968547552a09bcb170a7a17b[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Sat May 22 12:29:14 2021 -0400

    [SOFT-475] Broadcast low battery warning (#380)
    
    * Add warning bitset and cell voltage to PowerSelectStorage
    * Read voltage at PB1_3V3_CELL, store, and set POWER_SELECT_WARNING_BAT_LOW flag in warning bitset if < 2100 mV
    * Change SYSTEM_CAN_MESSAGE_POWER_SELECT_FAULT to SYSTEM_CAN_MESSAGE_POWER_SELECT_STATUS, containing fault, valid, and warning bitsets as well as 3V3 cell voltage
    * Replace STATUS with MEAS in other power select CAN message names for clarity
    * Rename PowerSelectFault macros to start with POWER_SELECT_FAULT for clarity
    * Add a test case to make sure cell low warning works OK
    * Some other minor naming changes/cleanup

[33mcommit 4fb4012c04ac0cac6ab83b59b446fd264524127a[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sun May 16 15:04:50 2021 -0400

    increased timeout for test_blink_event_generator (#374)
    
    * increased timeout
    
    * add id to build step
    
    Co-authored-by: Avery Chiu <averychiuu@gmail.com>

[33mcommit da31d4fb5bed16a55d6d43c462b98ae620dccf69[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Sat May 15 14:05:13 2021 -0400

    [SOFT-341] Power Select Rev 2 (#348)
    
    See https://uwmidsun.atlassian.net/l/c/uEtwKYk6.
    
    Brief summary of what this module (should) do:
    * Periodically read voltages and currents from all three power sources, as well as temperature from aux/DCDC
    * Broadcast these values
    * Check for whether any of the readings represent a fault. If so, turn off the LTC and broadcast the fault bitset under SYSTEM_CAN_MESSAGE_POWER_SELECT_FAULT
    * Handle interrupts from the DCDC fault pin and perform same fault handling as above
    * Handle SYSTEM_CAN_MESSGAGE_POWER_ON_MAIN_SEQUENCE messages by checking AUX or DCDC status
    * Handle SYSTEM_CAN_MESSGAGE_POWER_ON_AUX_SEQUENCE messages by checking AUX status

[33mcommit 6221aa0cbb93497a97cc7697571ab2c59cb99b7f[m
Author: Hewitt McGaughey <hewittmcgaughey@gmail.com>
Date:   Thu May 13 15:16:00 2021 -0400

    [SOFT-353] MCI updates from validation (#367)
    
    This finally cleans up the changes I made to get MCI working with the MCs from ~October - March.
    
    # MCP2515 fixes
    The main issue I was running into was with the MCP2515 that communicates with the motor controllers. With the old implementation (using a GenericCan interface), if it received multiple CAN messages from different IDs it would lock up for some reason and stop processing messages. The following changes solve this:
    - Use the mcp2515 driver directly in mci_broadcast.c to filter for one message ID at a time
    - Add mcp2515_set_filter() to mcp2515.c to allow for changing the filters without having to reinitialize everything. This more or less just copies the parts of mcp2515_init() that configure the filters
    - Generally, whenever a message is received from a MC, call its associated callback and change the filter to the next message we're looking for
    - Sometimes messages that we're not filtering for get through (???), so while doing the above we also check to make sure that the message received is the one we were looking for and don't change the filter if not
    - Changed mci_output.c to send messages to the MCs using the MCP2515 driver instead of generic CAN
    
    # Other changes
    - Added functionality to receive and store status info from the MCs
    - Added callbacks for motor temp and DSP temp measurements. Storing and broadcasting these values should be done in SOFT-421
    - Added a CAN message, SYSTEM_CAN_MESSAGE_MOTOR_STATUS, that just broadcasts the error/limit flags from both MCs' status info messages
    - Changed wavesculptor.h velocity definitions from 100/-100 rpm to 20000/-20000 rpm so we can control the motor using current
    - Updated tests to work with above changes while keeping as much of the original logic as possible
    
    # Notes
    - The MCP2515 still behaves weirdly sometimes, so I want to take a closer look at the driver when there's time and we're allowed to go onsite again
    - wavesculptor.h needs to be updated for the WS22, see SOFT-481
    - All these changes were tested with the actual motor at the end of March and seemed to work OK, could turn it on/off and run it at a constant speed by mocking pedal messages

[33mcommit 51eae59e8c33e2804e4e994beb4f078905e3ff30[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Thu May 13 10:59:21 2021 -0400

    [SOFT-484] Implement viewing and updating stores (#370)
    
    * Implement viewing and updating stores
    
    * Implement sim io with getters and setters
    
    * Clean up

[33mcommit 5a37a853f5d77fd478b66dbf87c35824771cb5ef[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Sat May 8 22:15:10 2021 -0400

    [SOFT-478] Created README for centre console (#365)

[33mcommit 76d643c067cdde144c70dfbfcc6e2eca7476e940[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Fri May 7 12:06:27 2021 -0400

    [SOFT-474] Muctl basics (#366)
    
    * Implement start stop list cat reset api
    
    * Add logging to muctl
    
    * Clean up things
    
    * Allow default projects in sims
    
    * Clean up and ergonomify
    
    * Fix pylint errors

[33mcommit ca19cee2d1cf3df223ee4c7282b74d470526cf98[m
Author: Daniel Ye <danielye4@gmail.com>
Date:   Thu May 6 23:41:27 2021 -0400

    [SOFT-447] Bootloader CAN ID handling (#358)
    
    This adds a bootloader_can ms-common module that implements the bootloader's CAN ID allocation scheme as described at https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/2030895112/Bootloader%2BFlash%2Bover%2BCAN#CAN-ID-allocation.

[33mcommit 5b70b72b777309f8c6d42e134bbe01005108d94d[m
Author: Daniel Zeng <78101830+PurpleRhythms@users.noreply.github.com>
Date:   Thu May 6 13:56:41 2021 -0400

    [SOFT-456] Brake Light Control (#356)
    
    In centre console, pedal_monitor.c receives updates from pedal and decides whether the pedal is pressed or released. We add an event for the pedal state change and use this to tell power distribution when to turn the brake light on and off through a brake_light_control module.
    
    To get PD to turn the brake light off, we transmit a SYSTEM_CAN_MESSAGE_LIGHTS with the lights ID (first u8) as EE_LIGHTS_TYPE_BRAKES and the state (second u8) as either EE_LIGHTS_STATE_OFF or EE_LIGHTS_STATE_ON.
    
    To save bandwidth on the CAN network, we only transmit this when the state changes.
    
    Co-authored-by: FourRhythm <FourRhythm@users.noreply.github.com>

[33mcommit 98694897d012e9546853400084d334bd74532c36[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue May 4 18:42:04 2021 -0400

    [SOFT-167] Fix a bunch of solar validation bugs (#363)
    
    These were actually discovered and fixed in like September, for some reason I never put up a PR for them. So here they are.
    
    Changes:
    * Increase the max MCP3427 conversion time which we wait before checking for a conversion, because I think it was throwing too many errors.
    * Actually assign the sample rate from the settings to the storage in MCP3427, we weren't doing that before and it was defaulting to 16 bits (slowest and most precise).
    * Fix an endianness issue in the SPV1020 driver, it turns out the SPV1020 is big-endian while the STM32 is little-endian. * Plus, relying on the endianness of the host system is a bad idea anyways.
    * Increase the SPI baudrate from 60000 to the 6000000 value we use everywhere else in the SPI and SPV1020 smoketests, plus a few tests and solar itself.
    * A couple MCP3427 address configs were misconfigured in solar, fix those.
    * The I2C1 and I2C2 pins were swapped in the MCP3427 smoketest and in solar. Fixed that by using the standard macros from controller_board_pins.h instead.

[33mcommit 0757e98dc3c154f9f84ffe76dc5d9398db175539[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Mon May 3 20:43:18 2021 -0400

    [SOFT-233] Fix typo in CI script (#362)
    
    I typo'd DEFINES instead of DEFINE when setting up the GitHub Actions CI script. That was causing STM32 builds to build with log-level debug instead of warn as it should be. Thanks @hewittmcg for finding this.

[33mcommit 1553ee14b1543bb25d8ecbd5ea9831ea481ea891[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Sun May 2 23:15:26 2021 -0400

    [SOFT-380] Fix buffer overflow in MU logging (#360)
    
    The MU LOG implementation was using a static buffer to store logs to be exported. This was causing issues in PD, where use of the BUG macro would cause strings to exceed the 256-byte buffer (and gcc would actually detect this at compile time!).
    
    To fix this, we get rid of the buffer entirely by using asprintf, a GNU extension which works like snprintf except it mallocs enough memory for the resulting string automatically. This does mean using the heap, but that's ok on MU since it's only for x86. Plus, we now don't have a `static char s_log_buf[256]` polluting the namespace of every file that includes log.h on MU.

[33mcommit 104c787ad690fda7f08ae314e37a1fa4eda0c63e[m
Author: Faraz Khoubsirat <58580514+farazkh80@users.noreply.github.com>
Date:   Sun May 2 15:29:43 2021 -0400

    [SOFT-450] Babydriver integration test (#336)
    
    This adds an MU integration test to verify that Babydriver's Python and C sides are compatible.

[33mcommit 95c75eea8a41c7c44f12cd7485a38e1e80304b92[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat May 1 09:58:03 2021 -0400

    [SOFT-470] musrv setup (#353)
    
    * Add installation via symlink
    
    * set up musrv/muctl command line
    
    * Parse args
    
    * Clean up config and error handling and some logging
    
    * Add pm log tailing from muctl
    
    * Give ReqHandler a ref to pm
    
    * Fix some light bugs and improve help messaging

[33mcommit e52b84905295ddc5f7aebb2ea297267717a11d09[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Fri Apr 30 19:23:09 2021 -0400

    [SOFT-473] Check for uncommitted codegen changes in CI (#355)
    
    If there are uncommitted codegen changes, we post a PR comment and fail early instead of mysteriously failing to compile.

[33mcommit 09132720939ac91d20793309690406ba188f52d9[m
Author: YiJie Zhu <64090163+YiJie-Zhu@users.noreply.github.com>
Date:   Fri Apr 30 17:41:18 2021 -0400

    [SOFT-455] Implement chip id (#349)
    
    STM32s have a factory-set 96-bit unique chip ID in a system register. See section 33.1 of the STM32F0xx manual.
    
    This could be useful in some contexts (e.g. maybe the bootloader), so this adds an ms-common module which retrieves the chip ID and exposes it as a custom data type.
    
    Co-authored-by: Jackie Zhu <64090163+jckpokimain@users.noreply.github.com>

[33mcommit ec67c4363983b378627994b6d372ff202ae9f4b0[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Thu Apr 29 19:03:46 2021 -0400

    [SOFT-469] Create steering readme (#351)

[33mcommit 065e2d36540e68dd5c7a470ecd8ad063b807ac1e[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Thu Apr 29 18:32:26 2021 -0400

    [SOFT-471] Add readme for BMS carrier (#352)

[33mcommit d2432527ccffd0ba33d47f5a5e5918ee5038c739[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Thu Apr 29 16:52:03 2021 -0400

    [SOFT-158] Fix a lot of bugs (#354)
    
    * Fix conceptual error in gpio MU: use input_pins correctly
    * Fix memory leak in PCA9539r init pin: if already initialized, free the old store and use the new one
    * Prevent initializing multiple stores with same key in store.c
    * Add test for initializing PD to be the correct one

[33mcommit 3cf4804d77c6d49d7a79f37405115dd260605561[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Apr 28 22:05:01 2021 -0400

    [SOFT-466] Experimental stack analyzer (#350)
    
    Chromium OS's Embedded Controller (EC) team has a super cool open source stack size analyzer tool which analyzes disassembly to find the maximum possible stack size. This copies and adapts the EC stack analyzer for our firmware. (It's BSD-licenced, so we're fine.) There's a good summary of the stack analyzer's capabilities in platform/stm32f0xx/scripts/stack_analyzer/README.md.
    
    To make the stack analyzer work without spewing ?? for every standard library symbol, this also adds bundled newlib binaries with debug symbols enabled. (They took like 6 hours to build on my machine!) Newlib is the implementation of the C standard library we use since it's designed for embedded stuff; usually we just use the one bundled with gcc-arm-embedded, but it has debug symbols stripped.
    
    Using the custom newlib binaries could also be generally useful for debugging with gdb, so we add an STDLIB_DEBUG make option which you can set to true to build with debug symbols enabled.
    
    Most of the actual stack_analyzer.py script is a) super long and complex and b) written by google engineers and not me, so it shouldn't have to be reviewed - if you want to search for the parts I wrote, I marked most of them with MidSun extension or something like that.

[33mcommit 021c4242d5ecdc53c989ad4fda694a7ef155f1b1[m
Author: Daniel Zeng <78101830+PurpleRhythms@users.noreply.github.com>
Date:   Wed Apr 28 21:48:40 2021 -0400

    [SOFT-349][SOFT-425] Fan control for solar (#327)
    
    * [SOFT-349] Store MPPT status instead of just CR bit
    
    The data store previously stored just the mysterious CR bit, which is
    the last bit of the MPPT status. For fan control we need other parts of
    the status. It's easiest to just store the whole thing and extract parts
    as needed with the spv1020_is_* functions from the SPV1020 driver. So
    now we store the whole status in the data store instead of just the CR
    bit.
    
    * [SOFT-349] Fan control for solar
    
    This adds the fan_control_solar module to solar, which does two things
    to control the fans on solar:
    - raise a fault whenever the fan fails or goes overtemperature
    - turn the fans to full speed whenever an mppt or a thermistor reports
      overtemp
    
    Co-authored-by: Ryan Dancy <ryan@keal.ca>
    Co-authored-by: Patrick Kim <s524kim@uwaterloo.ca>
    Co-authored-by: FourRhythm <FourRhythm@users.noreply.github.com>
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 6a926dd16627291582a4d0ca2a27f6f14b0e6e86[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Apr 24 16:24:30 2021 -0400

    [SOFT-465] Separate logging in MU (#345)
    
    * Implement logger
    
    * Separate logging
    
    * Clean up logger

[33mcommit e6043fdd7d0472691fef4f8df9cf6f86cb348f8c[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Apr 24 15:33:07 2021 -0400

    [SOFT-233] Set astroid version (#346)

[33mcommit 49bc0df6e15df4aef9105a9f4b5831947200063f[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Fri Apr 23 13:33:23 2021 -0400

    [SOFT-464] Invert project-sims relationship (#344)
    
    * WIP most tests work except gpio expanders and test_init_conds
    
    * All tests passing
    
    * Make StoreUpdate use a tuple key rather than two arguments
    
    * misc refactoring
    
    * Change sub_sim access api
    
    * Make SubSim an ABC
    
    * Fix linting

[33mcommit 4c0d57b237e824bf0a6e910cb76b87a9cf4951d6[m
Author: dependabot[bot] <49699333+dependabot[bot]@users.noreply.github.com>
Date:   Fri Apr 16 22:47:01 2021 -0400

    Bump pyyaml from 5.1 to 5.4 (#343)
    
    Bumps [pyyaml](https://github.com/yaml/pyyaml) from 5.1 to 5.4.
    - [Release notes](https://github.com/yaml/pyyaml/releases)
    - [Changelog](https://github.com/yaml/pyyaml/blob/master/CHANGES)
    - [Commits](https://github.com/yaml/pyyaml/compare/5.1...5.4)
    
    Signed-off-by: dependabot[bot] <support@github.com>
    
    Co-authored-by: dependabot[bot] <49699333+dependabot[bot]@users.noreply.github.com>

[33mcommit 9e3aeab33ee0d2a3aad3d0afc47a6ead30eea483[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Fri Apr 16 20:24:53 2021 -0400

    [SOFT-445] Centralized pip dependencies (#311)

[33mcommit efee793534511c6384fa699fd6d0ff51984f2e65[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Tue Apr 13 20:03:27 2021 -0400

    [SOFT-462] Rename MPXE to MU (#341)

[33mcommit 839155523cdfeffc2697ab561fc85ba9631ddb13[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Mon Apr 12 22:17:25 2021 -0400

    [SOFT-443] Fix bps heartbeat test with MS_TEST_ASSERT_EQUAL_AT_TIME (#326)
    
    This adds the MS_TEST_HELPER_ASSERT_EQUAL_AT_TIME macro, which tests that a pointer is equal to a value at a time (+/- 10ms).

[33mcommit 52fe7473c6a37205c865d39ee06625c90a7fa431[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Mon Apr 12 20:50:39 2021 -0400

    [SOFT-233] Adjust timings for MPXE (#340)

[33mcommit 031bec529089e9ede2ee2bd33c209d54ddc8c6fd[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Sun Apr 11 12:13:12 2021 -0400

    [Soft-345] startup conditions with unified pipes (#321)
    
    * Cmd created, able to send and receive
    
    * Init lock working correctly, initial conditions added
    
    * ALL TESTS PASSING BABYYYYYY
    
    * INITIAL CONDITIONS WORK, WITH MULTIPLE DRIVERS HELL YEAH
    
    * fix lint
    
    * cleanup time
    
    * another fix
    
    * Init condition locking working :)))) and command logic updated
    
    * Fix pr issues
    
    * Fix lint
    
    * Mci is broken - gpio set in sim.py never received in store.c poll thread, means that read complete signal never sent and mutex in project.write never unlocks. All others work though
    
    * fix lint
    
    * [SOFT-345] Write on main thread, and minor refactoring (#338)
    
    - MCI handler uses timers instead of blocking the poll thread
    - Project gets a ref to PM instead of needing it passed as an arg sometimes
    - All writes go through main thread via a queue and spaghetti concurrency
    
    * fix lint, all tests working!
    
    * update readme, remove debug comments
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit aa5576e3616c76f556ec010f05fdce8e979107dc[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Apr 8 22:32:38 2021 -0400

    [SOFT-460] Fix logging in MPXE (#337)
    
    We previously wouldn't initialize store in the MPXE LOG implementation. This caused segfaults when using babydriver with MPXE, because it calls LOG_DEBUG first thing before any initialization. Fix by calling store_config() in log_export so it's initialized for LOG.

[33mcommit 6a7501b7f976bbbd0d5b430fe8dae7a3ca14605a[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Apr 6 08:49:44 2021 -0400

    [SOFT-458] Add BUG macro for initialization debuggability (#332)
    
    BUG wraps a call and logs a bunch of debug information before exiting immediately if the call returns non-OK.
    It's meant to wrap initializations where the only sensible thing to do on failure is to stop immediately.

[33mcommit f451be7a6f7b3dfac08275aae89855b1562d41cb[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Mon Apr 5 21:14:27 2021 -0400

    [SOFT-234] Merge codegen into firmware (#310)
    
    See https://uwmidsun.atlassian.net/l/c/HUVz1yu7.
    This merges https://github.com/uw-midsun/codegen-tooling-msxiv into this repository.

[33mcommit 8378abab6c22a8d26de3e4893d1b0cbb9f662a55[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Sat Apr 3 21:39:39 2021 -0400

    [SOFT-396] General power distribution update to rev 3.1 (#330)
    
    Changelog:
    * Architectural change: move all the hardware configuration into an `output` module and refactor the rest of PD to work in terms of outputs. Now PD is more pleasant to work with!
    * Changes to update to PD rev 3.1:
      - On front, use UV cutoff fan output instead of the fan BTS7200
      - Change horn to a GPIO output since it's now on UV cutoff
      - Add a UV_VBAT output to read current from the UV cutoff battery voltage
      - Move around lots of pins to match rev 3.1
      - Confirm rear fan outputs with hardware
    * Functional changes:
      - Remove SYSTEM_CAN_MESSAGE_FRONT_POWER and all the pd_gpio outputs that used it
      - Check all the status codes in main.c and print helpful debugging messages
      - Remove `ack` field in can_rx_event_mapper since it doesn't do anything
      - TX a fault CAN message if there's a BTS7200/BTS7040 fault, and add fields to facilitate it
      - Fiddle with publish_data parameters to try and avoid overwhelming the event queue
      - Put can_rx_event_mapper events on a higher priority so they don't get lost
      - Add a bunch of divide-by-zero safeguards in pd_fan_ctrl
    * Stylistic changes:
      - Remove the abhorrent POWER_DISTRIBUTION prefix from everywhere once and for all

[33mcommit 8ef6ce62ad146e003296240ddec7f209ac8b2b95[m
Author: YiJie Zhu <64090163+YiJie-Zhu@users.noreply.github.com>
Date:   Sat Apr 3 20:51:46 2021 -0400

    [SOFT 453] Update steering to rev 4.0 (#329)
    
    - Implement regen-braking toggle, we store the state in steering so CAN messages are idempotent
    - Change high beams -> DRL
    - Fix pin defs moved on hardware
    - Fix typo in steering_digital_input.c

[33mcommit 48716b5659ba2fac901262dc59c9f4d326270710[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Apr 3 10:13:35 2021 -0400

    [SOFT-233] Deterministic soft timers (#331)

[33mcommit 10903c49613977c3adbdf3a971a0ce8a4a6f9d01[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Fri Apr 2 13:21:01 2021 -0400

    [SOFT-233] Unflake CAN and some tests (#328)
    
    * Unflake CAN and some tests
    
    * Update loopback mode to TX over vcan still, afix filtering, and fix a test

[33mcommit 5853cd821200cebccf62643f834a7a0fb804e9a2[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Mon Mar 29 16:50:06 2021 -0400

    [SOFT-233] Always return OK from retrying to avoid leftover acks (#325)
    
    The module `can_tx_retry_wrapper` is inherently broken for (at least) two reasons:
    
    1. Upon receiving the maximum number of failures it resets its internal statistics (i.e. retry count), but leaves the ghost ack live in the queue to time out later. At best this is a gracefully handled unpleasant surprise, but at worst this re-triggers the retry logic and broadcasts unwanted messages.
    2. If anyone were to employ this with an `ack_bitset` of more than one device, an ack from one device would leave `num_remaining` non-zero and cause a retry to all devices, potentially causing difficult side effects
    
    This PR solves both these issues by:
    
    1. Cancelling the ack no matter what the status was after the maximum number of errors are received
    2. Properly warning module users of its limitations with an informative comment
    
    This hopefully reduces some of the flakiness in dependent tests (test_relay_tx.c).

[33mcommit 975ef58acc7a2a88bbf741248cb3f16d3de2561f[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Sun Mar 28 13:27:45 2021 -0700

    [SOFT-426] Bootloader config (#293)

[33mcommit f875eb062e10c68dcf5bd3d0c6a34d8e4a85a425[m
Author: Keith Choa <keithchoa@gmail.com>
Date:   Sun Mar 28 15:50:13 2021 -0400

    [SOFT-280] Solar thermistor reading modification to celsius (#279)
    
    This implements temperature conversions in solar.

[33mcommit 301aaf60a1a460ea79f2f5e42bd68d618a69cd7b[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sat Mar 27 16:16:33 2021 -0400

    [SOFT-414] Update BPS fault sequencing (#306)
    
    See https://uwmidsun.atlassian.net/l/c/gFcHnf3A. BPS fault sequencing changed to, upon BPS fault:
    * discharge precharge on MCI then lock up centre console with a critical section infinite loop
    * turn on strobe in PD and change power setup to whatever's powered on aux

[33mcommit 7e3df8880f73faeea2f84348f67982dc0b00e96b[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Sat Mar 27 11:08:37 2021 -0700

    [SOFT-412] Update solar pins (#308)
    
    Also updates solar to use separate CAN device IDs for the 5 and 6 MPPT versions.

[33mcommit 3a770b36bf8d5ceaace2164985c84c24fd3df363[m
Author: YiJie Zhu <64090163+YiJie-Zhu@users.noreply.github.com>
Date:   Sat Mar 27 11:04:27 2021 -0400

    [SOFT-186] Change block comments to line comments (#322)

[33mcommit 8d4d63e56a8e0bc55e2adc8b0b2e046a34ce05fe[m
Author: Faraz Khoubsirat <58580514+farazkh80@users.noreply.github.com>
Date:   Thu Mar 25 22:02:59 2021 -0400

    [SOFT-449] Add a timeout of 12 minutes on GItHub Actions workflow (#323)

[33mcommit 303130aa92935198b7c92a9a60ae83cc33ca5a3e[m
Author: dependabot[bot] <49699333+dependabot[bot]@users.noreply.github.com>
Date:   Thu Mar 25 20:39:19 2021 -0400

    Bump pyyaml from 5.1 to 5.4 in /projects/can_dump/scripts (#324)
    
    Bumps [pyyaml](https://github.com/yaml/pyyaml) from 5.1 to 5.4.
    - [Release notes](https://github.com/yaml/pyyaml/releases)
    - [Changelog](https://github.com/yaml/pyyaml/blob/master/CHANGES)
    - [Commits](https://github.com/yaml/pyyaml/compare/5.1...5.4)
    
    Signed-off-by: dependabot[bot] <support@github.com>
    
    Co-authored-by: dependabot[bot] <49699333+dependabot[bot]@users.noreply.github.com>

[33mcommit d72468ac828b496f027cdcee2b6ebfb00b521a61[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Tue Mar 23 22:20:12 2021 -0400

    [SOFT-345] unify mpxe pipes (#319)
    
    * Changed logging to use a specific store so we can unify pipes
    
    * Clean up remnants of ctop_fifo

[33mcommit 8fa05c32ebef89328e9558dd04458be549b3fa3f[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Mon Mar 22 23:48:41 2021 -0400

    [SOFT-447] Patch x86 CAN library to allow a message with a zero CAN ID (#318)
    
    On x86, the flow for receiving a CAN message is the following:
      1. The RX thread (prv_rx_thread in can_hw.c) reads a CAN frame from the socket.
      2. The RX thread calls a handler, which turns out to be prv_rx_handler in can.c.
      3. prv_rx_handler calls can_hw_receive until it returns false. On x86, can_hw_receive will only rx one message (the most recently rx'd one), but it does this to support STM32 which can receive multiple in a row.
      4. On the first call, can_hw_receive sees that it has a valid frame received, passes the data upwards, resets it, and returns true; on the second call, it sees that it doesn't have a valid frame and returns false.
    
    Previously, we determined whether the frame was valid by assuming that a CAN arbitration ID of 0 would never happen, so the zero ID caused by memsetting the frame to 0 was checked. Now with the bootloader, an ID of 0 is possible (and likely), so we use a separate flag to determine if the frame is valid.
    
    Note that `read` returns -1 when it fails to read; we don't want to process a frame when it wasn't read correctly, so we check read's return value when setting the is-valid flag.

[33mcommit 358e18dce3054452036a8a6d9e4620956267a8b4[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Thu Mar 18 23:55:57 2021 -0400

    Fix MPXE issues (#315)
    
    This fixes several issues that popped up in MPXE immediately after (actually apparent before) merging.
    Also includes a couple of QOL improvements - removing a circular dependency, changing PASS to DONE since it shows even when integration tests don't pass.
    
    Co-authored-by: Ryan Dancy <ryan@keal.ca>

[33mcommit 9706a45c067305bbc6bcefdf6cf3c7aaedda5c97[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Tue Mar 16 22:54:54 2021 -0400

    MPXE (#198)
    
    * [SOFT-158] Add mpxe folder
    
    * [SOFT-251] Run something in mpxe (#111)
    
    Adds makefile support for the command 'make mpxe'. We split MPXE into
    'pieces', to avoid overalap with 'projects'. Also adds a demo hardcoded
    program to run the project can_communication through mpxe, printing its
    output.
    
    * [SOFT-253] Implement a thread-safe queue
    
    This implements a thread-safe queue. Special thanks @jzarnett for the
    inspiring sample code.
    
    * [SOFT-253] Move project manager proof of concept to different files
    
    To allow for other uses of main.
    
    * [SOFT-253] Test threadsafe queue
    
    This adds a basic producer/consumer test for the queue, and changes
    the makefiles to enable testing mpxe. Changes were made to the implementation
    where issues were found in test.
    
    * WIP [SOFT-254] Grab PID
    
    It leaks proccesses, but otherwise this properly builds and runs
    a project.
    
    * [SOFT-254] Everything is a mess
    
    Adds makefile support for protos, adds protos. Now, from main we
    run the tutorial board interrupt project yet and implement a pseudo
    system call interface for drivers to call into mpxe.
    
    * WIP: refactor
    
    * WIP
    
    * rewrite in python
    
    * WIP start python and datastore attachment
    
    * export to python
    
    * WTF it works
    
    * cleanup
    
    * remove accidental addition
    
    * [SOFT-254] Add comments
    
    * Updated README
    
    * [SOFT-158] implement mpxe as tests (#182)
    
    * [SOFT-158] Rework MPXE
    - moved pm to a class
    - moved project to a class
    - used introspection for message decoding
    - added interface for adding callbacks
    
    * [SOFT-158] Update store interface
    
    - Switched store to have a _config() and _register() call
    rather than the _init() call registering funcs and doing setup
    - updated gpio.c (x86) to reflect these changes
    
    * [SOFT-158] Fix imports and add pedal support
    
    * [SOFT-158] use unit test framework for tests
    
    * [SOFT-158] Start on mci mock and test
    
    * [SOFT-158] Update store interface
    
    - Switched store to have a _config() and _register() call
    rather than the _init() call registering funcs and doing setup
    - updated gpio.c (x86) to reflect these changes
    
    * [SOFT-158] Fix imports and add pedal support
    
    * [SOFT-158] use unit test framework for tests
    
    * [SOFT-158] cleanup working tests
    
    * [SOFT-158] Add comments
    
    * [SOFT-158] Add whitespace for clarity
    
    * [MPXE] Edit MCI test to be more clear
    
    * [MPXE] merge mci into mpxe branch
    
    * [MXPE] fix initialization order in mci
    
    * [MPXE] Use a generic 'sim' class for mocks
    
    * [MPXE] refactor to use generic IntTest class
    
    * [MPXE] shuffled makefile to make test dependencies in top-level make
    
    * Use absolute imports for mpxe
    
    * Don't not generate protobuf files
    
    * Get rid of calling make from python
    
    * Shuffle folder structure
    
    * Add fastmpxe phony target
    
    * Solve all the ResourceWarning
    
    * Add funky integration test prints
    
    * Update readme for mpxe
    
    * Clean Python protobufs properly
    
    * Move store.[ch] to lib mpxe-store and ignore mpxe-gen in lint
    
    * fix linting errors
    
    * fix pylint + reset x86/spi.c
    
    * only link libprotobuf-c if IS_MPXE is set
    
    * Only link libprotobuf-c if IS_MPXE is set, for real
    
    * Make generated file dirs if they don't exist
    
    * Remove outdated INT_TEST documentation
    
    * added locks to logging
    
    * remove excessive log in gpio.c
    
    * [MPXE] clean up log locks and remove i2c and spi logging
    
    * Address review comments and fix gpio indexing
    
    * Merge ADS1015 driver MPXE version with more ifdefs
    
    * Merge MPXE gpio.c to non MPXE gpio.c, surprisingly not too messy
    
    * merge MPXE mcp2515 with non mpxe version
    
    * Add MPXE support for interrupt edges
    
    * fix bug where logging before store is inited blocks
    
    * fix linting errors
    
    * fix other linting error
    
    * Enable Pylint on mpxe
    
    * [SOFT-158] add mpxe adt7476a (#211)
    
    * added protos
    
    * added mpxe support for adt7476a
    
    * remove printf and add newlines to i2c
    
    * finished the integration test
    
    * added protos
    
    added mpxe support for adt7476a
    
    remove printf and add newlines to i2c
    
    finished the integration test
    
    * fixed spacing
    
    * fix an accidentally deleted line in conflict resolution
    
    * fixed changes
    
    * changed ifdef according
    
    * addressed changes
    
    * [SOFT-334] mpxe ads1259 (#216)
    
    * [MPXE] WIP address simple review comments
    
    * [SOFT-322] MPXE adc (#218)
    
    * Soft 333 add mpxe pca9539r (#225)
    
    * [Soft 380] mpxe log locking cleanup (#235)
    
    * [SOFT-332] mpxe mcp23008 support (#241)
    
    * [SOFT-369] add multiple stores (#261)
    
    * [SOFT-424] Fix MPXE pylint errors (#305)
    
    * Run autopep8 with 'make format'
    
    autopep8 was introduced to master since the mpxe branch was forked and made mandatory,
    causing CI to fail. So I ran 'make format' to fix the CI errors.
    
    * Fix one final pylint error
    
    Co-authored-by: Ryan Dancy <ryan@keal.ca>
    Co-authored-by: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
    Co-authored-by: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
    Co-authored-by: Anthony Chang <54450499+anthony-chang@users.noreply.github.com>
    Co-authored-by: Mitchell Ostler <ostlermitchell44@gmail.com>
    Co-authored-by: selinahsu <37355542+selinahsu@users.noreply.github.com>
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 08b35dcb4c0ec8e8c9d9615373c8c32386699651[m
Author: Daniel Ye <danielye4@gmail.com>
Date:   Tue Mar 16 18:51:25 2021 -0400

    [SOFT-233] Remove unused CAN messages (#312)
    
    This removes many unused CAN messages to free up space for new ones.

[33mcommit 1a95479215859a9466af506c7f28d352a6f633c6[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Mar 13 12:12:28 2021 -0500

    [SOFT-439] Merge power distribution changes from mockup branch (#295)

[33mcommit 347dead3880d3d467a4dfc3708db480bc770dff6[m
Author: CheranMahalingam <65926174+CheranMahalingam@users.noreply.github.com>
Date:   Sun Mar 7 20:20:39 2021 -0500

    [SOFT-351] Implement centre console race/normal switch (#307)
    
    This implements the race/normal switch on centre console, which toggles the voltage regulator. If the voltage regulator errors, a warning is logged.

[33mcommit dd131b65e88de66659172e2db41fab43827ab4ad[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sun Mar 7 16:31:02 2021 -0500

    Merge pedal changes from mockup (#296)

[33mcommit 852c741852bbb2f0b9ef60ddafea4e39ef5cf9c1[m
Author: Kevin Lee <57779905+KevinLee5656@users.noreply.github.com>
Date:   Sun Mar 7 11:56:40 2021 -0500

    [SOFT-436] Fix pretty_size.sh error messages (#303)

[33mcommit 8f9e94702604b06615ba8298347a462c22e0897e[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Sat Mar 6 15:35:15 2021 -0500

    [SOFT-437] Integrate voltage regulator into power distribution (#301)
    
    Also change the fan control CAN messages to more generic "PD error" messages.

[33mcommit b2ab62e9be9770b4303de0ebed8fc01a55cec6f7[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Mar 4 15:38:01 2021 -0500

    [SOFT-441] Don't initialize interrupts and soft timers in x86 adc.c (#300)
    
    The x86 adc library was initializing interrupts and soft timers in adc_init, which was causing problems since reinitializing those libraries does nasty stuff like clearing all currently registered interrupts, cancelling soft timers, etc. Fixed by not initializing them.

[33mcommit 575542c1d4f216d74f345211bd9c9fb39abf1f71[m
Author: Sudhish Meenakshisundaram <65432331+sudhish3120@users.noreply.github.com>
Date:   Wed Mar 3 19:51:32 2021 -0500

    [SOFT-392] Babydriver: gpio interrupts python implementation (#281)
    
    This adds the Python implementation for the `register_gpio_interrupt` and `unregister_gpio_interrupt` functions, completing their implementation.

[33mcommit df1eb88e0eddb16de73fcce863a8e39170402932[m
Author: Keith Choa <keithchoa@gmail.com>
Date:   Wed Mar 3 18:16:42 2021 -0500

    [SOFT-429] Added auto-generated README.md (#282)
    
    `make new` now generates a README.md file at the root of the new project or library.

[33mcommit e632fb55df46579f07a93b0a6f8b850f2358e468[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Mar 2 21:10:03 2021 -0500

    [SOFT-440] Fix watchdog cancelling random soft timers (#299)
    
    The watchdog library wasn't setting the timer_id to SOFT_TIMER_INVALID_TIMER before calling watchdog_kick which cancels the timer with id timer_id, so if we're unlucky, the default timer_id would be a valid timer id used by some other soft timer which would get cancelled, causing super hard to debug issues. This appeared when trying to debug babydriver.
    
    Fixed by setting the timer ID in watchdog_start. Also changed all the empty parameter lists in test_watchdog.c to (void) instead of () to comply with conventions.

[33mcommit f8319f74e383e01dcf7ff3b352c0b9008daeba74[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sun Feb 28 13:53:30 2021 -0500

    [SOFT-439] Quality of life changes from mockup branch (#294)
    
    Summary of changes:
    - libraries/ms-common/src/x86/can_hw.c
      * Add an option for x86 CAN to use can0 instead of vcan0, super useful for emulating projects or acks
    - projects/leds/rules.mk, projects/leds/src/main.c
      * Rename controller_board_blinking_leds cus lol
    - projects/smoke_acks/rules.mk, projects/smoke_acks/src/main.c
      * Add a super useful ack mock project to use from laptop
    - projects/smoke_can/src/main.c
      * Fix loopback, since it should definitely default to false

[33mcommit 1674d0d60a1dc3e82cb47e60c3983a9765e63a66[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Feb 24 20:33:25 2021 -0500

    [SOFT-416] Minimal bootloader with linker scripts (#273)
    
    This adds a minimal bootstrapped version of the bootloader. The bootloader itself does nothing except jump to the application code. The bulk of the complexity is in the linker scripts for organizing the bootloader and application code in memory.
    
    There's also a provision for selecting stm32 linker scripts per project from rules.mk (mostly useful for the bootloader project), and a temporary makefile target, temp-bootloader-write, for building the bootloader and application code and flashing both together.

[33mcommit 8b2dcdd485d8272a9419be52580e12f17120ccc2[m
Author: NOURISH CHERISH <nourishnew@gmail.com>
Date:   Tue Feb 23 19:33:13 2021 -0500

    [SOFT-393] Voltage regulator driver (#257)
    
    This adds the `voltage_regulator` driver, which handles the 5V regulator found on centre console and other boards.

[33mcommit aea8466888980ddd547a3794cea3a81a9867a1d4[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Mon Feb 22 20:20:07 2021 -0500

    [SOFT-301] Fix race condition in x86 CAN library (#290)
    
    This fixes the inconsistently failing CAN unit tests and the majority of the instability of CAN on x86.
    
    The culprit seems to be prv_tx_handler in can.c. It’s a callback which runs every time the can_hw implementation registers a CAN_HW_EVENT_TX_READY (which is just a condition where the callback is called, not an event queue event). On stm32, there are 3 CAN mailboxes, so on each TX event can_hw tries to TX, and if there are no mailboxes it swallows the event (but does not pop from the CAN fifo) and exits. The prv_tx_handler callback reraises the TX event on the next TX when this happens (which is when the CAN fifo is nonempty and a TX occurs) so that TXes aren’t lost.
    
    However, on x86, there are no CAN mailboxes and we have a TX thread that’s consuming TXes (from the TX fifo, which isn’t the same as the CAN fifo) and putting them on the vcan bus. Whenever the TX thread completes a TX, it of course calls prv_tx_handler as the TX callback. Meanwhile, whenever a TX event gets processed, can_hw_transmit and prv_handle_tx in can_fsm pop a message from the CAN fifo, feed it to the TX fifo, and signal the TX thread with a semaphore. Sometimes, the TX thread takes its sweet time waking up when the semaphore is raised, and sometimes it wakes up right away.
    
    This creates a race condition. If all the TX events can be processed and the CAN fifo emptied before the TX thread wakes up for the first time, then the CAN fifo is empty when prv_tx_handler is called first, and no extra TX event is raised. However, if the TX thread wakes up (and completes its ~250us sleep after the first write) before all the TX events are processed, then the CAN fifo is nonempty for prv_tx_handler and an extraneous TX event is raised, which a) messes up tests using MS_TEST_HELPER_CAN_TX_RX in a big way and b) even if it doesn’t, the TX that the extra event is supposed to save will already have been TX’d by the time the extra event gets processed, and tons of weird things could probably happen. This is what causes all the test instability, like when I added a LOG_DEBUG to a MS_TEST_HELPER_CAN_TX loop and suddenly everything started failing.
    
    Fixed by adding an X86 macro defined on x86 only and using it to make prv_tx_handler only do its thing on stm32.
    
    * Fix PD's publish_data for CAN changes
    
    publish_data was sending 22 CAN messages at once, which exhausted the
    event queue (size 20) but not the CAN tx fifo (size 32), so previously
    the bug in can.c was saving it. Change it to send them incrementally
    like solar/data_tx, now it works.
    
    * Make test_data_tx faster and less flaky
    
    Due to the CAN bug, data_tx's tests previously had to tx only one
    message at a time, with 350ms between each tx. This was *super* slow,
    and because there were ~50 messages to tx, super flaky. This changes the
    test to tx 8 at a time, 150ms apart, which is way faster (<1s instead of
    10+ seconds) and less flaky.
    
    Co-authored-by: mitchellostler <ostlermitchell44@gmail.com>

[33mcommit 5ced79d42999608c1b0bae608852d804aca0213e[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Sat Feb 20 10:39:25 2021 -0500

    [SOFT-432] Migrate from Travis CI to GitHub Actions (#283)
    
    This replaces our Travis script with a GitHub Actions workflow, because GitHub Actions is free for open source and Travis CI no longer is.

[33mcommit 2e56bc7e32aae4790fea73bc6ebe936ce0031af3[m
Author: Kevin Lee <57779905+KevinLee5656@users.noreply.github.com>
Date:   Tue Feb 16 23:10:57 2021 -0500

    [SOFT-433] Add *.code-workspace to .gitignore (#284)

[33mcommit eb8288d15c95a5cad8dd014b3480a6d83e6d15f2[m
Author: Henry Zhou <43188301+hzhou0@users.noreply.github.com>
Date:   Tue Feb 16 23:00:56 2021 -0500

    [SOFT-338] Add autopep8 format to build process (#276)

[33mcommit 934e7a3c0a56ed93af91678d0b54ab0986fa1669[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Sun Feb 14 10:54:19 2021 -0500

    [SOFT-420] Update Unity to v2.5.2 (#277)

[33mcommit b3295c3b07e686f88517a892879b131fd95a4fc3[m
Author: Kyle Chan <64808590+Kyle02@users.noreply.github.com>
Date:   Wed Feb 10 06:14:41 2021 -0800

    [SOFT-377] Babydriver spi exchange firmware (#267)
    
    This adds the firmware implementation of the babydriver spi_exchange function.

[33mcommit 640c3719e46e5ba2f108fbaac53a798b820569ee[m
Author: selinahsu <37355542+selinahsu@users.noreply.github.com>
Date:   Tue Feb 9 08:53:37 2021 -0500

    [SOFT-385] Babydriver CAN sending (#245)
    
    This adds `can_send`, `load_dbc`, and `can_send_raw` commands to Babydriver.
    
    Co-authored-by: CheranMahalingam <cheranm@outlook.com>

[33mcommit d5456ac146f93223f1aafd90c8f0f0abfd9a90cc[m
Author: Het Shah <59981187+shahhet14@users.noreply.github.com>
Date:   Mon Feb 8 21:03:05 2021 -0500

    [SOFT-327] Integrate python tests (#272)
    
    This adds `make pytest`, `make pytest_all`, and `make install_requirements`.

[33mcommit ec4efd8921d34b356983fbc19a9a8f40e3cf0472[m
Author: Keith Choa <keithchoa@gmail.com>
Date:   Sun Feb 7 22:56:07 2021 -0500

    [SOFT-422] Fix status_ok_or_return to only evaluate argument once (#275)

[33mcommit 1cbb110a7d359836acd78a6e98e4718fafa0273c[m
Author: Daniel Zeng <78101830+PurpleRhythms@users.noreply.github.com>
Date:   Sat Feb 6 13:56:59 2021 -0500

    [SOFT-423] Update .gitignore with Eclipse .project (#274)
    
    Co-authored-by: FourRhythm <FourRhythm@users.noreply.github.com>

[33mcommit 7ffaeec6ccc874ae56b00f58b82574837df875b2[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Feb 2 23:15:46 2021 -0500

    [SOFT-419] Enable parameterized tests (#271)
    
    Our testing framework Unity gives us a way to use parameterized tests! See test_parameterized.c for an example.
    Also includes a monkey patch to the test runner generator script to remove `(RUN_TEST_NO_ARGS)` test log spam.

[33mcommit af0ac21d91964d8ca8d7dcd31ccab6c523dc30db[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Fri Jan 22 19:18:52 2021 -0500

    [SOFT-384] Implement x86 wait (#246)
    
    This implements wait() on x86.

[33mcommit 9a24cb28b0659a78bc370f7d58d6e25c68c05d05[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Thu Jan 21 19:57:44 2021 -0500

    [SOFT-395] UV lockout detection (#259)
    
    This sends a CAN message when a UV lockout condition is detected on front power distribution.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 0bdfdd82f7515dbda9c5b7d88e093a7f50cf0f1f[m
Author: CheranMahalingam <65926174+CheranMahalingam@users.noreply.github.com>
Date:   Mon Jan 11 20:56:21 2021 -0500

    [SOFT-394] PD Rev 3.1 front/rear recognition (#254)
    
    This updates the front/rear recognition on power distribution to rev 3.1's system.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit bdf4921de58fe71973162a090b0288a76dec2d61[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Mon Jan 11 07:46:50 2021 -0500

    [SOFT-405] Use a buffer for reading from next_message (#256)

[33mcommit 8082be191cc713c57e280ee0c5b5570b71f85757[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Tue Jan 5 14:27:30 2021 -0500

    [SOFT-390] PCF8523 RTC driver (#250)
    
    This adds a driver for the PCF8523 Real-Time Clock used on centre console.

[33mcommit ea5fd9d49f1282a01a1212c0f7e30c38c2eb7faf[m
Author: CheranMahalingam <65926174+CheranMahalingam@users.noreply.github.com>
Date:   Sat Jan 2 11:59:07 2021 -0500

    [SOFT-391] Babydriver: firmware implementation of gpio_interrupts (#251)
    
    This adds the firmware portion of the gpio interrupts module for Babydriver.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 95a0847ba271d235a3e75c45df79b7995c7a19d5[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Dec 31 11:00:12 2020 -0500

    [SOFT-386] Update centre console button pins to latest rev (#252)

[33mcommit f67f31ee10fcd99c11a0997c45baa0d751990028[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Wed Dec 30 16:52:00 2020 -0500

    [SOFT-379] spi_exchange python (#249)
    
    This adds the Python side of the spi_exchange Babydriver function.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit f90219420ed826a3810d3c4d7c2cafbf1d8afac3[m
Author: NOURISH CHERISH <nourishnew@gmail.com>
Date:   Sun Dec 27 16:00:06 2020 -0500

    [SOFT-388] Smoke test for MAX6643 fan controller (#247)
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit f6a53097fd6db1ea83f74052b1cb2e5c4802b9b5[m
Author: Sudhish Meenakshisundaram <65432331+sudhish3120@users.noreply.github.com>
Date:   Mon Dec 21 16:16:18 2020 -0500

    [SOFT-344] BTS7040 smoke test (#244)
    
    This adds the smoke test for the BTS7040 for power distribution.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 13cf7208830adc992c9f54dd9faaa35c45d79a2a[m
Author: Raiyan Sayeed <raiyan.business@gmail.com>
Date:   Mon Dec 21 15:34:08 2020 -0500

    Update README to include pylint line (#248)
    
    Hopefully saves time for anyone else who had a hard time looking for it.

[33mcommit d85dc304e983132df2711d2043c631b167840c1d[m
Author: CheranMahalingam <65926174+CheranMahalingam@users.noreply.github.com>
Date:   Sat Dec 12 22:45:22 2020 -0500

    [SOFT-372] i2c_write firmware implementation (#240)
    
    This adds the firmware implementation of the i2c_write function to Babydriver. The i2c_write function is now fully functional.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 010330d8a544a56c886d1b4f4ed1b857d8f0666d[m
Author: Daniel Ye <danielye4@gmail.com>
Date:   Wed Dec 9 20:18:57 2020 -0500

    [SOFT-371] Babydriver i2c read python (#242)
    
    This implements the i2c_read Babydriver function in Python.

[33mcommit ad2f406336a0c7c8b545687cf324c7aba61af9f0[m
Author: abdullahs26 <74064210+abdullahs26@users.noreply.github.com>
Date:   Mon Dec 7 23:56:37 2020 -0500

    [SOFT-376] Babydriver: I2C write Python (#243)
    
    This implements the Babydriver i2c_write function on the Python side.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 9a6a06b7177568d19aa6eb653711796dae42d3d3[m
Author: Patrick Kim <72056241+s524kim@users.noreply.github.com>
Date:   Fri Dec 4 20:20:27 2020 -0800

    [SOFT-363] STM32 memory usage data formatting (#239)
    
    A script which pretty-prints the memory usage data upon building for STM32.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit deca2903c7d8802aa10aa4e3d66a5dbb14fe60ee[m
Author: NOURISH CHERISH <nourishnew@gmail.com>
Date:   Fri Dec 4 22:36:34 2020 -0500

    [SOFT-382] Smoketest for DRV120 (#238)
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 3ebde0e241a9c3258635fb800066411b0c4bda87[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Dec 2 19:41:16 2020 -0500

    [SOFT-306] Fix the adc_read test (#236)
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>
