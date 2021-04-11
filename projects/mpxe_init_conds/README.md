# mpxe_init_conds
Test used by mpxe to verify that initial conditions are working. Verifies that nothing has broken in terms of the concurrent mechanisms used in MPXE.
Initial conditions refers to states that are available on startup, mimicing hardware pins etc that may be already set by external sources.

Used only for mpxe on x86, and should not be flashed onto stm32 (although probably won't break anything)
Initializes 2 instances of pca9539r drivers (on x86) and checks that initial conditions can be written to both of them by the mpxe framework.

To verify run either `make mpxe` or `make mpxe TEST=init_conds`. Output should be as follows:

[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_0 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_1 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_2 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_3 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_4 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_5 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_6 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 0_7 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_0 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_1 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_2 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_3 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_4 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_5 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_6 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:43: State for First PCA9539R pin 1_7 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_0 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_1 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_2 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_3 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_4 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_5 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_6 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 0_7 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_0 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_1 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_2 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_3 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_4 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_5 == 0
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_6 == 1
[mpxe_init_conds] [0] projects/mpxe_init_conds/src/main.c:50: State for Second PCA9539R pin 1_7 == 0


