
D:
cd D:/Users/Alex/Desktop/Banana

git add *.*
git add kernel
git add firmware
git add applications
git add bochs
git add drivers
git add packages
git add sysroot
git add installer/*.txt
git add installer/*.bat
git add installer/*.s
git add installer/*.py

git commit -a --allow-empty-message -m ""
git remote add origin https://github.com/A22347/Banana.git
git push -u origin main

pause