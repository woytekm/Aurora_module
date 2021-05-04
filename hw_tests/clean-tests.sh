echo "===== Aurora module functional tests ====="
echo "=== running make clean for test dirs... "

for test in 01-flash-usb 02-battery-adc 03-gps; do
 echo "Cleaning $test ..."
 cd $test/Aurora/blank/armgcc/
 make clean
 cd ../../../../
done

echo "All tests cleaned"

