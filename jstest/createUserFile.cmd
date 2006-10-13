for %%i in (*.user) do set userFile=%%i
copy %userFile% %userFile:~0,-38%.vcproj.%COMPUTERNAME%.%USERNAME%.user" 
