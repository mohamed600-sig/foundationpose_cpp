# Required runtime libs

## x86_64

### runtime installed libs
```bash
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb && sudo rm cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install libgoogle-glog-dev
sudo apt install libnvinfer10=10.9.0.34-1+cuda12.8 libnvinfer-plugin10=10.9.0.34-1+cuda12.8 
sudo apt install libnvonnxparsers10=10.9.0.34-1+cuda12.8  libnvinfer-vc-plugin10=10.9.0.34-1+cuda12.8
```
### cv-cuda
```bash
wget https://github.com/CVCUDA/CV-CUDA/releases/download/v0.15.0/cvcuda-lib-0.15.0-cuda12-x86_64-linux.deb  &&\ 
wget https://github.com/CVCUDA/CV-CUDA/releases/download/v0.15.0/cvcuda-dev-0.15.0-cuda12-x86_64-linux.deb

sudo dpkg -i cvcuda-lib-0.15.0-cuda12-x86_64-linux.deb && sudo dpkg -i cvcuda-dev-0.15.0-cuda12-x86_64-linux.deb

rm cvcuda-lib-0.15.0-cuda12-x86_64-linux.deb && rm cvcuda-dev-0.15.0-cuda12-x86_64-linux.deb
```
## jetson (JP36.4.7)

### cv-cuda
```bash

wget https://github.com/CVCUDA/CV-CUDA/releases/download/v0.15.0/cvcuda-lib-0.15.0-cuda12-aarch64-linux.deb && wget https://github.com/CVCUDA/CV-CUDA/releases/download/v0.15.0/cvcuda-dev-0.15.0-cuda12-aarch64-linux.deb
sudo dpkg -i cvcuda-lib-0.15.0-cuda12-aarch64-linux.deb && sudo dpkg -i cvcuda-dev-0.15.0-cuda12-aarch64-linux.deb
rm cvcuda-lib-0.15.0-cuda12-aarch64-linux.deb && rm cvcuda-dev-0.15.0-cuda12-aarch64-linux.deb

```

---
