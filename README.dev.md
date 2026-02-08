cmd /c "`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=amd64 >nul 2>&1 && cmake -G Ninja -B D:\flux\build -S D:\flux -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=D:/flux/llvm-dev/lib/cmake/llvm -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl 2>&1"

cmd /c "`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=amd64 >nul 2>&1 && cmake --build D:\flux\build 2>&1"

New-Item -ItemType Directory -Path "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\DIA SDK\lib\amd64" -Force; Copy-Item "C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\lib\amd64\diaguids.lib" "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\DIA SDK\lib\amd64\diaguids.lib"

New-Item -ItemType Directory -Path "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\DIA SDK\lib\amd64" -Force; Copy-Item "C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\lib\amd64\diaguids.lib" "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\DIA SDK\lib\amd64\diaguids.lib"

cmd /c "`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=amd64 >nul 2>&1 && cmake -G Ninja -B D:\flux\build -S D:\flux -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=D:/flux/llvm-dev/lib/cmake/llvm -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl 2>&1"

cmd /c "`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=amd64 >nul 2>&1 && cmake --build D:\flux\build 2>&1"

(Get-Content "D:\flux\llvm-dev\lib\cmake\llvm\LLVMExports.cmake") -replace 'C:/Program Files \(x86\)/Microsoft Visual Studio/2019/Professional/DIA SDK/lib/amd64/diaguids.lib', 'C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK/lib/amd64/diaguids.lib' | Set-Content "D:\flux\llvm-dev\lib\cmake\llvm\LLVMExports.cmake"

cmd /c ""

"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 >nul && cmake --build D:\flux\build
