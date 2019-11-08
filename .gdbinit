# Disable signal interrupts from x86 interrupts
# SIGRTMIN + INTERRUPT_PRIORITY_LOW
handle SIG35 nostop noprint
# SIGRTMIN + INTERRUPT_PRIORITY_NORMAL
handle SIG36 nostop noprint
# SIGRTMIN + INTERRUPT_PRIORITY_HIGH
handle SIG37 nostop noprint
# SIGRTMIN + NUM_INTERRUPT_PRIORITIES
handle SIG38 nostop noprint
