@echo off

gcc ^
main.c ^
feature_management.c ^
-Imbedtls\include ^
-Lmbedtls\build\library ^
-lmbedcrypto ^
-lmbedtls ^
-lmbedx509 ^
-o test.exe

echo.
echo Build complete.
pause