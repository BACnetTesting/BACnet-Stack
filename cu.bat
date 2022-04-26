@echo off 
rem loop over all directories and sub directories  
for /D %%d in (*) do (                           
   cd %%d                                        
   if exist cu.bat call cu.bat                     
   cd ..                                           
)                                                  

del /s /q *.orig 
del /s /q *.o 
