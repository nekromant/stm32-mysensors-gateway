Import("env")
import re
import os


STM32_FRAMEWORK_DIR = env['PROJECT_PACKAGES_DIR'] + "/framework-arduinoststm32-maple"
MYSENSORS_DIR = env['PROJECT_LIBDEPS_DIR']


def sed(file, pattern, replacement):
  with open(file, "r") as sources:
    lines = sources.readlines()
  with open(file, "w") as sources:
    for line in lines:
      if line.find("/* PATCHED */") != -1:
        sources.write(line)
      else:
        sources.write(re.sub(pattern, replacement + "/* PATCHED */", line))

def replace(file, pattern, replacement):
  with open(file, "r") as sources:
    lines = sources.readlines()
  with open(file, "w") as sources:
    for line in lines:
      if line.find("/* PATCHED */") != -1:
        sources.write(line)
      else:
        print(line.replace(pattern, replacement + "/* PATCHED */"))
        sources.write(line.replace(pattern, replacement + "/* PATCHED */"))



print("\n================================= STM32 FRAMEWORK PATCHER =================================\n")
if os.path.isdir(STM32_FRAMEWORK_DIR):
  for root, dirs, files in os.walk(STM32_FRAMEWORK_DIR):
    for file in files:
      if 'main.cpp' in file:
        main_cpp_file = os.path.join(root, file)
        print("[INFO] Patching file: {}".format(main_cpp_file))
        sed(main_cpp_file, '.*int main', '__attribute__ ((weak)) int main')
        sed(main_cpp_file, '.*void premain', '__attribute__(( constructor (101))) __attribute__ ((weak)) void premain') 
else:
  print("[WARN] Couldn't find stm32 framework directory! Searched for: {}".format(STM32_FRAMEWORK_DIR))
print("\n============================================================================================\n")

print("\n================================= MYSENSORS FRAMEWORK PATCHER =================================\n")
if os.path.isdir(MYSENSORS_DIR):
  for root, dirs, files in os.walk(MYSENSORS_DIR):
    for file in files:
      if 'MyMainSTM32F1.cpp' in file:
        main_cpp_file = os.path.join(root, file)
        print("[INFO] Patching file: {}".format(main_cpp_file))
        replace(main_cpp_file, '__attribute__(( constructor (101))) void premain()', 'void orig_premain_ditched()') 
else:
  print("[WARN] Couldn't find MySensors framework directory! Searched for: {}".format(MYSENSORS_DIR))
  exit(1)
print("\n============================================================================================\n")
