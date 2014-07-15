export PATH="/usr/bin/:$PATH"
export STMSDKDIR="/Users/Candrian/Documents/ARM_Projects/STM32F4-Discovery_FW_V1.1.0"
export PATH=$PATH:/Applications/Arm/arm-cs-tools/bin
#/usr/local/bin/openocd -f /usr/local/share/openocd/scripts/stm32f4discovery.cfg &
make clean
make
#killall $(ps aux | grep "openocd -f /usr/local/share/openocd/scripts/stm32f4discovery.cfg" | grep -v grep | awk '{print $2}')
