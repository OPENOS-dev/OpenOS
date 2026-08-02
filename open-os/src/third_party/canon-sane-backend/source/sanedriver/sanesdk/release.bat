cd ..
call release.bat
cd %~dp0

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" prj_win\sanesdk.sln /t:rebuild /p:Configuration=Release;Platform=x86

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" prj_win\sanesdk.sln /t:rebuild /p:Configuration=Release;Platform=x64

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" prj_win\sanesdk.sln /t:rebuild /p:Configuration=Debug;Platform=x86

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" prj_win\sanesdk.sln /t:rebuild /p:Configuration=Debug;Platform=x64
