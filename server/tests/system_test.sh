#!/bin/bash
((
echo open 127.0.0.1 2525
sleep 1
echo "EHLO smtp-test.ru"
sleep 1
echo "MAIL <aa@yandex.ru>"
sleep 1
echo "RCPT <bb@yandex.ru>"
sleep 1
echo "DATA"
sleep 1
echo "EHLO smtp-test.ru"
sleep 1
echo "Hello, nigga"
sleep 1
echo "."
sleep 1
echo "QUIT"
sleep 1
) | telnet)
