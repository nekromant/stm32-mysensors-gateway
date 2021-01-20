Import("env")
import re
import os


STM32_FRAMEWORK_DIR = env['PROJECT_PACKAGES_DIR'] + "/framework-arduinoststm32-maple"


def sed(file, pattern, replacement):
  with open(file, "r") as sources:
    lines = sources.readlines()
  with open(file, "w") as sources:
    for line in lines:
      sources.write(re.sub(pattern, replacement, line))


print("\n================================= STM32 FRAMEWORK PATCHER =================================\n")
if os.path.isdir(STM32_FRAMEWORK_DIR):
  for root, dirs, files in os.walk(STM32_FRAMEWORK_DIR):
    for file in files:
      if 'main.cpp' in file:
        main_cpp_file = os.path.join(root, file)
        print("[INFO] Patching file: {}".format(main_cpp_file))
        sed(main_cpp_file, '.*int main', '__attribute__ ((weak)) int main')
#        sed(main_cpp_file, '.*init\(\)', '// init(); ')
        sed(main_cpp_file, '.*void premain', '__attribute__(( constructor (101))) __attribute__ ((weak)) void premain') 
else:
  print("[WARN] Couldn't find stm32 framework directory! Searched for: {}".format(STM32_FRAMEWORK_DIR))
print("\n============================================================================================\n")
