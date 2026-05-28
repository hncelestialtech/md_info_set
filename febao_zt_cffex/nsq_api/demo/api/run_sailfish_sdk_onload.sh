export LD_LIBRARY_PATH=.:./h5sdk
export EF_RXQ_SIZE=2048
#export EF_DONT_ACCELERATE=1     #应用程序若要使用onload加速，使用#号注释该环境变量即可

onload -p latency ./HSNsqApiDemo ${1}
#./HSNsqApiDemo ${1}