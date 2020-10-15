[33mcommit 0cbc1632e9e8047370436196a0fe2a94a6b39176[m[33m ([m[1;36mHEAD -> [m[1;32msoft_999_kyle_chan_fw_103[m[33m, [m[1;31morigin/master[m[33m, [m[1;31morigin/HEAD[m[33m, [m[1;32mwip_getting_started[m[33m, [m[1;32mmaster[m[33m)[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Wed Sep 16 20:24:47 2020 -0400

    [SOFT-196] ADT7476A (#160)
    
    * changed i2c_write_reg to i2c_write, added config for strt bit
    
    * pass two bytes using i2c_write
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit bb2c29f8ba3908a616d4104a7185dbcad092fa68[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Sep 16 18:58:34 2020 -0400

    [SOFT-309] Fix PCA9539R driver and smoke test (#161)
    
    * Add missing gpio_init() in PCA9539R smoke test
    
    * Change write register format
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 64a5034276b4d64a0927fcb589b5f9e92d4767b7[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Sep 16 18:49:33 2020 -0400

    [SOFT-311] Don't turn on horn in startup sequence (#162)
    
    * [SOFT-311] Don't turn on the horn at startup - why does this have to be said
    
    * [SOFT-311] Replace with main display, which was probably what was intended

[33mcommit 6f4f0652088e0853e254a34321489f9c72c4d33c[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Sep 12 23:25:59 2020 -0400

    [SOFT-172] Implement current sense (#147)
    
    * [SOFT-172] Implement current sense
    
    Current sense stores the last 20 readings, and faults
    bps in case of overcurrent.
    
    * [SOFT-172] Add calibration values from hardware
    
    * [SOFT-172] Test current sense
    
    * [SOFT-172] Address comments
    
    * [SOFT-172] Alter fault_bps api
    
    Also edit ads1259 driver slightly to properly clear
    the fault_bps bitset when there isn't a fault.
    
    * [SOFT-172]: Implement secondary suggestions

[33mcommit 8f3387c477c05a7cfc796edd7a0761680f9235f3[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Sep 10 20:19:37 2020 -0400

    [SOFT-303] Integrate data_tx into solar main module (#156)
    
    This makes solar code complete. 🎉

[33mcommit e0c916d4093044d871921a7e1a97612a7b56b0bd[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Sep 9 22:21:27 2020 -0400

    [SOFT-302] Update README to clone correct repository (#155)
    
    * Update README to clone correct repository
    
    * [SOFT-302] cd into the right directory too
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 3d72c72dd7fe0ec886cb859ca557b928bf943a42[m
Author: Het Shah <59981187+shahhet14@users.noreply.github.com>
Date:   Wed Sep 9 20:42:59 2020 -0400

    [SOFT 214] data_tx solar module (#139)
    
    * [SOFT-290] Add solar fault can msg (#145)
    
    * [SOFT-290] Add solar fault CAN message
    
    * Remove unused SOLAR_DATA_FRONT CAN message
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>
    
    * created unit tests
    
    * fixed include file name
    
    * fixed minor syntax errors
    
    * made process event function return bool
    
    * fixed can message count variable name
    
    * redesigned data_tx module using soft timers
    
    * reduced time between transmiting messages
    
    * made data_tx more robust and updated unit tests to tx all data points
    
    * modified unit tests to prevent them from failing if NUM_DATA_POINTS is not a multiple of msgs_per_tx_iteration
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 319d66c0b07bede059d593d5c255ee88924520e1[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Tue Sep 8 14:36:32 2020 -0400

    [SOFT-61] Implement fault bps (#149)
    
    * [SOFT-61] Implement fault bps
    
    Change relay sequence a bit to allow for external
    faulting of relays, and separate fault header from
    bms.h.
    
    * [SOFT-61] Add tests for fault_bps
    
    * [SOFT-61] Correct anti recursion logic
    
    * [SOFT-61] Modify tests to accomodate changes

[33mcommit c18edfb94cf68d1c079d4ce583e5c6a69267a3bc[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Tue Sep 8 13:14:51 2020 -0400

    [SOFT-174] fan control module (#148)
    
    * reset format
    
    * added args to speed function
    
    * fixed comment
    
    * fixed naming
    
    * init i2c
    
    * added get data, interrupt
    
    * added i2c read and write address to storage
    
    * draft pr comments, changed fan inputs
    
    * speed function now accepts number between 0-100
    
    * removed pointer from register interrupt
    
    * reverted last commit
    
    * fixed build errors
    
    * removed unneeded function
    
    * fixed file naming
    
    * fixed additional formatting things
    
    * Formatting (Automated commit)
    
    * added unit tests, out of range argument for speed
    
    * added mock to makefile
    
    * Formatting (Automated commit)
    
    * fixed newline error
    
    * organized makefile, fixed mock
    
    * Formatting (Automated commit)
    
    * fixed build issue
    
    * Formatting (Automated commit)
    
    * passes tests now
    
    * Formatting (Automated commit)
    
    * started smoke test
    
    * address pr comments
    
    * added pins to smoke test
    
    * Formatting (Automated commit)
    
    * fixed random number
    
    * fixed rand
    
    * changed naming of pwm ports
    
    * Formatting (Automated commit)
    
    * naming
    
    * changed rand to cycle through times instead
    
    * Formatting (Automated commit)
    
    * Formatting (Automated commit)
    
    * added log message
    
    * log header
    
    * Formatting (Automated commit)
    
    * fixed fan naming build error
    
    * fixed naming build error
    
    * added callback, support for single controller
    
    * styling
    
    * build error
    
    * initial commit
    
    * assuming temps are in degrees Celsius
    
    * return error
    
    * addressed pr comments, added test

[33mcommit d55d1a62cfcbfeb696ccc161c6a45a6af7b5f5ba[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Fri Aug 28 23:55:53 2020 -0400

    [SOFT-290] Solar main module, fault_handler, command_rx (#146)
    
    This adds the main, fault_handler, and command_rx modules for solar.
    
    * [SOFT-290] Add solar fault CAN message
    
    * Remove unused SOLAR_DATA_FRONT CAN message
    
    * [SOFT-216] Implement solar_fsm
    
    * [SOFT-216] Add tests for solar_fsm
    
    * [SOFT-216] Add solar_fsm config
    
    * [SOFT-216] Handle CAN inside solar_fsm
    
    * [SOFT-216] Lengthen CAN delay to try to get Travis to pass
    
    * [SOFT-290] Change fault events to exported enum
    
    * [SOFT-290] Implement and add tests for fault_tx
    
    * [SOFT-290] Add main.c
    
    * [SOFT-290] Lengthen solar_fsm CAN delay even more for Travis
    
    * [SOFT-290] Major refactor: don't use fault events, split up solar_fsm
    
    * [SOFT-290] Change extern constants to functions returning const pointers
    
    * [SOFT-290] Change relay_fsm's FSM name
    
    * [SOFT-290] Address review comments
    
    * [SOFT-290] Return const pointers in solar_config

[33mcommit 0c2759fb2be239a6d38376bfb9c72128c04c47f7[m
Author: Hewitt McGaughey <58054834+hewittmcg@users.noreply.github.com>
Date:   Wed Aug 19 18:42:59 2020 -0400

    [SOFT-272] passive balancing (#142)
    
    * init commit, wrote base code. might need to update once SOFT-9 merged in + add unit tests
    
    * fixed issue with soft timer, added ms-drivers to bms_carrier include path
    
    * debugging issue with prv_wakeup_idle
    
    * test cases pass now, need to implement passive_balance.c soft timer cb, clean up afe settings in test sequence
    
    * changed soft timer cb implementation
    
    * minor formatting fixes
    
    * minor formatting changes
    
    * made changes to implementation + test sequence to be called in cell_sense
    
    * formatted, removed extraneous code from test sequence
    
    * made requested changes + some formatting/clarity changes
    
    * quick requested changes

[33mcommit 4a4c6d3cd3ced7b7c17b4e4aed99389ac850fed1[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Aug 18 21:20:41 2020 -0400

    [SOFT-290] Add solar fault can msg (#145)
    
    * [SOFT-290] Add solar fault CAN message
    
    * Remove unused SOLAR_DATA_FRONT CAN message
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 9873a93b6e24523f85ebac64eebb84106a3d8bfc[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Aug 18 20:51:04 2020 -0400

    [SOFT-216] solar_fsm solar module (#143)
    
    * [SOFT-216] Implement solar_fsm
    
    * [SOFT-216] Add tests for solar_fsm
    
    * [SOFT-216] Add solar_fsm config
    
    * [SOFT-216] Handle CAN inside solar_fsm
    
    * [SOFT-216] Lengthen CAN delay to try to get Travis to pass

[33mcommit f6a0864c1dc30948ef5ad55fcd338b4f1ea617d3[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Aug 15 13:59:28 2020 -0400

    [SOFT-175] debug current sense (#144)
    
    * WIP debug current sense
    
    * WIP
    
    * WIP: tons of logging
    
    * WIP: added spi baudrate
    
    * added more logging
    
    * removed broken delay
    
    * WIP removed a bunch of logging, fixed rreg
    
    * log formatting
    
    * WIP: change vref to double
    
    * fix vref
    
    * fix rreg command
    
    * Fix internal reference on reset, fix writing to registers, and change ADS1259_VREF_EXTERNAL to 0x08
    
    * [SOFT-175] Clean up debugging
    
    Change some printf statements to LOG_DEBUG, and remove some
    unnecessary checks put in place for debugging purposes.
    
    * [SOFT-175] Implement suggestions
    
    * [SOFT-175] Commented out of date test cases
    
    * [SOFT-175] Fix formatting
    
    Co-authored-by: Liam Hawkins <liam.hawkins@uwmidsun.com>
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit b4191313260f9868f73af013391c75e04e358566[m
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Wed Aug 12 18:28:55 2020 -0400

    [SOFT 281] added loopback mode smoketest (#141)
    
    * added loopback mode smoketest
    
    * compacted variables and added message to indicate loopback success

[33mcommit 8086dc9c52a4ed6069ee3724c48cf6259ea973df[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Aug 4 09:47:11 2020 -0400

    [SOFT-215] fault_monitor solar module (#140)
    
    * [SOFT-215] Implement fault_monitor
    
    * [SOFT-215] Add tests for fault_monitor
    
    * [SOFT-215] Use long constants for STM32
    
    * [SOFT-215] Use uint64_t instead of overflow trickery
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 782eb5a3802153eb403e9d08174d17531f75912c[m
Author: Tuan (Tristan) Le <38996499+tristanle22@users.noreply.github.com>
Date:   Sat Aug 1 16:27:32 2020 -0400

    [soft-168] - MCP3427 smoke test (#137)
    
    Log the reading from MCP3427 whenever reading is available.
    
    Can run on one or multiple MCP3427.
    Can only run on either voltage or current sense MCP3427 at a time.
    
    * [soft-168] - Add smoke test
    
    * [soft-168] - Addressed PR comments
    
    * [soft-168] - Addressing PR comments

[33mcommit c802ace079c9dfc7f596d101139d1943c1e0c7dc[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Sat Aug 1 12:02:31 2020 -0400

    [SOFT-212] Solar logger module (#138)
    
    * [SOFT-212] Implement logger
    
    * [SOFT-212] Add logger tests and fixes
    
    * [SOFT-212] Make test logging cross platform
    
    The STM32F0xx platform has the default size of |int| and |unsigned int|
    as 2 bytes, while the x86 platform has them at 4 bytes. This causes
    problems when matching formatting to a type with a specific size when
    we're building for two different platforms.
    
    * [SOFT-212] Change int to long int in logging
    
    Co-authored-by: g-jessmuir <e.jessmuir@gmail.com>

[33mcommit 70a417c8beb44dc25a0765400254148b9e3d821e[m
Author: jacobpiirsalu <46936408+jacobpiirsalu@users.noreply.github.com>
Date:   Fri Jul 31 21:29:54 2020 -0400

    [SOFT-229] Power selection smoke test (#127)
    
    * implmented smoke test
    
    * updated smoke_test.h to meet lint format
    
    * updated power_selection smoke test to be contained in power_selection.c
    
    * linted power_selection
    
    * removed unnecessary comments and removed includes from power_selection.c
    
    * removed unnecessary comments
    
    * linted code

[33mcommit 298bdac84e1dbf84d5fa57d2641049e35f548c04[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Jul 25 14:07:24 2020 -0400

    [SOFT-171] Implement bms relay control (#124)
    
    * [SOFT-171] Implement BMS relay controls
    
    This implements BMS relay controls as sequences rather than an FSM, since things need to happen
    asynchronously, an FSM would over-complicate things.
    To close relays, the steps are:
    1. close ground
    2. assert it closed properly, if not fault
    3. close hv
    4. assert it closer properly, if not fault
    5. broadcast done.
    A similar procedure takes place for opening, but we open them at the same time.
    We need to stagger the closing of the relays to escape simultaneous inrush current, which
    could trigger overcurrent protection.
    
    * [SOFT-171] Implement suggestions for bms_carrier relay sequence
    
    * [SOFT-171] Remove internal state assertions
    
    * [SOFT-177] Rename BMS AFE faults
    
    Remove the |CURRENT_SENSE_| portion of bms fault EEs because
    they are no longer accurate.

[33mcommit 1260ab695e980fcba895e0c443189286222b6748[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Fri Jul 24 23:29:48 2020 -0400

    [SOFT-276] smoketest can (#135)
    
    * changed address to PA0 instead of PA1
    
    * created smoke test can
    
    * finished smoke test
    
    * addressed changes

[33mcommit eacef565573d4a4d754740b72a74c6d4c261ac16[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Fri Jul 24 20:27:19 2020 -0400

    [SOFT-277] data_store restructure and scaling factors (#136)
    
    * [SOFT-277] Restructure data_store to avoid depending on number of MPPTs
    
    * [SOFT-277] Convert data_store to 32-bit integers
    
    * [SOFT-277] Add scaling factor to sense_mcp3427
    
    * [SOFT-277] Add scaling factor to sense_mppt
    
    * [SOFT-277] Convert currents in data_store to uA
    
    * [SOFT-277] Implement suggestions

[33mcommit 65b0cfc05d123a051e068012f8bdf5894e35192e[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Fri Jul 24 16:34:39 2020 -0400

    [SOFT-221] Smoke adt7476a (#134)
    
    * reset format
    
    * added args to speed function
    
    * fixed comment
    
    * fixed naming
    
    * init i2c
    
    * added get data, interrupt
    
    * added i2c read and write address to storage
    
    * draft pr comments, changed fan inputs
    
    * speed function now accepts number between 0-100
    
    * removed pointer from register interrupt
    
    * reverted last commit
    
    * fixed build errors
    
    * removed unneeded function
    
    * fixed file naming
    
    * fixed additional formatting things
    
    * Formatting (Automated commit)
    
    * added unit tests, out of range argument for speed
    
    * added mock to makefile
    
    * Formatting (Automated commit)
    
    * fixed newline error
    
    * organized makefile, fixed mock
    
    * Formatting (Automated commit)
    
    * fixed build issue
    
    * Formatting (Automated commit)
    
    * passes tests now
    
    * Formatting (Automated commit)
    
    * started smoke test
    
    * address pr comments
    
    * added pins to smoke test
    
    * Formatting (Automated commit)
    
    * fixed random number
    
    * fixed rand
    
    * changed naming of pwm ports
    
    * Formatting (Automated commit)
    
    * naming
    
    * changed rand to cycle through times instead
    
    * Formatting (Automated commit)
    
    * Formatting (Automated commit)
    
    * added log message
    
    * log header
    
    * Formatting (Automated commit)
    
    * fixed fan naming build error
    
    * fixed naming build error
    
    * added callback, support for single controller
    
    * styling
    
    * build error
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 9301635850be594f4d0110ef7e1299c5b37c575d[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Wed Jul 22 19:59:48 2020 -0400

    [SOFT-196] adt7476a (#128)
    
    * reset format
    
    * added args to speed function
    
    * fixed comment
    
    * fixed naming
    
    * init i2c
    
    * added get data, interrupt
    
    * added i2c read and write address to storage
    
    * draft pr comments, changed fan inputs
    
    * speed function now accepts number between 0-100
    
    * removed pointer from register interrupt
    
    * reverted last commit
    
    * fixed build errors
    
    * removed unneeded function
    
    * fixed file naming
    
    * fixed additional formatting things
    
    * Formatting (Automated commit)
    
    * added unit tests, out of range argument for speed
    
    * added mock to makefile
    
    * Formatting (Automated commit)
    
    * fixed newline error
    
    * organized makefile, fixed mock
    
    * Formatting (Automated commit)
    
    * fixed build issue
    
    * Formatting (Automated commit)
    
    * passes tests now
    
    * Formatting (Automated commit)
    
    * address pr comments
    
    * changed naming of pwm ports
    
    * naming
    
    * fixed naming build error
    
    * added interrupt to tests
    
    * Formatting (Automated commit)
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 64550769e1458ac3fd663c75ccd78944d32cd524[m
Author: Gerald Aryeetey <gndaryee@uwaterloo.ca>
Date:   Tue Jul 21 10:34:32 2020 -0400

    [SOFT-269] Align precharge control with hardware (#129)
    
    * Align precharge control with hardware
    
    * Change monitor case order to match sequence
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 939e5e6eedfb4151ec1657d496ec32e0dea00e8c[m
Author: Gerald Aryeetey <gndaryee@uwaterloo.ca>
Date:   Tue Jul 21 10:13:49 2020 -0400

    [SOFT-9] BMS Carrier: Cell Sense Implementation (#131)
    
    * Initial Implementation of cell_sense
    
    * Apply autoformat fixes
    
    * Changes such that cell_sense actually builds
    
    * Also fix wrong bps input
    
    TODO: Should rename EE to not be current sense?
    TODO: Get tests functional
    
    * Implement tests
    
    * Currently mocks fault_bps but isn't ideal
    * Fix potential overflow bug
    * Fix typo in cell_sense
    
    * Formatting fixes
    
    * More generic checks so that this can be run on H/W :)
    
    * Fix linting issues
    
    * Separate num_thermistors from num_cells
    
    * Didn't change smoketest for now
    
    * Apply code review changes
    
    * revert num_thermistors
    * events have a *_END (remove NUM_*_EVENTS)
       * you can do for loops easily
       * can add a BEGIN for cleanness but use this for cosistency
    
    * Fix linting issues
    
    * Remove irrelevant TODO
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit e89baa4bd46bee87e0d6756eba75fd7d804e3baa[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sat Jul 18 12:58:50 2020 -0400

    [SOFT-179] smoketest spv1020 (#133)
    
    Smoke test for the SPV1020 MPPT.
    
    * changed address to PA0 instead of PA1
    
    * started smoke test
    
    * finished smoke test
    
    * fixed changes
    
    * added documentations
    
    * added \n to messages and removed unnneeded depedencies
    
    * needed interrupt to run on platform x86
    
    * make lint and format
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 8dd1ed3f88206b9ea5bf05faa1767e32b132aa76[m
Author: Tuan (Tristan) Le <38996499+tristanle22@users.noreply.github.com>
Date:   Wed Jul 15 19:57:35 2020 -0400

    [SOFT-224] Smoke bts7200 (#121)
    
    * [soft-224] - Added smoke test bts7200
    
    * [soft-224] - Yoink power distro into this smoke test
    
    * [soft-224] - Added instructions
    
    * [soft-224] - Fixed instructions
    
    * [soft-224] - Bad bad formatting
    
    * [soft-224] - Clean up unused code and addressed PR comments
    
    * [soft-224] - Removed unnecessary code
    
    * [soft-224] - Fixed weird clang-format
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 371ee07af0bec54a621e28177f6cbf049568cf03[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Jul 15 19:06:06 2020 -0400

    [SOFT-210] sense_temperature (#130)
    
    * [SOFT-211] Add sense_temperature header and config
    
    * [SOFT-211] Implement sense_temperature
    
    * [SOFT-210] Implement rest of sense_temperature tests

[33mcommit 074069b59b73907a3e78543fafd50a4a268366f0[m
Author: Het Shah <59981187+shahhet14@users.noreply.github.com>
Date:   Mon Jul 13 18:23:24 2020 -0400

    [SOFT-239] Mcp23008 smoke test (#126)
    
    * smoke test will initialize pins, check for correct initialization and toggling of states
    
    * include ms-drivers in make file, more descriptive function names and other minor fixes
    
    * updated function name changes
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 735af1eb51bd643d50fb739074c6db57c1050cbd[m
Author: Gerald Aryeetey <gndaryee@uwaterloo.ca>
Date:   Sun Jul 12 11:27:44 2020 -0400

    [SOFT-223] Quick Lint/Format Bugfix (#132)
    
    * Fix bug
    
    Quick lint/format wouldn't find any files (no arguments, but issue should probably happen with args)
    
    * Increase changed scope
    
    Wouldn't look at changed files that were already committed (or pushed).

[33mcommit 83f4d4dd520dfb18f0d4b096cef4be8e8b99e530[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Jul 9 23:24:15 2020 -0400

    [SOFT-209] sense_mcp3427 solar module (#118)
    
    This is a general sense module for all sensing from MCP3427s. It therefore supersedes sense_current, which I deleted. Also added a config for sense_mcp3427 in solar_config. Since it depends on the number of MPPTs there are (5 or 6), there's now a function in solar_config for getting the correct config for the board type.
    
    * [SOFT-209] Implement sense_mcp3427
    
    * [SOFT-209] Add config for sense_mcp3427
    
    * [SOFT-209] Add tests to sense_config
    
    * [SOFT-209] Add sense_mcp3427 tests
    
    * [SOFT-209] Remove sense_current, superseded by sense_mcp3427
    
    * [SOFT-209] Remove data_store_set mock from test_sense_mcp3427
    
    * [SOFT-209] Reorder config, add fault test, generate addresses
    
    * [SOFT-209] Extract callbacks from mcp3427_start mock
    
    * Formatting (Automated commit)
    
    * [SOFT-209] Add fault reset test case
    
    * [SOFT-209] Remove duplicated define
    
    * [SOFT-209] Remove accidentally committed sense_mppt.h
    
    * [SOFT-209] Resolve incompatibilities in sense_mppt
    
    * [SOFT-209] Pass a data struct instead of array indices
    
    * [SOFT-209] Apply include section rules to solar_config

[33mcommit f296ed3d97af6c839a9180e214b3077be3ff80b3[m
Author: Hewitt McGaughey <58054834+hewittmcg@users.noreply.github.com>
Date:   Thu Jul 9 20:18:58 2020 -0400

    [SOFT-189] DRV120 relay driver (#123)
    
    * added files for driver, test sequence, and formatted
    
    * changed pin used in test to correct value, clarified variable name
    
    * made requested changes, added some comments to increase clarity
    
    * made requested changes: formatting/clarity
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 4d0fe9ddb6a352878d5998fb93e5205788957ff9[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Wed Jul 8 22:21:42 2020 -0400

    [SOFT-211] sense_mppt (#122)
    
    Every sense cycle, sense_mppt reads the current, voltage, PWM, and status from each MPPT. It stores the current/voltage/PWM and the CR bit of the status in the data store, then checks the status byte for faults and raises a fault event if it finds any.
    
    * [SOFT-211] Implement sense_mppt
    
    * [SOFT-211] Restructure sense_mppt
    
    * [SOFT-211] Add sense_mppt tests
    
    * [SOFT-211] Fix formatting
    
    * [SOFT-211] Change mppt ownership, add Mppt typedef

[33mcommit 848a450986327b1551bb95b0ab483832b8d108b8[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Mon Jul 6 19:30:19 2020 -0700

    [SOFT-175] Smoke test ads1259 (#125)
    
    * Create smoke test for ads1259_adc
    
    * Fix formatting
    
    * Fix magic number
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit b6b0c915be5df5e28b8ba42ae77d4c08d0366dec[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Mon Jul 6 20:15:07 2020 -0400

    [SOFT-260] Charger (#105)
    
    * [SOFT-259] Implement charger hardware interfacing
    
    Implements the charger modules battery_monitor, charger_controller,
    connection_sense, and control_pilot. Also includes the charger_events header
    and tests for the four modules implemented.
    
    * [SOFT-260] Implement charger business logic
    
    Implementations documented in headers. Implements the high level logic
    for controlling the charger. Tests must still be written for sequences.
    
    * [SOFT-260] Test charger business logic
    
    This commit includes tests for the begin charging sequence and the stop
    charging sequence. Also includes some minor refactoring.
    
    * [SOFT-260] Implement review suggestions
    
    * [SOFT-260] Implement suggestions.
    
    This commit adds `#include "unity.h"` to test files, replaces a couple
    `TEST_ASSERT` with `TEST_ASSERT_EQUAL`, fixes the main loop function, and
    adds a comment to control_pilot.h to make its use more clear.

[33mcommit 1091999eef40dc0c97306214893bc92b7289f455[m
Author: Max Zhu <44929655+Max-MZ@users.noreply.github.com>
Date:   Mon Jul 6 19:19:52 2020 -0400

    [SOFT-196] Implement driver for adt7476a (#116)
    
    * reset format
    
    * added args to speed function
    
    * fixed comment
    
    * fixed naming
    
    * init i2c
    
    * added get data, interrupt
    
    * added i2c read and write address to storage
    
    * draft pr comments, changed fan inputs
    
    * speed function now accepts number between 0-100
    
    * removed pointer from register interrupt
    
    * reverted last commit
    
    * fixed build errors
    
    * removed unneeded function
    
    * fixed file naming
    
    * fixed additional formatting things
    
    * Formatting (Automated commit)
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 4dd1ae0befb7f900d13a482498e45a5bb08f90f0[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Mon Jul 6 16:25:35 2020 -0400

    [SOFT-197] Update killswitch to match new architecture (#117)
    
    * [SOFT-197] Update killswitch to match new architecture
    
    This commit updates BPS and killswitch to match the new architecture
    as well as implements tests.
    
    * [SOFT-197] Fix BMS module issues

[33mcommit ccd5225236de6fa6186c46203e155a6445eb37ac[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Jul 2 16:46:56 2020 -0400

    [SOFT-270] Fix power distribution main event loop (#120)
    
    Oops: the main event loop wouldn't run.

[33mcommit c192a1101b1f3a3455066be9de89d917dc45987b[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Wed Jul 1 15:53:01 2020 -0700

    [SOFT-173] Implement ads1259 driver (#92)
    
    * publishing branch to github
    
    * Add register and command defs
    
    * Add macros to compile register configurations
    
    * Add most recent file changes for draft PR
    
    * Finalize driver structure
    
    * Add storage member 'reading' and data conversions
    
    * Fix build issues and create test file
    
    * Fix format issues
    
    * Make second fix to format
    
    * Fix Formatting
    
    * Update to latest version
    
    * initialize test setup
    
    * Write tests for ads1259_init
    
    * Create tests and add Voltage conversion
    
    * finalize unit tests
    
    * Fix requested changed
    
    * Fix voltage conversion and add greater coverage of test cases
    
    * finalize driver
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 0ab214f71b0c1b3a308a6d0e1937ce01227c168b[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sun Jun 28 21:28:12 2020 -0400

    [SOFT-146] Add resistance to thermistor (#101)
    
    * added resistance to thermistor
    
    * got rid of solar
    
    * make lint and format
    
    * fixed changes
    
    * added enum to resistance_to_temp for clarity
    
    * added enum to resistance_to_temp
    
    * added enum to resistance_to_temp
    
    * make lint and format
    
    Co-authored-by: JARVIS <jarvisweng@hotmail.com>
    Co-authored-by: g-jessmuir <e.jessmuir@gmail.com>

[33mcommit ae46d78499069216794a95237ac17f4f045c1e11[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Jun 27 20:04:17 2020 -0400

    [SOFT-219] Add required BMS CAN messages (#115)
    
    BMS needs to transmit relay state and fan state.

[33mcommit 8e49282941a36982def7a47c100a6c54b3ac1b08[m
Author: JarvisWeng <33078162+JarvisWeng@users.noreply.github.com>
Date:   Sat Jun 27 19:55:06 2020 -0400

    [SOFT-213] mppt solar module (#108)
    
    * changed address to PA0 instead of PA1
    
    * added mppt
    
    * finished mppt tests
    
    * fixed changes
    
    * fix merge conflicts
    
    * fixed changes
    
    Co-authored-by: JARVIS <jarvisweng@hotmail.com>

[33mcommit db85b0d6ae9606c21f8f69b5f68735aa1050519d[m
Author: Tuan (Tristan) Le <38996499+tristanle22@users.noreply.github.com>
Date:   Sat Jun 27 11:41:20 2020 -0400

    [soft-263] - Quick linting and format (#113)
    
    * [soft-263] - Add function to find files in a specific project
    
    * [soft-263] - Modify FIND function to search in specific projects
    
    * [soft-263] - Added LIBRARY_DIR
    
    * [soft-263] - Added lint_quick option
    
    * [soft-263] - Added format_quick, run format_quick and lint_quick during at commit
    
    * [soft-263] - Restore hooks/pre-commit, fix minor bug
    
    * [soft-263] - Use git diff instead of git status for fetching changed/new files
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit cc13a45c2a38edff669dacf895548809e388c2c6[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Sat Jun 27 11:35:15 2020 -0400

    [SOFT-208] sense_current module for solar (#114)
    
    * [SOFT-208] Add base of sense_current
    
    * [SOFT-208] Scale MCP3427 conversion results to 16 bits always
    
    * [SOFT-208] Add basic sense_current callback implementation
    
    * [SOFT-208] Add data_store_init and data_store_get_is_set
    
    * [SOFT-208] Use settings struct for mcp3427 config
    
    * [SOFT-208] Raise a fault event if the MCP3427 faults too much
    
    * [SOFT-208] Add tests to sense_current
    
    * [SOFT-208] Move sense_current settings to solar_config
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit 00d0c57181bb47901cbe58902483f8ce949f5503[m
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Thu Jun 25 12:35:56 2020 -0400

    [SOFT-235] Add DEFINE make argument, use in Travis (#112)
    
    * [SOFT-235] Add DEFINES make argument, use in Travis
    
    * [SOFT-235] Remove debug log
    
    * [SOFT-235] Use the correct make arg in Travis
    
    * [SOFT-235] Add documentation to README
    
    Co-authored-by: Jess Muir <e.jessmuir@gmail.com>

[33mcommit feb151045ed746ad5930c1c46c7f6acf00876889[m
Author: cathcai <45928040+cathcai@users.noreply.github.com>
Date:   Wed Jun 24 20:17:20 2020 -0400

    [SOFT-161] Remove the 'power_distribution' prefix (#103)
    
    * [SOFT-161] Remove the 'power_distribution' prefix
    
    * Cleaned up previous file name comments, added 'pd' prefix to gpio.h and events.h
    
    * Formating updates.
    
    * minor header name change
    
    * Prepended pd_ to gpio_config.h, gpio.c, gpio_config.c, and test_gpio.c for consistency. Cleaned up file naming dependencies.
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit 939bc44f340cc81592addb7f7378d6c07cd816c7[m
Author: Gerald Aryeetey <gndaryee@uwaterloo.ca>
Date:   Tue Jun 23 18:58:52 2020 -0400

    [SOFT-222] Smoketest ltc6811 (#109)
    
    * Initial implementation of smoke test
    
    * Also removes verbose tests for ltc_afe
    
    * Apply formatting changes
    
    * Fix prints on x86
    
    * Bug fixes and minor refactoring in ltc_afe
    
    * fix bug with calculating offsets
    * Make read_reg and voltage_reg mappings more explicit
    * Fix incorrect register for read AUX B
    * Remove debug statements
    
    * Apply formatting fixes
    
    * Simplify changing log behaviour/config
    
    * Simple PR changes
    * Add settings at the top
    * Enable/disable logging using event name mapping
    * Enable/disable voltage/temp test using num samples
    * TODO: Should decide whether or not to do cell discharge tests

[33mcommit ad04d701e3d7ecfff02a4263750b019b34d3d123[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat Jun 20 18:42:15 2020 -0400

    [SOFT-184] Spi functions temperature (#106)
    
    * First commit
    
    * First commit
    
    * Added STCOMM command
    
    * Fixed STCOMM command
    
    * Added basic functionality for WRCOMM command
    
    * Fixed WRCOMM command and wrote to COMM register
    
    * Fixed switch condition based off of ADG731 truth table
    
    * Fixed naming
    
    * Shortened the command, added PEC, and fixed GPIO usage
    
    * Fixed tx_len for SPI and crc15
    
    * Changed STCOMM to send clock cycles
    
    * Removed unneeded definition
    
    * Fixed magic numbers
    
    * Fixed WRCOMM command and PEC
    
    * Fixed naming and turned off all GPIOs before starting WRCOMM command
    
    * Added ICOM2 and unneeded parameter
    
    * Fixed naming
    
    * Fixed naming in other parts of code

[33mcommit 8de1c2a2d2e6be48348007ad5d6761d2e2becc93[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Jun 20 17:18:17 2020 -0400

    [SOFT-262] Implement charging manager and tests (#107)
    
    Charging manager manages the charge state to ensure that
    charger can't start charging the car unless we're in park.

[33mcommit af1403454ccfc930d6aebc9be29cfff614330b70[m
Author: Jess Muir <e.jessmuir@gmail.com>
Date:   Sat Jun 20 16:06:22 2020 -0400

    [SOFT-148] centre console fault handling (#98)
    
    * changed the state machines to hanlde and self-clear their transition faults
    
    * added fault reporting messages
    
    * Formatting (Automated commit)
    
    * reports transition errors now
    
    * added a fault monitor with a watchdog
    
    * Formatting (Automated commit)
    
    * format and lint
    
    * disabled main event generator whenever there is faults, added fault monitor initialization to main
    
    * Formatting (Automated commit)
    
    * typo
    
    * making tests pass
    
    * made easy changes
    
    * [SOFT-148] Add fault handling to centre console
    
    Rather than doing nothing upon fault, turn the car off.
    
    * [SOFT-148] Implement suggestions
    
    Co-authored-by: Arshan Khanifar <arshankhanifar@gmail.com>

[33mcommit a66f13327eb892b8696140c3cf7463281e0db074[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Wed Jun 17 20:36:20 2020 -0400

    [SOFT-203] I2C implementation on x86 (#97)
    
    * First commit
    
    * Added more I2C functionality on x86
    
    * Resolved changes
    
    * Fixed printing on one line
    
    * Resolved breaking the verbosity of the log
    
    Co-authored-by: Ryan Dancy <41841896+copperium@users.noreply.github.com>

[33mcommit c8a4cfdaef3020da6004d48899ffdd0d9dd9fb18[m
Merge: 2d72042 71257eb
Author: Ryan Dancy <41841896+copperium@users.noreply.github.com>
Date:   Tue Jun 16 21:48:33 2020 -0400

    Merge pull request #96 from uw-midsun/soft_187_mcp3427_adc
    
    [SOFT-187] MCP3427 ADC

[33mcommit 71257eb51e02f5dddb96cdae078a866c181dc74e[m
Merge: 08e0cdb 2d72042
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Tue Jun 16 21:43:51 2020 -0400

    Merge branch 'master' into soft_187_mcp3427_adc

[33mcommit 08e0cdb433e85f505d3404920f0fb207eb719d1f[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed Jun 17 01:29:37 2020 +0000

    [SOFT-187] Address comments

[33mcommit 2d720424425a711a404c5c49527ff19418c84847[m[33m ([m[1;31morigin/soft-179_smoke_spv1020[m[33m)[m
Merge: 15cb5f3 5e6623b
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Tue Jun 16 00:13:08 2020 -0400

    Merge pull request #102 from uw-midsun/soft_207_sense
    
    [SOFT-207] Sense module for solar

[33mcommit 5e6623b5205b66804553b25ea6487cb75d839179[m
Merge: d9ee528 15cb5f3
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon Jun 15 23:58:55 2020 -0400

    Merge branch 'master' into soft_207_sense

[33mcommit d9ee528d824a84bd42089e78df5cce2cd04c46fb[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Tue Jun 16 02:55:22 2020 +0000

    [SOFT-207] Address review

[33mcommit 61c35671b2498ac9fc80171200e7bc97d9464731[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sun Jun 14 00:57:48 2020 +0000

    [SOFT-207] Add sense tests

[33mcommit 15cb5f3d42f6aa6fba818f86ccf39ec0f5434ddd[m
Merge: 915dcd2 3925fb3
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 16:31:17 2020 -0400

    Merge pull request #100 from uw-midsun/soft_199_tutorial_board_gpio_pin_fix
    
    [SOFT 199] Fixed pin

[33mcommit 3925fb37dad5398526b075521673a58c30deba61[m
Merge: c335949 915dcd2
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 15:30:34 2020 -0400

    Merge branch 'master' into soft_199_tutorial_board_gpio_pin_fix

[33mcommit 915dcd2186aba1d88ebfd6ffaa732f68e96da1a5[m
Merge: 7cf317f e5fe5e1
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 15:30:21 2020 -0400

    Merge pull request #99 from uw-midsun/soft_145_charger_can_msgs
    
    [SOFT-145] Add charger CAN messages

[33mcommit e5fe5e13f2b13d243433797ded77ad3e29c98aeb[m
Merge: c5cf814 7cf317f
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 15:21:33 2020 -0400

    Merge branch 'master' into soft_145_charger_can_msgs

[33mcommit c335949fc1ac20dbf5903319ff815fee1e88972d[m
Merge: d6021af 7cf317f
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 13:06:30 2020 -0400

    Merge branch 'master' into soft_199_tutorial_board_gpio_pin_fix

[33mcommit d6021affa310c554789bf0b7a241e251b71baaab[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat Jun 13 16:49:39 2020 +0000

    Fixed pin

[33mcommit 7cf317fe98d895ffa85c7539554952469c2dec28[m
Merge: 2c48c50 1bcd0a2
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 12:21:27 2020 -0400

    Merge pull request #93 from uw-midsun/soft_165_ads1015_smoke
    
    [SOFT-165] Ads1015 smoke test

[33mcommit c5cf814e2d0556d22792cb316a819953f4a56877[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 13 06:43:54 2020 -0400

    [SOFT-145] Add charger CAN messages
    
    Add some CAN messages for charger to work.

[33mcommit bf4a9b9fc5156c8ef7cbdafda44d9d16698d666f[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat Jun 13 03:09:09 2020 +0000

    [SOFT-207] Implement sense

[33mcommit 0e998d11f92ab24551908cbd17ca4374d33807b1[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat Jun 13 02:23:59 2020 +0000

    [SOFT-187] ...forgot to run make format again

[33mcommit d52f93955544cde6dbe6d0591e53fa3bae5f426c[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 12 22:18:13 2020 -0400

    [SOFT-187] Cache Mcp3427Storages to save time processing events

[33mcommit c8614c091edea8fc0404b47e0d9333f739a14307[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 12 21:22:32 2020 -0400

    [SOFT-187] Use consistent chip identifiers on stm32 and x86

[33mcommit e0cbb65d3709e7ea17e1d2185b6c365501462daa[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 12 19:45:49 2020 -0400

    [SOFT-187] ...and fix lint error

[33mcommit bc78d93dd9594cb4b35c1c1f94fd9ba0f6d0692b[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 12 19:31:58 2020 -0400

    [SOFT-187] Fix format for CI

[33mcommit 4cc2bf880c05a8167a9ba6189d4fdd9940e346e0[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Thu Jun 11 23:26:40 2020 -0400

    [SOFT-187] Add tests for MCP3427

[33mcommit 1bcd0a291fefd15838197c91c49e758016346d7b[m
Merge: 4012f54 5aaf08a
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Fri Jun 12 00:31:54 2020 +0000

    Merge branch 'soft_165_ads1015_smoke' of https://github.com/uw-midsun/firmware_xiv into soft_165_ads1015_smoke

[33mcommit 4012f5446eaf7a0915e532675c63082701c25fff[m
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Fri Jun 12 00:31:17 2020 +0000

    Addressed PR comments

[33mcommit f26a0ff5b1e02243ac747e5ad0cd09a515f177bf[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed Jun 10 22:47:03 2020 -0400

    [SOFT-187] Add x86 MCP3427 implementation

[33mcommit 23941eb38e67fa79f08d3c7b4b93691b120509a5[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed Jun 10 22:29:52 2020 -0400

    [SOFT-187] Separate conversion results into separate signed channel values

[33mcommit 5aaf08a85529ebee04db771975b3eafef937ff6f[m
Merge: b6694ba 2c48c50
Author: Tuan (Tristan) Le <38996499+tristanle22@users.noreply.github.com>
Date:   Wed Jun 10 21:35:24 2020 -0400

    Merge branch 'master' into soft_165_ads1015_smoke

[33mcommit b6694ba2ef472618bcdc240c0bcf43b0d69c3f44[m
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Wed Jun 10 22:46:43 2020 +0000

    Addressed PR issues

[33mcommit 2c48c508eaead800293da5ce3be2f09b4dee1b62[m
Merge: a2dd72d 090c799
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 10 13:20:15 2020 -0400

    Merge pull request #91 from uw-midsun/soft_65_port_afe_code
    
    [SOFT-65] Port AFE Code

[33mcommit 6dcc03d571df57ace06c9e1d775a1d2921fe991e[m
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Wed Jun 10 15:48:34 2020 +0000

    Clearer instruction

[33mcommit 7b390edaa743870dd66043fbd6033f1aa0a23c44[m
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Wed Jun 10 15:41:35 2020 +0000

    Added iterative reading, modifed make file

[33mcommit cef9dca434a0d2c96d93c830b3c28093ebc60284[m
Author: tristanle22 <tristan.tuanle@gmail.com>
Date:   Tue Jun 9 15:07:26 2020 +0000

    Added smoke test for ADS1015

[33mcommit 090c799d04d693e8c42015c8eb65cdc62f8a84cb[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Tue Jun 9 11:34:04 2020 +0000

    Apply test_format changes

[33mcommit 7232184e18abb6ad328e77acc4c78d2eb465c8d1[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Tue Jun 9 03:07:27 2020 +0000

    Note TODOs for aux code

[33mcommit a15b3f634fa29c94b9aabb2f41261121386ef978[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Tue Jun 9 02:50:23 2020 +0000

    Remove old LTC adc header

[33mcommit 1f5cd70ccb1412be3bef2958937da3b59b61c0a1[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Tue Jun 9 02:43:31 2020 +0000

    Remove unused adc include in afe test

[33mcommit 350d60fd2d6949d1f5df7a7d7ac4bde35e576887[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 18:16:36 2020 +0000

    Fix debug print formatters

[33mcommit 4142297a0c0edcdfbc8779fdfaf39040bc76fa96[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 17:13:30 2020 +0000

    Decode tx_data directly in test
    
    * instead of using funky derefence

[33mcommit 9201ef2424685c2bee61063d0b02a2eb1165d15f[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 15:48:53 2020 +0000

    Fix comment in crc15

[33mcommit 01f5a77e07a04d5bc7a39c75a0947f538dc3aa58[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 08:45:51 2020 +0000

    Fix typo in header name

[33mcommit 72482058c19d108371785b42b66d0d848f989e18[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 08:39:01 2020 +0000

    Fix misnaming of ltc6811 and lint/format errors

[33mcommit b65accf8af2180d07bb7fbf46a2aab2f6a87da79[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 08:05:49 2020 +0000

    remove prospective cell sense implementation

[33mcommit 944a936d5ceaa865fcd57a5fa6a23fa4770342a5[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 08:00:21 2020 +0000

    Rename everything to be ltc6811

[33mcommit b665ddd17946bb289d433a46080ea4c2ca851cc0[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 07:21:25 2020 +0000

    Implement tests
    
    * Takes a while on x86 so implementation may need to be moved to smoketest
    * Bugfixes to get it to work

[33mcommit 84f4cd221d5686439bf9714954ed36f9956482c1[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Mon Jun 8 07:19:26 2020 +0000

    Remove adc test as well (for now)

[33mcommit 2f5c17086906736ae9bad140b248d72690054155[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sun Jun 7 22:30:42 2020 -0400

    [SOFT-187] Clean up MCP3427 driver

[33mcommit 09fdb8752773179d47fa1df375026b566ed5ee7f[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 22:57:23 2020 +0000

    Remove adc code for now

[33mcommit 366715cab96e44564205ded4069da5ca836dff48[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 22:53:38 2020 +0000

    Remove plutus files

[33mcommit d9299e5ead8182ec66795035b9d6030ed301c730[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 13:34:20 2020 +0000

    WIP: cell_sense

[33mcommit 7cfd60ca79ebaa55d8bbb46596ad6db07acf5b1d[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 09:04:18 2020 +0000

    Port AFE implementation
    
    TODO: Fix tests
    TODO: Update comments/variables/filenames to match new chip

[33mcommit bb1292f59df2b2fc95a1e4579aade216766a9ae2[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 08:58:52 2020 +0000

    Port ADC code to ms-drivers
    
    TODO: New underlying chip, so should update accordingly

[33mcommit 3683c9e511a22e980a6748948281d8d280ffae0b[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun Jun 7 08:57:01 2020 +0000

    Move CRC15 code to ms-common
    
    This is where CRC32 is ...

[33mcommit 2d4ee424b5b53816c3814fe157f8bbf751fb9c62[m
Author: Gerald Aryeetey <gndaryee@uwaterloo.ca>
Date:   Sun May 17 01:48:55 2020 -0400

    Ensure that it tries to read from all four registers (eventually)

[33mcommit b24951cbcd5729df3de3485f769521cd2cb8927b[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 03:54:32 2020 +0000

    Restore the fact that there are 12 cells and the bitset should reflect that

[33mcommit fe6efcf773c7cf0aee1421da9e1d2ef546eeb274[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:52:24 2020 +0000

    Fix formatters for new debug prints

[33mcommit 90b002d7323539ffa43f7435bf7b8ec91e152257[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:38:56 2020 +0000

    Fix naming of debug print

[33mcommit f8cc7ccc8a3c81480bc7c3d5ce080310babd3b5f[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:38:21 2020 +0000

    Verify that CRC is failing

[33mcommit 1900598db07718df1a2ff6b100f0f70ebd72e19b[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:27:32 2020 +0000

    Remove potentially extra AFE registers?

[33mcommit 64acab317338cc560f0a762e6e487398f62c4512[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:18:32 2020 +0000

    Add debugging prints along afe read path

[33mcommit 603fb9c13ee7006f89d7d1cc27a195467879152f[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:12:50 2020 +0000

    Only kick-off fault monitor once

[33mcommit 3e0afa515af398d4a32a48c93ee81f5e88f96881[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 01:02:37 2020 +0000

    Add debug print to ensure running

[33mcommit 4750b13b64ace4aa7cde7f7745c0c0ba1bbe7cad[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 00:50:30 2020 +0000

    Remove can dependency

[33mcommit fba7142f567be81b259288faa9e2e43d2597c43b[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 17 00:44:04 2020 +0000

    Fix debug print type

[33mcommit 71ce6d1f1590760bb630517a68b09c914890aab4[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 3 16:30:07 2020 +0000

    Modify more for querying single cell
    
    * Bitsets should only specify the use of 1 cell

[33mcommit 32bf8d60502289debdce6d12702249c36965bbf5[m
Author: Gerald Aryeetey <aryeeteygerald@rogers.com>
Date:   Sun May 3 16:21:57 2020 +0000

    Port over plutus but for debugging a single cell

[33mcommit a2dd72dc27794d83f3d8285532238c5d2e53c4d8[m
Merge: 9edad54 8985f93
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:59:03 2020 -0400

    Merge pull request #65 from uw-midsun/soft_137_steering_stalk
    
    [SOFT-137] Steering Stalk

[33mcommit 8985f93079dc8ecb6e81ce45b77e2bb535ff39ab[m
Merge: fac0c9a 9edad54
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:34:06 2020 -0400

    Merge branch 'master' into soft_137_steering_stalk

[33mcommit 9edad541336f181d472c939f9bca31452864b5e3[m
Merge: c6bbdad 883ba2f
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:32:43 2020 -0400

    Merge pull request #73 from uw-midsun/soft_146_power_selection
    
    [SOFT-146] power selection

[33mcommit 883ba2f36dcc77364860e7db366c0f076896f1a4[m
Merge: eb42afb c6bbdad
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:12:37 2020 -0400

    Merge branch 'master' into soft_146_power_selection

[33mcommit c6bbdad63589e87143ad698d1ea456daf388067e[m
Merge: 11dd2e8 cbf05b2
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:12:20 2020 -0400

    Merge pull request #87 from uw-midsun/soft_206_data_store
    
    [SOFT-206] Implement data_store solar module

[33mcommit eb42afbdf0088124b09262349d808caa22836ff3[m
Merge: b86f669 11dd2e8
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:01:23 2020 -0400

    Merge branch 'master' into soft_146_power_selection

[33mcommit cbf05b2f7fe0612372e12b90eb313ac95309fbcf[m
Merge: b1d3794 11dd2e8
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat Jun 6 19:00:23 2020 -0400

    Merge branch 'master' into soft_206_data_store

[33mcommit b1d3794db99a2a9be8172f17e4c89413bfe6e985[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat Jun 6 18:56:40 2020 -0400

    [SOFT-206] Rename enter -> set, add test and fix invalid one

[33mcommit 6c10a4b2bad6906193ef7118b5c038459b7b9388[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 5 23:32:45 2020 -0400

    [SOFT-206] Fix the same typo twice

[33mcommit 8c57e17ec9a8ece8b42cb9a7531fec91fe625d9f[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 5 23:23:55 2020 -0400

    [SOFT-206] Add main.c so the build doesn't fail

[33mcommit 48a77ca811f3efce180b6bdffbf33f84f763b88f[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 5 23:11:14 2020 -0400

    [SOFT-206] Add tests for data_store

[33mcommit 01420d12a170cb64a2218383a19b5ca50545816b[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 5 22:27:20 2020 -0400

    [SOFT-206] Implement data_store

[33mcommit b341103c7091c4aa2178c7efc7a93b26ac6faca1[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri Jun 5 22:08:43 2020 -0400

    [SOFT-206] Create project, add data_store header

[33mcommit 11dd2e895b3531629205b83c67aae3f47c1cb54c[m
Merge: cfada22 94c11a1
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Fri Jun 5 09:00:25 2020 -0400

    [SOFT-205] Merge pull request #85 from uw-midsun/soft_205_spi_implementation_x86
    
    Add x86 implementation for SPI

[33mcommit 94c11a197c4753d5dc222f758fd032d144d3ffb7[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Thu Jun 4 22:13:50 2020 +0000

    Changed decimal to hex for log debugs

[33mcommit 9892f5e09b2af58792df56a7f31a35553426f55e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Thu Jun 4 12:53:30 2020 +0000

    Changed naming of static variable

[33mcommit 708afd14f7c9c42bdb038e022a5932decab0868e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Thu Jun 4 01:21:20 2020 +0000

    Added x86 implementation for SPI

[33mcommit cfada226162449a2dba46021c9d72484df8b567b[m
Merge: 276f7ae f590e4f
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 3 19:11:08 2020 -0400

    Merge pull request #77 from uw-midsun/soft_188_spv1020
    
    [SOFT-188] SPV1020 MPPT driver

[33mcommit f590e4f291e3a6817153b547994beca71e0de6d4[m
Merge: 91e1e8e 276f7ae
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 3 19:06:43 2020 -0400

    Merge branch 'master' into soft_188_spv1020

[33mcommit 91e1e8ed51064f78ac537c2e3b620e84e9cdf7fb[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed Jun 3 19:04:44 2020 -0400

    [SOFT-188] Remove carriage returns

[33mcommit f4724397077fea7a33f4c3fc607d686c36cd26b5[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed Jun 3 19:02:41 2020 -0400

    [SOFT-188] Implement more suggestions

[33mcommit fac0c9a3609c0ef400e7e7cf3de8598e2c89b192[m
Merge: 3a6c0df 276f7ae
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 3 18:49:16 2020 -0400

    Merge branch 'master' into soft_137_steering_stalk

[33mcommit b86f66967952899257c53320417c22a30efbe853[m
Merge: e30720e 276f7ae
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 3 18:48:46 2020 -0400

    Merge branch 'master' into soft_146_power_selection

[33mcommit 276f7ae2aa2f61c3feef8c7166af326697298885[m
Merge: 99b48fb c21bd87
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Wed Jun 3 13:59:55 2020 -0400

    Merge pull request #82 from uw-midsun/soft_202_bms_carrier_layout
    
    [SOFT-202] Add headers / layout for BMS carrier

[33mcommit c21bd872ecf2999310938e59931fdc7536e2f4fd[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon Jun 1 20:01:26 2020 -0400

    added headers

[33mcommit 4e7b46e288831b573500bab945da3160e027895d[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sun May 31 23:01:18 2020 -0400

    [SOFT-188] Implement suggestions

[33mcommit 99b48fb686212ac674fe4946a28a2826b9d5302c[m
Merge: 6192ad5 ec8a7d2
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat May 30 16:59:24 2020 -0400

    Merge pull request #78 from uw-midsun/soft_199_tutorial_board_gpio_pin_fix
    
    [SOFT-199] Changed pin for tutorial board adc

[33mcommit e30720e274d37b184369d549796b12faf8e2246e[m
Author: Jarvis <jarvis0910@gmail.com>
Date:   Sat May 30 20:02:51 2020 +0000

    fixed changes

[33mcommit ec8a7d21ce1b6dd6196ff579e1191be4f10e2270[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 19:52:19 2020 +0000

    Changed pin for tutorial board adc

[33mcommit 3a6c0dfed5fb55ad939385fbc76f755a0fd3d3d9[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 19:32:48 2020 +0000

    fixed formatting

[33mcommit d75447f05cd8c58dd7e6d79403d726af071f8b5f[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 19:27:16 2020 +0000

    Addressed changes

[33mcommit dccd4c016ed0c26497757c8c02a83dd295941ebb[m
Merge: 9be3a13 6192ad5
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 19:12:50 2020 +0000

    Merge branch 'master' of https://github.com/uw-midsun/firmware_xiv into soft_137_steering_stalk

[33mcommit 9be3a13b8321c5ba2790bccb4d85a6b75a39b437[m
Merge: 445f243 a36184a
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 18:56:22 2020 +0000

    Merge branch 'soft_137_steering_stalk' of https://github.com/uw-midsun/firmware_xiv into soft_137_steering_stalk

[33mcommit 445f243c6976d469e5d2df77198e592d822da2c0[m
Author: Avery Chiu <a27chiu@uwaterloo.ca>
Date:   Sat May 30 18:54:14 2020 +0000

    Undo changes to tutorial_board_adc

[33mcommit e33eb6f658c82b8480567c9f00f79c1f8d64757c[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat May 30 13:45:54 2020 -0400

    [SOFT-188] Add warning on unnecessary SHUT on x86

[33mcommit 3fc08019a83ffc5c2e94ab99d0a92782bbf770bb[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat May 30 13:36:00 2020 -0400

    [SOFT-188] Fix the SPI interfacing

[33mcommit 5770735b75d4210355db407a6b59e343890851be[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Sat May 30 11:23:16 2020 -0400

    [SOFT-188] Allow checking the mystery CR bit

[33mcommit 1e23d869964ddf16d8cce26720d4a741cdf4e5db[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Fri May 29 22:35:40 2020 -0400

    [SOFT-187] Copy MSXII's MCP3427 driver

[33mcommit a36184a713ac1bd94256b933a472783e3d9a3845[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 23:21:42 2020 +0000

    Omitted adc_periodic_reader from make

[33mcommit 6192ad536843ddbd217250e53431f3800e365b40[m
Merge: 7428244 1e89f38
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Fri May 29 12:35:01 2020 -0400

    Merge pull request #76 from uw-midsun/soft_191_pre_commit_hook
    
    [SOFT-191] pre commit hook

[33mcommit 1db2fa6870b30a0af14b14b857143a8889614a66[m
Merge: 559f987 a15c948
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 02:12:35 2020 +0000

    Merge branch 'soft_137_steering_stalk' of https://github.com/uw-midsun/firmware_xiv into soft_137_steering_stalk

[33mcommit 1e89f3895e75acd329c851aea7eea5fb07e9ab0e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 02:11:19 2020 +0000

    changed hook

[33mcommit 559f987154f5cc62c2ee01391511c74a318b718b[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 02:09:53 2020 +0000

    Changed test file name

[33mcommit a15c9482df314ec7ffb851319e429166aaa3ebf8[m
Author: Jarvis <jarvis0910@gmail.com>
Date:   Fri May 29 02:09:11 2020 +0000

    changed test file name

[33mcommit c047e53c0c76db2dd2c1f663aa9cd8333446930f[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 02:02:47 2020 +0000

    Fixed formatting

[33mcommit e1a9efb1f6a64a8086cf616ed264c2fbc8b59caf[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 01:45:31 2020 +0000

    Changed CC increase/decrease to be digital

[33mcommit 80e0bf7031e281a2dac53415ead4be351f02c19b[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 29 00:57:05 2020 +0000

    Changing travis build order

[33mcommit 5d053b868e0c3607b7ea4e2f36e659b4fdb6053c[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed May 27 22:51:04 2020 -0400

    [SOFT-188] Add tests for SPV1020
    
    The tests are fairly simple since nothing complex is happening in
    firmware. All pass.

[33mcommit 5c1c38baaa936c1c1b0c9f6a51f0124094838391[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Wed May 27 22:12:00 2020 -0400

    [SOFT-188] Implement SPV1020 on x86
    
    Also, factor out common x86 and stm32 functionality into
    src/spv1020_mppt.c. Did you know you can have both x86/stm32 C files and
    a common C file compiled simultaneously?

[33mcommit 9a8a0aa3d59eaea9fa1c51f69b653e4cd25b5000[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 23:14:48 2020 +0000

    Added check to .yml.bak file as well

[33mcommit eac229cbb8123bca1892b946a79b7fa8f650518b[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 23:12:48 2020 +0000

    Added code to exit Travis if there is an error early on

[33mcommit 47278c171e79b6fa335e6e07751640307f805f94[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 23:05:54 2020 +0000

    changed travis build

[33mcommit 4106b82bf2b090a006032d3f3c12ffd1edcc8377[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 21:00:02 2020 +0000

    added hook, note that developers must manually move this to the .git file

[33mcommit 9ff3cf31ae0cadd9af3e75d04d14ad42c374e77d[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 20:54:39 2020 +0000

    Adjusted test for travis

[33mcommit fdaa2e77448a83378882a1dce24e869676cd652e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 15:03:04 2020 +0000

    Remove whitespace

[33mcommit a5fe970857ee25ae3fff694cd49784177ac3919b[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Tue May 26 23:05:17 2020 -0400

    [SOFT-188] Implement SPV1020 on STM32

[33mcommit 64a69dd002ef212318b6fe28e11eebdd7169f0f1[m
Author: Ryan Dancy <ryan@keal.ca>
Date:   Tue May 26 22:26:53 2020 -0400

    [SOFT-188] Define initial SPV1020 interface

[33mcommit 7454dde99e5af7013a37085e35f1a18ddf1c37ac[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 00:38:11 2020 +0000

    added space

[33mcommit 1b1dd3487afd9fca224188461db3eaf5a67bf5a8[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 00:25:51 2020 +0000

    testing to see what happens when it fails

[33mcommit be8993c479e2e95453ef4b211ee62775386adc9e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 00:17:15 2020 +0000

    testing hooks

[33mcommit 837206d02a5ff5cbdc0f1a6f085a9bac55aa4dbf[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Wed May 27 00:07:38 2020 +0000

    making adc_periodic_reader test more robust

[33mcommit db669e1c5e00e08c109a4c099e7907daf8815cad[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 23:46:13 2020 +0000

    testing if it works

[33mcommit 6b69de296fac49a76a9ed348d4cac471b538dea9[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 23:38:54 2020 +0000

    Testing precommit hook

[33mcommit 9af7877c6bb8fd6cb2cb6646919358b3dd3dc570[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 23:33:00 2020 +0000

    changed comment

[33mcommit d393df986816e1ba8c2c32b7e30a3aa689a58b46[m
Author: Jarvis <jarvis0910@gmail.com>
Date:   Tue May 26 21:23:12 2020 +0000

    copied make gen files again

[33mcommit 761673c279fb419efca49f81d3e4ac0ad69c7d75[m
Author: Jarvis <jarvis0910@gmail.com>
Date:   Tue May 26 20:48:00 2020 +0000

    lint and format

[33mcommit 02b6af7ced5279514cdafe912992330d4871dce1[m
Merge: c0f15b1 7428244
Author: JARVIS <jarvisweng@hotmail.com>
Date:   Tue May 26 16:39:34 2020 -0400

    fixed conflicts

[33mcommit c0f15b1bd896df7b8794e3b262962623cfbc9fb5[m
Author: Jarvis <jarvis0910@gmail.com>
Date:   Tue May 26 20:31:44 2020 +0000

    make format and lint

[33mcommit 40492248ec9ff1029216d81ed84251f54fa06310[m
Author: JARVIS <jarvisweng@hotmail.com>
Date:   Tue May 26 16:25:59 2020 -0400

    finished tests

[33mcommit 74282445121474145f1eed6477fb834dbc4e95cd[m
Merge: 328303b 9579fd6
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Tue May 26 11:40:11 2020 -0400

    Merge pull request #71 from uw-midsun/soft_185_x86_adc
    
    [SOFT-185] implement x86 adc.c

[33mcommit 9579fd68300d70ef30dd514c46b1dc7190f3cb56[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Tue May 26 00:18:01 2020 -0400

    added support for using adc callback to mock values

[33mcommit b18cece3a948492491ed49d101dc304f5ce04eae[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon May 25 23:43:50 2020 -0400

    fixed x86 implementation of temperature reading and added testcase for adc temperature reading

[33mcommit e8043be48c7f019f8bcd34c1024cbbc415d46d25[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 02:02:54 2020 +0000

    fixing build

[33mcommit f70b2c80f57bceab42944c711176a78eb7ce2998[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 01:30:29 2020 +0000

    Decreased interval time

[33mcommit ce01d909b6a0142b9de9115e5713f3e5c885f09e[m
Merge: 8797594 328303b
Author: Avery Chiu <averychiuu@gmail.com>
Date:   Mon May 25 21:14:54 2020 -0400

    Merge branch 'master' into soft_137_steering_stalk

[33mcommit 8797594c5d8dd0e6bfd880c8070134b36bd03461[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 01:00:54 2020 +0000

    fixed comment

[33mcommit ca70c7ce5686ba0a33e305bbcbbab85626c7ff7e[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 00:54:23 2020 +0000

    changed comment

[33mcommit 26ae36b8b0c89b031886f77e5a23bdeeb052d5cd[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Tue May 26 00:45:14 2020 +0000

    Resolved changes and moved definitions to header files

[33mcommit 096317bdb86b3b61e1d0a89396fd959a15e9563c[m
Merge: 42e05a0 efd5592
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon May 25 10:49:59 2020 -0400

    Merge branch 'soft_185_x86_adc' of https://github.com/uw-midsun/firmware_xiv into soft_185_x86_adc

[33mcommit 42e05a0c0ed8848d2159d3df0c3bef8395c843f2[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon May 25 10:49:48 2020 -0400

    fixed, now run tests for adc on x86

[33mcommit efd559281a6765b587c1bb83fdf930f0cbfe9872[m
Merge: 57a0a9e 328303b
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon May 25 10:03:19 2020 -0400

    Merge branch 'master' into soft_185_x86_adc

[33mcommit 57a0a9e210e5249c0efa7213fa754f5e515e7259[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Mon May 25 10:02:09 2020 -0400

    implement x86 adc.c

[33mcommit 328303bca323f28d5c59b4a9570355d2aa04651d[m
Merge: 1d1fcb2 1612f8f
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sun May 24 23:23:37 2020 -0400

    Merge pull request #70 from uw-midsun/SOFT-169-PCA9539R-Smoke-test
    
    [SOFT-169] pca9539r smoke test

[33mcommit 1612f8f63b50832555d4f54a1099a0453178fdad[m
Merge: 51ec0f5 1d1fcb2
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sun May 24 23:02:19 2020 -0400

    Merge branch 'master' into SOFT-169-PCA9539R-Smoke-test

[33mcommit 51ec0f5331cb3b1ec7c6bd89b2f5855bc3c5bf45[m
Author: mitchellostler <ostlermitchell44@gmail.com>
Date:   Mon May 25 02:49:49 2020 +0000

    made funcs prv

[33mcommit c096ba6b81a9b800040c8919b9ddbe4835b95c9a[m
Author: mitchellostler <ostlermitchell44@gmail.com>
Date:   Mon May 25 02:29:30 2020 +0000

    Resolving code review

[33mcommit fd9010c30d706bd34e2dcbe76b9ccc849f175cf4[m
Author: JARVIS <jarvisweng@hotmail.com>
Date:   Sun May 24 13:47:47 2020 -0400

    still trying to mock prv_checker

[33mcommit 16b649159c3c9eaaaf631b2150afbe0e4cdfa98a[m
Author: JARVIS <jarvisweng@hotmail.com>
Date:   Sun May 24 13:15:43 2020 -0400

    fixing tests

[33mcommit 310d29b41e5f9d4bcd32fdbd99634edd37e2f702[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Sat May 23 21:26:40 2020 +0000

    fixes for PR

[33mcommit e758b50f68dc1a2316fd07744e5f2948ac283887[m
Author: JARVIS <jarvisweng@hotmail.com>
Date:   Sat May 23 17:24:03 2020 -0400

    finished adding can messages

[33mcommit cad5f00092522c8856be5f508b5e7211d02a7cb0[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Sat May 23 20:45:05 2020 +0000

    Fixy fixes

[33mcommit d5d03d95bb02d145c53f584fa6fdc9677b0a6398[m
Author: Mitchell Ostler <ostlermitchell44@gmail.com>
Date:   Sat May 23 19:50:28 2020 +0000

    Completed pca9539 smoke test

[33mcommit ecec82774bf52976f9af41df0652678920d68065[m
Author: Mitchell Ostler <mlostler@uwaterloo.ca>
Date:   Sat May 23 10:58:27 2020 -0700

    Update with TODO list

[33mcommit 1d1fcb2fb29d72cc0a36611c781fe5f1337d14ba[m
Merge: 796517b e2a4682
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat May 23 11:28:58 2020 -0400

    Merge pull request #68 from uw-midsun/soft_160_edit_make_pedal
    
    don't run pedal calib on x86

[33mcommit e2a4682f7cd9a399f5f193bfd730b1c0382a553e[m
Author: g-jessmuir <e.jessmuir@gmail.com>
Date:   Sat May 23 10:34:20 2020 -0400

    don't run pedal calib on x86

[33mcommit a4a41cc5e4dbbaeefbb779a96b0a28bcf3651ba3[m
Author: Mitchell Ostler <mlostler@uwaterloo.ca>
Date:   Fri May 22 21:49:24 2020 -0700

    First commit for PCA9539 Smoke test

[33mcommit a6f7ba4f597b81e9800c446f616a4f1eb231be30[m
Author: AveryChiu64 <a27chiu@uwaterloo.ca>
Date:   Fri May 22 22:02:21 2020 +0000

    Fixed highbeam rear CAN message
