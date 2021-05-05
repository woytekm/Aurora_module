echo "===== Aurora module functional tests ====="
echo "Run jlink and jlink_rtt in two other terminals to see test results..."
echo "(JLinkExe -if swd -device nrf52 -speed auto -autoconnect 1)"
echo "(JLinkRTTClient)"
echo "=== press enter to continue..."
read r

for test in 01-flash-usb 02-battery-adc 03-gps 04-blinky 05-buzzer 06-i2c-scan; do
 cd $test/Aurora/blank/armgcc/
 cat test_subject.txt
 make flash
 echo "=== observe test results on JLink RTT screen"
 echo "=== ENTER -> next test"
 read r
 cd ../../../../
done

echo "All tests done"

