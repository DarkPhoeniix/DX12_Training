@echo off

set SDK_VERSION=2025.1
set SDK_URL=https://developer.nvidia.com/downloads/assets/tools/secure/nsight-aftermath-sdk/2025_1_0/windows/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009.zip
set SDK_PATH=ThirdParty\NsightAftermathSDK

if not exist %SDK_PATH% (
  curl -L "%SDK_URL%" -o AftermathSDK.zip
  mkdir %SDK_PATH%
  tar -xf AftermathSDK.zip -C %SDK_PATH%
  del AftermathSDK.zip
)
