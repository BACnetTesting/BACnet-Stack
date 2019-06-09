@echo off 
rem loop over all directories and sub directories  
for /D %%d in (*) do (                           
   cd %%d                                        
   if exist cu.bat call cu.bat                     
   cd ..                                           
)                                                  

if exist *.orig del /s /q *.orig 
if exist *.b32 del /s /q *.b32 
if exist *.o del /s /q *.o 
